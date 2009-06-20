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

#include "controlflowgraphview.h"

#include <kparts/part.h>
#include <klibloader.h>
#include <kservice.h>
#include <kmessagebox.h>
#include <kactioncollection.h>

#include "duchaincontrolflow.h"
#include "dotcontrolflowgraph.h"

ControlFlowGraphView::ControlFlowGraphView(QWidget *parent)
: QWidget(parent), m_part(0),
m_duchainControlFlow(new DUChainControlFlow), m_dotControlFlowGraph(new DotControlFlowGraph)
{
    setupUi(this);
    KLibFactory *factory = KLibLoader::self()->factory("kgraphviewerpart");
    if (factory)
    {
        m_part = factory->create<KParts::ReadOnlyPart>(this);
  	if (m_part)
	{
	    horizontalLayout->addWidget(m_part->widget());
	    connect(zoomoutToolButton, SIGNAL(clicked()), m_part->actionCollection()->action("view_zoom_out"), SIGNAL(triggered()));
	    connect(zoominToolButton, SIGNAL(clicked()), m_part->actionCollection()->action("view_zoom_in"), SIGNAL(triggered()));

	    connect(m_duchainControlFlow,  SIGNAL(foundRootNode(const Declaration*)),
                    m_dotControlFlowGraph, SLOT  (foundRootNode(const Declaration*)));
	    connect(m_duchainControlFlow,  SIGNAL(foundFunctionCall(const Declaration*, const Declaration*)),
                    m_dotControlFlowGraph, SLOT  (foundFunctionCall(const Declaration*, const Declaration*)));
	    connect(m_duchainControlFlow,  SIGNAL(graphDone()), m_dotControlFlowGraph, SLOT(graphDone()));
	    connect(m_dotControlFlowGraph, SIGNAL(graphSaved(const KUrl &)), m_part, SLOT(openUrl(const KUrl &)));

	    m_duchainControlFlow->controlFlowFromCurrentDefinition(0);
	}
        else
	    KMessageBox::error(this, i18n("Could not load the KGraphViewer kpart"));
    }
    else
        KMessageBox::error(this, i18n("Could not find the KGraphViewer factory"));
}

ControlFlowGraphView::~ControlFlowGraphView()
{
    if (m_duchainControlFlow != 0) delete m_duchainControlFlow;
    if (m_dotControlFlowGraph != 0) delete m_dotControlFlowGraph;
    if (m_part != 0) delete m_part;
}
