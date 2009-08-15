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

#include <kgenericfactory.h>
#include <kaboutdata.h>
#include <kservice.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>

#include <interfaces/context.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/contextmenuextension.h>
#include <interfaces/icore.h>
#include <language/duchain/declaration.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/functiondefinition.h>
#include <language/duchain/use.h>

#include "controlflowgraphview.h"

using namespace KDevelop;

K_PLUGIN_FACTORY(ControlFlowGraphViewFactory, registerPlugin<KDevControlFlowGraphViewPlugin>();)
K_EXPORT_PLUGIN(ControlFlowGraphViewFactory(KAboutData("kdevcontrolflowgraphview","kdecontrolflowgraph", ki18n("Control Flow Graph"), "0.1", ki18n("Control flow graph support in KDevelop"), KAboutData::License_GPL)))

class KDevControlFlowGraphViewFactory: public KDevelop::IToolViewFactory{
public:
    KDevControlFlowGraphViewFactory(KDevControlFlowGraphViewPlugin *plugin) : m_plugin(plugin) {}
    virtual QWidget* create(QWidget *parent = 0)
    {
	ControlFlowGraphView *controlFlowGraphView = new ControlFlowGraphView(parent);

	foreach (KDevelop::IDocument *document, m_plugin->core()->documentController()->openDocuments())
	{
	    controlFlowGraphView->textDocumentCreated(document);
	    if (document->textDocument())
		foreach (KTextEditor::View *view, document->textDocument()->views())
		    controlFlowGraphView->viewCreated(document->textDocument(), view);
	}

	QObject::connect(m_plugin->core()->documentController(), SIGNAL(textDocumentCreated(KDevelop::IDocument *)),
		         controlFlowGraphView, SLOT(textDocumentCreated(KDevelop::IDocument *)));
	QObject::connect(m_plugin->core()->projectController(), SIGNAL(projectOpened(KDevelop::IProject*)),
		         controlFlowGraphView, SLOT(projectOpened(KDevelop::IProject*)));
	QObject::connect(m_plugin->core()->projectController(), SIGNAL(projectClosing(KDevelop::IProject*)),
		         controlFlowGraphView, SLOT(projectClosing(KDevelop::IProject*)));
	QObject::connect(m_plugin->core()->projectController(), SIGNAL(projectClosed(KDevelop::IProject*)),
		         controlFlowGraphView, SLOT(projectClosed(KDevelop::IProject*)));

        return controlFlowGraphView;
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
: KDevelop::IPlugin (ControlFlowGraphViewFactory::componentData(), parent)
{
    m_viewFactory = new KDevControlFlowGraphViewFactory(this);
    core()->uiController()->addToolView("Control Flow Graph", m_viewFactory);
}

KDevControlFlowGraphViewPlugin::~KDevControlFlowGraphViewPlugin()
{
}

void KDevControlFlowGraphViewPlugin::unload()
{
    core()->uiController()->removeToolView(m_viewFactory);
}

KDevelop::ContextMenuExtension
KDevControlFlowGraphViewPlugin::contextMenuExtension(KDevelop::Context* context)
{
    KDevelop::ContextMenuExtension extension;

    if (context->hasType(KDevelop::Context::EditorContext))
	qDebug() << "Editor Context";
    else if (context->hasType(KDevelop::Context::FileContext)) {
	qDebug() << "File Context";
    } else if (context->hasType(KDevelop::Context::CodeContext)) {
	qDebug() << "Code Context";
//   KDevelop::DeclarationContext *codeContext = dynamic_cast<KDevelop::DeclarationContext*>(context);
// 
//   if (!codeContext)
//       return menuExt;
// 
//   DUChainReadLocker readLock(DUChain::lock());
//   Declaration* decl(codeContext->declaration().data());
// 
//   if (decl)
//   {
//     if(decl->inSymbolTable()) {
//       if(!ClassTree::populatingClassBrowserContextMenu() && ICore::self()->projectController()->findProjectForUrl(decl->url()
// .toUrl()) &&
//         decl->kind() == Declaration::Type && decl->internalContext() && decl->internalContext()->type() == DUContext::Class)
// {
//         //Currently "Find in Class Browser" seems to only work for classes, so only show it in that case
// 
//         m_findInBrowser->setData(QVariant::fromValue(DUChainBasePointer(decl)));
//         menuExt.addAction( KDevelop::ContextMenuExtension::ExtensionGroup, m_findInBrowser);
//       }
// 
//       m_openDec->setData(QVariant::fromValue(DUChainBasePointer(decl)));
//       menuExt.addAction( KDevelop::ContextMenuExtension::ExtensionGroup, m_openDec);
// 
//       if(FunctionDefinition::definition(decl)) {
//         m_openDef->setData(QVariant::fromValue(DUChainBasePointer(decl)));
//         menuExt.addAction( KDevelop::ContextMenuExtension::ExtensionGroup, m_openDef);
//       }
//     }
//   }

	} else if (context->hasType(KDevelop::Context::ProjectItemContext)) {
	    qDebug() << "Project Item Context";
/*                KDevelop::ProjectItemContext* prjctx = dynamic_cast<KDevelop::ProjectItemContext*>(context);
                m_prjItems = prjctx->items();
                ext.addAction(KDevelop::ContextMenuExtension::ExtensionGroup, m_formatFilesAction);*/
        }

    
    return extension;
}

#include "kdevcontrolflowgraphviewplugin.moc"
