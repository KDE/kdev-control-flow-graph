/***************************************************************************
 *   Copyright 2009 Sandro Andrade <sandro.andrade@gmail.com>              *
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

#include <kgenericfactory.h>
#include <kaboutdata.h>
#include <kservice.h>
#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ktexteditor/cursor.h>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>
#include <interfaces/ilanguagecontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/idocument.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <language/duchain/declaration.h>
#include <language/duchain/functiondefinition.h>
#include <language/interfaces/codecontext.h>
#include <language/backgroundparser/backgroundparser.h>
#include <language/backgroundparser/parsejob.h>

#include "controlflowgraphview.h"

using namespace KDevelop;

K_PLUGIN_FACTORY(ControlFlowGraphViewFactory, registerPlugin<KDevControlFlowGraphViewPlugin>();)
K_EXPORT_PLUGIN(ControlFlowGraphViewFactory(KAboutData("kdevcontrolflowgraphview","kdecontrolflowgraph", ki18n("Control Flow Graph"), "0.1", ki18n("Control flow graph support in KDevelop"), KAboutData::License_GPL)))

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
KDevelop::IPlugin (ControlFlowGraphViewFactory::componentData(), parent),
m_toolViewFactory(new KDevControlFlowGraphViewFactory(this)),
m_activeToolView(0)
{
    core()->uiController()->addToolView(i18n("Control Flow Graph"), m_toolViewFactory);

    QObject::connect(core()->documentController(), SIGNAL(textDocumentCreated(KDevelop::IDocument *)),
		     this, SLOT(textDocumentCreated(KDevelop::IDocument *)));
    QObject::connect(core()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)),
		     this, SLOT(projectOpened(KDevelop::IProject*)));
    QObject::connect(core()->projectController(), SIGNAL(projectClosed(KDevelop::IProject*)),
		     this, SLOT(projectClosed(KDevelop::IProject*)));
    QObject::connect(core()->languageController()->backgroundParser(), SIGNAL(parseJobFinished(KDevelop::ParseJob*)),
		     this, SLOT(refreshActiveToolView()));
		     
    m_exportControlFlowGraph = new QAction(i18n("Export control flow graph"), this);
    connect(m_exportControlFlowGraph, SIGNAL(triggered(bool)), this, SLOT(slotExportControlFlowGraph(bool)));
}

KDevControlFlowGraphViewPlugin::~KDevControlFlowGraphViewPlugin()
{
}

void KDevControlFlowGraphViewPlugin::unload()
{
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

KDevelop::ContextMenuExtension
KDevControlFlowGraphViewPlugin::contextMenuExtension(KDevelop::Context* context)
{
    KDevelop::ContextMenuExtension extension;

    KDevelop::DeclarationContext *codeContext = dynamic_cast<KDevelop::DeclarationContext*>(context);

    if (!codeContext)
	return extension;

    DUChainReadLocker readLock(DUChain::lock());
    Declaration *declaration(codeContext->declaration().data());

    if (declaration && declaration->inSymbolTable())
    {
	if (FunctionDefinition::definition(declaration))
	{
	    m_exportControlFlowGraph->setData(QVariant::fromValue(DUChainBasePointer(declaration)));
	    extension.addAction(KDevelop::ContextMenuExtension::ExtensionGroup, m_exportControlFlowGraph);
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
	    controlFlowGraphView->setProjectButtonsEnabled(true);
	    controlFlowGraphView->refreshGraph();
	}
    }
}

void KDevControlFlowGraphViewPlugin::textDocumentCreated(KDevelop::IDocument *document)
{
    connect(document->textDocument(), SIGNAL(viewCreated(KTextEditor::Document *, KTextEditor::View *)),
	    this, SLOT(viewCreated(KTextEditor::Document *, KTextEditor::View *)));
}

void KDevControlFlowGraphViewPlugin::viewCreated(KTextEditor::Document *document, KTextEditor::View *view)
{
    Q_UNUSED(document);
    connect(view, SIGNAL(cursorPositionChanged(KTextEditor::View *, const KTextEditor::Cursor &)),
	    this, SLOT(cursorPositionChanged(KTextEditor::View *, const KTextEditor::Cursor &)));
    connect(view, SIGNAL(destroyed(QObject *)), this, SLOT(viewDestroyed(QObject *)));
    connect(view, SIGNAL(focusIn(KTextEditor::View *)), this, SLOT(focusIn(KTextEditor::View *)));
}

void KDevControlFlowGraphViewPlugin::viewDestroyed(QObject *object)
{
    Q_UNUSED(object);
    if (!core()->documentController()->activeDocument())
    {
	if (m_activeToolView)
	    m_activeToolView->newGraph();
    }
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

void KDevControlFlowGraphViewPlugin::slotExportControlFlowGraph(bool)
{
}

void KDevControlFlowGraphViewPlugin::setActiveToolView(ControlFlowGraphView *activeToolView)
{
    m_activeToolView = activeToolView;
    refreshActiveToolView();
}

#include "kdevcontrolflowgraphviewplugin.moc"
