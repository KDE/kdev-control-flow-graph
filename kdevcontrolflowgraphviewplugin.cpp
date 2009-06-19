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

#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/icore.h>
#include <language/duchain/declaration.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/functiondefinition.h>
#include <language/duchain/use.h>

#include "controlflowgraphview.h"
#include "duchaincontrolflow.h"
#include "dotcontrolflowgraph.h"

using namespace KDevelop;

K_PLUGIN_FACTORY(ControlFlowGraphViewFactory, registerPlugin<KDevControlFlowGraphViewPlugin>();)
K_EXPORT_PLUGIN(ControlFlowGraphViewFactory(KAboutData("kdevcontrolflowgraphview","kdecontrolflowgraph", ki18n("Control Flow Graph"), "0.1", ki18n("Control flow graph support in KDevelop"), KAboutData::License_GPL)))

class KDevControlFlowGraphViewFactory: public KDevelop::IToolViewFactory{
public:
    KDevControlFlowGraphViewFactory(KDevControlFlowGraphViewPlugin *plugin) : mplugin(plugin) {}
    virtual QWidget* create(QWidget *parent = 0)
    {
	ControlFlowGraphView *cfgview = new ControlFlowGraphView(parent);

	DUChainControlFlow duccf;
	DotControlFlowGraph dcfg(cfgview->tempFile());

	QObject::connect(&duccf, SIGNAL(foundRootNode(const Declaration*)), &dcfg, SLOT(foundRootNode(const Declaration*)));
	QObject::connect(&duccf, SIGNAL(foundFunctionCall(const Declaration*, const Declaration*)), &dcfg, SLOT(foundFunctionCall(const Declaration*, const Declaration*)));
	QObject::connect(&duccf, SIGNAL(graphDone()), &dcfg, SLOT(graphDone()));

	duccf.controlFlowFromCurrentDefinition(0);
	cfgview->drawGraph();

        return cfgview;
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
    KDevControlFlowGraphViewPlugin *mplugin;
};

KDevControlFlowGraphViewPlugin::KDevControlFlowGraphViewPlugin (QObject *parent, const QVariantList &)
: KDevelop::IPlugin (ControlFlowGraphViewFactory::componentData(), parent)
{
    m_factory = new KDevControlFlowGraphViewFactory(this);
    core()->uiController()->addToolView("Control Flow Graph", m_factory);
}

KDevControlFlowGraphViewPlugin::~KDevControlFlowGraphViewPlugin()
{
}

#include "kdevcontrolflowgraphviewplugin.moc"
