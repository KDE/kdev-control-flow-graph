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

#ifndef DUCHAINCONTROLFLOWJOB_H
#define DUCHAINCONTROLFLOWJOB_H

#include <QPointer>

#include <KJob>

#include <interfaces/istatus.h>
#include <ThreadWeaver/QObjectDecorator>
#include "duchaincontrolflowinternaljob.h"

class DUChainControlFlow;
class DUChainControlFlowInternalJob;
class KDevControlFlowGraphViewPlugin;

using namespace KDevelop;

class DUChainControlFlowJob : public KJob, public IStatus
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IStatus)
public:
    DUChainControlFlowJob(const QString &jobName, DUChainControlFlow *duchainControlFlow);
    DUChainControlFlowJob(const QString &jobName, KDevControlFlowGraphViewPlugin *plugin);
    virtual ~DUChainControlFlowJob();

    virtual QString statusName() const;

    void setControlFlowJobType(DUChainControlFlowInternalJob::ControlFlowJobType controlFlowJobType);
    
    virtual void start();
    virtual bool doKill();
Q_SIGNALS:
    // Implementations of IStatus signals
    void clearMessage(KDevelop::IStatus *);
    void showMessage(KDevelop::IStatus *, const QString &message, int timeout = 0);
    void hideProgress(KDevelop::IStatus *);
    void showProgress(KDevelop::IStatus *, int minimum, int maximum, int value);
    void showErrorMessage(const QString &, int);
private Q_SLOTS:
    void done();
private:
    void init(const QString &jobName);
    DUChainControlFlow *m_duchainControlFlow;
    KDevControlFlowGraphViewPlugin *m_plugin;
    QPointer<DUChainControlFlowInternalJob> m_internalJob;
    DUChainControlFlowInternalJob::ControlFlowJobType m_controlFlowJobType;
};

#endif
