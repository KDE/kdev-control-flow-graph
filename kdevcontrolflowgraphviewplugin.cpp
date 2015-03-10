/***************************************************************************
 *   Copyright 2009 Sandro Andrade <sandroandrade@kde.org>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "kdevcontrolflowgraphviewplugin.h"

#include <QAction>

#include <KAboutData>
#include <KMessageBox>
#include <KPluginFactory>

#include <interfaces/icore.h>
#include <interfaces/context.h>
#include <interfaces/iproject.h>
#include <interfaces/idocument.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iruncontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/contextmenuextension.h>

#include <language/duchain/codemodel.h>
#include <language/duchain/declaration.h>
#include <language/duchain/classdeclaration.h>
#include <language/duchain/types/functiontype.h>
#include <language/duchain/functiondefinition.h>
#include <language/duchain/persistentsymboltable.h>
#include <language/duchain/classfunctiondeclaration.h>

#include <language/interfaces/codecontext.h>

#include <language/backgroundparser/parsejob.h>
#include <language/backgroundparser/backgroundparser.h>

#include <project/projectmodel.h>

#include <KTextEditor/Document>
#include <KTextEditor/View>

#include "duchaincontrolflow.h"
#include "dotcontrolflowgraph.h"
#include "controlflowgraphview.h"
#include "duchaincontrolflowjob.h"

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(ControlFlowGraphViewFactory, "kdevcontrolflowgraphview.json", registerPlugin<KDevControlFlowGraphViewPlugin>();)

class KDevControlFlowGraphViewFactory: public KDevelop::IToolViewFactory{
public:
    KDevControlFlowGraphViewFactory(KDevControlFlowGraphViewPlugin *plugin) : m_plugin(plugin) {}
    virtual QWidget* create(QWidget *parent = 0)
    {
        return new ControlFlowGraphView(m_plugin, parent);
    }
    virtual Qt::DockWidgetArea defaultPosition()
    {
        return Qt::BottomDockWidgetArea;
    }
    virtual QString id() const
    {
        return "org.kdevelop.ControlFlowGraphView";
    }
private:
    KDevControlFlowGraphViewPlugin *m_plugin;
};

KDevControlFlowGraphViewPlugin::KDevControlFlowGraphViewPlugin (QObject *parent, const QVariantList &)
:
KDevelop::IPlugin (QStringLiteral("kdevcontrolflowgraphview"), parent),
m_toolViewFactory(new KDevControlFlowGraphViewFactory(this)),
m_activeToolView(0),
m_project(0),
m_abort(false)
{
    core()->uiController()->addToolView(i18n("Control Flow Graph"), m_toolViewFactory);

    QObject::connect(core()->documentController(), SIGNAL(textDocumentCreated(KDevelop::IDocument*)),
                     SLOT(textDocumentCreated(KDevelop::IDocument*)));
    QObject::connect(core()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)),
                     SLOT(projectOpened(KDevelop::IProject*)));
    QObject::connect(core()->projectController(), SIGNAL(projectClosed(KDevelop::IProject*)),
                     SLOT(projectClosed(KDevelop::IProject*)));
    QObject::connect(core()->languageController()->backgroundParser(), SIGNAL(parseJobFinished(KDevelop::ParseJob*)),
                     SLOT(parseJobFinished(KDevelop::ParseJob*)));
                     
    m_exportControlFlowGraph = new QAction(i18n("Export Control Flow Graph"), this);
    connect(m_exportControlFlowGraph, SIGNAL(triggered(bool)), SLOT(slotExportControlFlowGraph(bool)), Qt::UniqueConnection);

    m_exportClassControlFlowGraph = new QAction(i18n("Export Class Control Flow Graph"), this);
    connect(m_exportClassControlFlowGraph, SIGNAL(triggered(bool)), SLOT(slotExportClassControlFlowGraph(bool)), Qt::UniqueConnection);

    m_exportProjectControlFlowGraph = new QAction(i18n("Export Project Control Flow Graph"), this);
    connect(m_exportProjectControlFlowGraph, SIGNAL(triggered(bool)), SLOT(slotExportProjectControlFlowGraph(bool)), Qt::UniqueConnection);
}

KDevControlFlowGraphViewPlugin::~KDevControlFlowGraphViewPlugin()
{
}

QString KDevControlFlowGraphViewPlugin::statusName() const
{
    return i18n("Control Flow Graph");    
}

void KDevControlFlowGraphViewPlugin::unload()
{
    // When calling removeToolView all existing views are destroyed and their destructor invoke unRegisterToolView.
    core()->uiController()->removeToolView(m_toolViewFactory);
}

void KDevControlFlowGraphViewPlugin::registerToolView(ControlFlowGraphView *view)
{
    m_toolViews << view;
}

void KDevControlFlowGraphViewPlugin::unRegisterToolView(ControlFlowGraphView *view)
{
    m_toolViews.removeAll(view);
}

QPointer<ControlFlowGraphFileDialog> KDevControlFlowGraphViewPlugin::exportControlFlowGraph(ControlFlowGraphFileDialog::OpeningMode mode)
{
    QPointer<ControlFlowGraphFileDialog> fileDialog = new ControlFlowGraphFileDialog((QWidget *) ICore::self()->uiController()->activeMainWindow(), mode);
    if (fileDialog->exec() == QDialog::Accepted)
    {
        if (fileDialog && !fileDialog->selectedFiles().isEmpty())
        {
            QString fileName = fileDialog->selectedFiles()[0];
            if (!fileName.isEmpty())
                return fileDialog;
        }
    }
    return 0;
}

KDevelop::ContextMenuExtension
KDevControlFlowGraphViewPlugin::contextMenuExtension(KDevelop::Context* context)
{
    KDevelop::ContextMenuExtension extension;

    if (context->hasType(Context::CodeContext) || context->hasType(Context::EditorContext))
    {
        KDevelop::DeclarationContext *codeContext = dynamic_cast<KDevelop::DeclarationContext*>(context);

        if (!codeContext)
            return extension;

        DUChainReadLocker readLock(DUChain::lock());
        Declaration *declaration(codeContext->declaration().data());

        // Insert action for generating control flow graph from method
        if (declaration && declaration->type<KDevelop::FunctionType>() && (declaration->isDefinition() || FunctionDefinition::definition(declaration)))
        {
            m_exportControlFlowGraph->setData(QVariant::fromValue(DUChainBasePointer(declaration)));
            extension.addAction(KDevelop::ContextMenuExtension::ExtensionGroup, m_exportControlFlowGraph);
        }
        // Insert action for generating control flow graph for the whole class
        else if (declaration && declaration->kind() == Declaration::Type &&
                declaration->internalContext() &&
                declaration->internalContext()->type() == DUContext::Class)
        {
            m_exportClassControlFlowGraph->setData(QVariant::fromValue(DUChainBasePointer(declaration)));
            extension.addAction(KDevelop::ContextMenuExtension::ExtensionGroup, m_exportClassControlFlowGraph);
        }
    }
    else if (context->hasType(Context::ProjectItemContext))
    {
        KDevelop::ProjectItemContext *projectItemContext = dynamic_cast<KDevelop::ProjectItemContext*>(context);
        if (projectItemContext)
        {
            QList<ProjectBaseItem *> items = projectItemContext->items();
            foreach(ProjectBaseItem *item, items)
            {
                ProjectFolderItem *folder = item->folder();
                if (folder && !folder->parent())
                {
                    m_exportProjectControlFlowGraph->setData(QVariant::fromValue(folder->project()->name()));
                    extension.addAction(KDevelop::ContextMenuExtension::ExtensionGroup, m_exportProjectControlFlowGraph);
                }
            }
        }
    }
    return extension;
}

void KDevControlFlowGraphViewPlugin::projectOpened(KDevelop::IProject* project)
{
    Q_UNUSED(project);
    foreach (ControlFlowGraphView *controlFlowGraphView, m_toolViews)
        controlFlowGraphView->setProjectButtonsEnabled(true);
    refreshActiveToolView();
}

void KDevControlFlowGraphViewPlugin::projectClosed(KDevelop::IProject* project)
{
    Q_UNUSED(project);
    if (core()->projectController()->projectCount() == 0)
    {
        foreach (ControlFlowGraphView *controlFlowGraphView, m_toolViews)
        {
            controlFlowGraphView->setProjectButtonsEnabled(false);
            controlFlowGraphView->newGraph();
        }
    }
    refreshActiveToolView();
}

void KDevControlFlowGraphViewPlugin::parseJobFinished(KDevelop::ParseJob* parseJob)
{
    if (core()->documentController()->activeDocument() &&
        parseJob->document().toUrl() == core()->documentController()->activeDocument()->url())
        refreshActiveToolView();
}

void KDevControlFlowGraphViewPlugin::textDocumentCreated(KDevelop::IDocument *document)
{
    connect(document->textDocument(), SIGNAL(viewCreated(KTextEditor::Document*, KTextEditor::View*)),
            SLOT(viewCreated(KTextEditor::Document*, KTextEditor::View*)));
}

void KDevControlFlowGraphViewPlugin::viewCreated(KTextEditor::Document *document, KTextEditor::View *view)
{
    Q_UNUSED(document);
    connect(view, SIGNAL(cursorPositionChanged(KTextEditor::View*, KTextEditor::Cursor)),
            SLOT(cursorPositionChanged(KTextEditor::View*, KTextEditor::Cursor)));
    connect(view, SIGNAL(destroyed(QObject*)), SLOT(viewDestroyed(QObject*)));
    connect(view, SIGNAL(focusIn(KTextEditor::View*)), SLOT(focusIn(KTextEditor::View*)));
}

void KDevControlFlowGraphViewPlugin::viewDestroyed(QObject *object)
{
    Q_UNUSED(object);
    if (!core()->documentController()->activeDocument() && m_activeToolView)
        m_activeToolView->newGraph();
}

void KDevControlFlowGraphViewPlugin::focusIn(KTextEditor::View *view)
{
    if (view)
        cursorPositionChanged(view, view->cursorPosition());
}

void KDevControlFlowGraphViewPlugin::cursorPositionChanged(KTextEditor::View *view, const KTextEditor::Cursor &cursor)
{
    if (m_activeToolView)
        m_activeToolView->cursorPositionChanged(view, cursor);
}

void KDevControlFlowGraphViewPlugin::refreshActiveToolView()
{
    if (m_activeToolView)
        m_activeToolView->refreshGraph();
}

void KDevControlFlowGraphViewPlugin::slotExportControlFlowGraph(bool value)
{
    // Export graph for a given function
    Q_UNUSED(value);

    if (m_duchainControlFlow || m_dotControlFlowGraph)
    {
        KMessageBox::error((QWidget *) core()->uiController()->activeMainWindow(), i18n("There is a graph being currently exported. Please stop it before requiring a new one"));
        return;
    }

    DUChainReadLocker lock(DUChain::lock());

    Q_ASSERT(qobject_cast<QAction *>(sender()));
    QAction *action = static_cast<QAction *>(sender());
    Q_ASSERT(action->data().canConvert<DUChainBasePointer>());
    DeclarationPointer declarationPointer = qvariant_cast<DUChainBasePointer>(action->data()).dynamicCast<Declaration>();

    Declaration *declaration = declarationPointer.data();
    if (!declaration)
    {
        KMessageBox::error((QWidget *) core()->uiController()->activeMainWindow(), i18n("Could not generate function control flow graph"));
        return;
    }

    if (!declaration->isDefinition())
    {
        declaration = FunctionDefinition::definition(declaration);
        if (!declaration || !declaration->internalContext())
        {
            KMessageBox::error((QWidget *) core()->uiController()->activeMainWindow(), i18n("Could not generate control flow graph"));
            return;
        }
    }
    if ((m_fileDialog = exportControlFlowGraph()))
    {
        DUChainControlFlowJob *job = new DUChainControlFlowJob(declaration->qualifiedIdentifier().toString(), this);
        job->setControlFlowJobType(DUChainControlFlowInternalJob::ControlFlowJobBatchForFunction);
        m_ideclaration = IndexedDeclaration(declaration);
        connect (job, SIGNAL(result(KJob*)), SLOT(generationDone(KJob*)));
        ICore::self()->runController()->registerJob(job);
    }
    action->setData(QVariant::fromValue(DUChainBasePointer()));
}

void KDevControlFlowGraphViewPlugin::slotExportClassControlFlowGraph(bool value)
{
    // Export graph for all functions of a given class - individual per-function graphs will be merged
    Q_UNUSED(value);

    if (m_duchainControlFlow || m_dotControlFlowGraph)
    {
        KMessageBox::error((QWidget *) core()->uiController()->activeMainWindow(), i18n("There is a graph being currently exported. Please stop it before requiring a new one"));
        return;
    }

    DUChainReadLocker lock(DUChain::lock());

    Q_ASSERT(qobject_cast<QAction *>(sender()));
    QAction *action = static_cast<QAction *>(sender());
    Q_ASSERT(action->data().canConvert<DUChainBasePointer>());
    DeclarationPointer declarationPointer = qvariant_cast<DUChainBasePointer>(action->data()).dynamicCast<Declaration>();

    Declaration *declaration = declarationPointer.data();
    if (!declaration)
    {
        KMessageBox::error((QWidget *) core()->uiController()->activeMainWindow(), i18n("Could not generate class control flow graph"));
        return;
    }

    if ((m_fileDialog = exportControlFlowGraph(ControlFlowGraphFileDialog::ForClassConfigurationButtons)))
    {
        DUChainControlFlowJob *job = new DUChainControlFlowJob(declaration->qualifiedIdentifier().toString(), this);
        job->setControlFlowJobType(DUChainControlFlowInternalJob::ControlFlowJobBatchForClass);
        m_ideclaration = IndexedDeclaration(declaration);
        connect (job, SIGNAL(result(KJob*)), SLOT(generationDone(KJob*)));
        ICore::self()->runController()->registerJob(job);
    }
    action->setData(QVariant::fromValue(DUChainBasePointer()));
}

void KDevControlFlowGraphViewPlugin::slotExportProjectControlFlowGraph(bool value)
{
    // Export graph for all classes of a given project - individual per-class graphs will be merged
    Q_UNUSED(value);

    if (m_duchainControlFlow || m_dotControlFlowGraph)
    {
        KMessageBox::error((QWidget *) core()->uiController()->activeMainWindow(), i18n("There is a graph being currently exported. Please stop it before requiring a new one"));
        return;
    }

    Q_ASSERT(qobject_cast<QAction *>(sender()));
    QAction *action = static_cast<QAction *>(sender());
    Q_ASSERT(action->data().canConvert<QString>());
    QString projectName = qvariant_cast<QString>(action->data());

    if (projectName.isEmpty())
    {
        KMessageBox::error((QWidget *) core()->uiController()->activeMainWindow(), i18n("Could not generate project control flow graph - project name empty"));
        return;
    }

    IProject *project = core()->projectController()->findProjectByName(projectName);

    if (!project)
    {
        KMessageBox::error((QWidget *) core()->uiController()->activeMainWindow(), i18n("Could not generate project control flow graph - project not found"));
        return;
    }

    if ((m_fileDialog = exportControlFlowGraph(ControlFlowGraphFileDialog::ForClassConfigurationButtons)))
    {
        DUChainControlFlowJob *job = new DUChainControlFlowJob(projectName, this);
        job->setControlFlowJobType(DUChainControlFlowInternalJob::ControlFlowJobBatchForProject);
        m_project = project;
        connect (job, SIGNAL(result(KJob*)), SLOT(generationDone(KJob*)));
        ICore::self()->runController()->registerJob(job);
    }
    action->setData(QVariant::fromValue(QString()));
}

void KDevControlFlowGraphViewPlugin::generateControlFlowGraph()
{
    DUChainReadLocker readLock(DUChain::lock());

    Declaration *declaration = m_ideclaration.data();
    if (!declaration)
        return;

    m_abort = false;
    m_dotControlFlowGraph = new DotControlFlowGraph;
    m_duchainControlFlow = new DUChainControlFlow(m_dotControlFlowGraph);

    configureDuchainControlFlow(m_duchainControlFlow, m_dotControlFlowGraph, m_fileDialog);

    m_duchainControlFlow->generateControlFlowForDeclaration(m_ideclaration, IndexedTopDUContext(declaration->topContext()), IndexedDUContext(declaration->internalContext()));
    exportGraph();
}

void KDevControlFlowGraphViewPlugin::generateClassControlFlowGraph()
{
    DUChainReadLocker readLock(DUChain::lock());

    Declaration *declaration = m_ideclaration.data();
    if (!declaration)
        return;

    m_abort = false;
    m_dotControlFlowGraph = new DotControlFlowGraph;
    m_duchainControlFlow = new DUChainControlFlow(m_dotControlFlowGraph);
    
    configureDuchainControlFlow(m_duchainControlFlow, m_dotControlFlowGraph, m_fileDialog);

    if (!declaration->isForwardDeclaration() && declaration->internalContext())
    {
        int i = 0;
        int max = declaration->internalContext()->localDeclarations().size();
        // For each function declaration
        ClassFunctionDeclaration *functionDeclaration;
        foreach (Declaration *decl, declaration->internalContext()->localDeclarations())
        {
            if (m_abort)
                break;

            emit showProgress(this, 0, max-1, i);
            emit showMessage(this, i18n("Generating graph for function %1", decl->identifier().toString()));
            ++i;
            if ((functionDeclaration = dynamic_cast<ClassFunctionDeclaration *>(decl)))
            {
                Declaration *functionDefinition = FunctionDefinition::definition(functionDeclaration);
                if (functionDefinition)
                    m_duchainControlFlow->generateControlFlowForDeclaration(IndexedDeclaration(functionDefinition), IndexedTopDUContext(functionDefinition->topContext()), IndexedDUContext(functionDefinition->internalContext()));
            }
        }
    }
    if (!m_abort && !m_fileDialog->selectedFiles().isEmpty())
    {
        emit showMessage(this, i18n("Saving file %1", m_fileDialog->selectedFiles()[0]));
        exportGraph();
    }
    emit hideProgress(this);
    emit clearMessage(this);
}

void KDevControlFlowGraphViewPlugin::generateProjectControlFlowGraph()
{
    if (!m_project)
        return;

    m_abort = false;
    m_dotControlFlowGraph = new DotControlFlowGraph;
    m_duchainControlFlow = new DUChainControlFlow(m_dotControlFlowGraph);

    configureDuchainControlFlow(m_duchainControlFlow, m_dotControlFlowGraph, m_fileDialog);

    DUChainReadLocker readLock(DUChain::lock());

    int i = 0;
    int max = m_project->fileSet().size();
    // For each source file
    foreach(const IndexedString &file, m_project->fileSet())
    {
        emit showProgress(this, 0, max-1, i);
        ++i;

        uint codeModelItemCount = 0;
        const CodeModelItem *codeModelItems = 0;
        CodeModel::self().items(file, codeModelItemCount, codeModelItems);
        
        for (uint codeModelItemIndex = 0; codeModelItemIndex < codeModelItemCount; ++codeModelItemIndex)
        {
            const CodeModelItem &item = codeModelItems[codeModelItemIndex];
            
            if ((item.kind & CodeModelItem::Class) && !item.id.identifier().last().toString().isEmpty())
            {
                uint declarationCount = 0;
                const IndexedDeclaration *declarations = 0;
                PersistentSymbolTable::self().declarations(item.id.identifier(), declarationCount, declarations);
                // For each class declaration
                for (uint j = 0; j < declarationCount; ++j)
                {
                    Declaration *declaration = dynamic_cast<Declaration *>(declarations[j].declaration());
                    if (declaration && !declaration->isForwardDeclaration() && declaration->internalContext())
                    {
                        // For each function declaration
                        ClassFunctionDeclaration *functionDeclaration;
                        foreach (Declaration *decl, declaration->internalContext()->localDeclarations())
                        {
                            emit showMessage(this, i18n("Generating graph for %1 - %2", file.str(), decl->qualifiedIdentifier().toString()));
                            if ((functionDeclaration = dynamic_cast<ClassFunctionDeclaration *>(decl)))
                            {
                                if (m_abort)
                                    break;
                                
                                Declaration *functionDefinition = FunctionDefinition::definition(functionDeclaration);
                                if (functionDefinition)
                                    m_duchainControlFlow->generateControlFlowForDeclaration(IndexedDeclaration(functionDefinition), IndexedTopDUContext(functionDefinition->topContext()), IndexedDUContext(functionDefinition->internalContext()));
                            }
                        }
                        if (m_abort)
                            break;
                    }
                }
                if (m_abort)
                    break;
            }
        }
        if (m_abort)
            break;
    }
    if (!m_abort && !m_fileDialog->selectedFiles().isEmpty())
    {
        emit showMessage(this, i18n("Saving file %1", m_fileDialog->selectedFiles()[0]));
        exportGraph();
    }
    m_project = 0;
    emit hideProgress(this);
    emit clearMessage(this);
}

void KDevControlFlowGraphViewPlugin::requestAbort()
{
    m_abort = true;
}

void KDevControlFlowGraphViewPlugin::setActiveToolView(ControlFlowGraphView *activeToolView)
{
    m_activeToolView = activeToolView;
    refreshActiveToolView();
}

void KDevControlFlowGraphViewPlugin::generationDone(KJob *job)
{
    job->deleteLater();

    delete m_dotControlFlowGraph;
    delete m_duchainControlFlow;

    if (!m_abort)
        KMessageBox::information((QWidget *) (core()->uiController()->activeMainWindow()),
                                 i18n("Control flow graph exported"),
                                 i18n("Export Control Flow Graph"));
}

void KDevControlFlowGraphViewPlugin::exportGraph()
{
    DotControlFlowGraph::mutex.lock();
    if (!m_fileDialog->selectedFiles().isEmpty()) {
        m_dotControlFlowGraph->exportGraph(m_fileDialog->selectedFiles()[0]);
    }
    DotControlFlowGraph::mutex.unlock();
}

void KDevControlFlowGraphViewPlugin::configureDuchainControlFlow(DUChainControlFlow *duchainControlFlow, DotControlFlowGraph *dotControlFlowGraph, ControlFlowGraphFileDialog *fileDialog)
{
    duchainControlFlow->setControlFlowMode(fileDialog->controlFlowMode());
    duchainControlFlow->setClusteringModes(fileDialog->clusteringModes());
    duchainControlFlow->setMaxLevel(fileDialog->maxLevel());
    duchainControlFlow->setUseFolderName(fileDialog->useFolderName());
    duchainControlFlow->setUseShortNames(fileDialog->useShortNames());
    duchainControlFlow->setDrawIncomingArcs(fileDialog->drawIncomingArcs());

    dotControlFlowGraph->prepareNewGraph();
}

#include "kdevcontrolflowgraphviewplugin.moc"