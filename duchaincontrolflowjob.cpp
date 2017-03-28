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

#include <ThreadWeaver/Queue>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iruncontroller.h>

#include <klocalizedstring.h>
#include <QDebug>
DUChainControlFlowJob::DUChainControlFlowJob(const QString &jobName, DUChainControlFlow *duchainControlFlow)
 : m_duchainControlFlow(duchainControlFlow),
   m_plugin(nullptr),
   m_internalJob(nullptr),
   m_controlFlowJobType(DUChainControlFlowInternalJob::ControlFlowJobInteractive)
{
    init(jobName);
}

DUChainControlFlowJob::DUChainControlFlowJob(const QString &jobName, KDevControlFlowGraphViewPlugin *plugin)
 : m_duchainControlFlow(nullptr),
   m_plugin(plugin),
   m_internalJob(nullptr),
   m_controlFlowJobType(DUChainControlFlowInternalJob::ControlFlowJobInteractive)
{
    init(jobName);
}

void DUChainControlFlowJob::init(const QString &jobName)
{
    setObjectName(i18n("Generating control flow graph for %1", jobName));
    setCapabilities(Killable);    
    setAutoDelete(false);
    ICore::self()->uiController()->registerStatus(this);
}

DUChainControlFlowJob::~DUChainControlFlowJob()
{
}

QString DUChainControlFlowJob::statusName() const
{
    return i18n("Control Flow Graph");
}

void DUChainControlFlowJob::setControlFlowJobType(DUChainControlFlowInternalJob::ControlFlowJobType controlFlowJobType)
{
    m_controlFlowJobType = controlFlowJobType;   
}

void DUChainControlFlowJob::start()
{
    emit showProgress(this, 0, 0, 0);
    emit showMessage(this, objectName());

    m_internalJob = new DUChainControlFlowInternalJob(m_duchainControlFlow, m_plugin);
    m_internalJob->setControlFlowJobType(m_controlFlowJobType);
    connect(m_internalJob, &DUChainControlFlowInternalJob::done,
            this, &DUChainControlFlowJob::done);
    ThreadWeaver::Queue::instance()->enqueue(ThreadWeaver::JobPointer(m_internalJob));
}

bool DUChainControlFlowJob::doKill()
{
    m_internalJob->requestAbort();
    return false;
}

void DUChainControlFlowJob::done()
{
    emit hideProgress(this);
    emit clearMessage(this);

    emitResult();
}
