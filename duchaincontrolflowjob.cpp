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

#include <KLocale>

#include <ThreadWeaver/Weaver>

#include "duchaincontrolflowinternaljob.h"

DUChainControlFlowJob::DUChainControlFlowJob(const QString &jobName, DUChainControlFlow *duchainControlFlow)
 : m_duchainControlFlow(duchainControlFlow),
   m_internalJob(0)
{
    setObjectName(i18n("Control flow graph generation for %1", jobName));
    setCapabilities(Killable);
}

DUChainControlFlowJob::~DUChainControlFlowJob()
{
    delete m_internalJob;
}

void DUChainControlFlowJob::start()
{
    m_internalJob = new DUChainControlFlowInternalJob(m_duchainControlFlow);
    connect(m_internalJob, SIGNAL(done(ThreadWeaver::Job*)), SLOT(done(ThreadWeaver::Job*)));
    ThreadWeaver::Weaver::instance()->enqueue(m_internalJob);
}

bool DUChainControlFlowJob::doKill()
{
    // Run controller stops all jobs with Quietly KillVerbosity, but plugins needs emitResult()
    emitResult();
    return true;
}

void DUChainControlFlowJob::done(ThreadWeaver::Job*)
{
    emitResult();
}
