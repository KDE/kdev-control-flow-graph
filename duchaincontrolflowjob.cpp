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

#include "duchaincontrolflowjob.h"

#include <ThreadWeaver/Weaver>

#include "duchaincontrolflowinternaljob.h"

DUChainControlFlowJob::DUChainControlFlowJob(DUChainControlFlow *duchainControlFlow)
 : m_duchainControlFlow(duchainControlFlow),
   m_internalJob(0)
{
}

DUChainControlFlowJob::~DUChainControlFlowJob()
{
}

void DUChainControlFlowJob::start()
{
    m_internalJob = new DUChainControlFlowInternalJob(m_duchainControlFlow);
    connect(m_internalJob, SIGNAL(done(ThreadWeaver::Job*)), SLOT(done(ThreadWeaver::Job*)));
    ThreadWeaver::Weaver::instance()->enqueue(m_internalJob);
}

void DUChainControlFlowJob::done(ThreadWeaver::Job*)
{
    delete m_internalJob;
    m_internalJob = 0;
    emitResult();
}
