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

#include "duchaincontrolflowinternaljob.h"

#include "duchaincontrolflow.h"
#include "kdevcontrolflowgraphviewplugin.h"

DUChainControlFlowInternalJob::DUChainControlFlowInternalJob(DUChainControlFlow *duchainControlFlow, KDevControlFlowGraphViewPlugin *plugin)
 : m_duchainControlFlow(duchainControlFlow),
   m_plugin(plugin),
   m_controlFlowJobType(DUChainControlFlowInternalJob::ControlFlowJobInteractive)
{
    kDebug();
}

DUChainControlFlowInternalJob::~DUChainControlFlowInternalJob()
{
    kDebug();
}

void DUChainControlFlowInternalJob::setControlFlowJobType(DUChainControlFlowInternalJob::ControlFlowJobType controlFlowJobType)
{
    m_controlFlowJobType = controlFlowJobType;   
}

void DUChainControlFlowInternalJob::requestAbort()
{
    kDebug();
    if (m_plugin)
    {
        kDebug() << "Requesting abort";
        m_plugin->requestAbort();
    }
}

void DUChainControlFlowInternalJob::run()
{
    switch(m_controlFlowJobType)
    {
        case ControlFlowJobInteractive:
        {
            if (m_duchainControlFlow)
                m_duchainControlFlow->run();
            break;
        }
        case ControlFlowJobBatchForFunction:
        {
            if (m_plugin)
                m_plugin->generateControlFlowGraph();
            break;
        }
        case ControlFlowJobBatchForClass:
        {
            if (m_plugin)
                m_plugin->generateClassControlFlowGraph();
            break;
        }
        case ControlFlowJobBatchForProject:
        {
            if (m_plugin)
                m_plugin->generateProjectControlFlowGraph();
            break;
        }
    };
}
