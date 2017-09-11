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

#ifndef DUCHAINCONTROLFLOWINTERNALJOB_H
#define DUCHAINCONTROLFLOWINTERNALJOB_H

#include <ThreadWeaver/Job>
#include <QObject>

class DUChainControlFlow;
class KDevControlFlowGraphViewPlugin;

class DUChainControlFlowInternalJob : public QObject, public ThreadWeaver::Job
{
    Q_OBJECT

public:
    DUChainControlFlowInternalJob(DUChainControlFlow *duchainControlFlow, KDevControlFlowGraphViewPlugin *plugin);
    ~DUChainControlFlowInternalJob() override;

    enum ControlFlowJobType { ControlFlowJobInteractive, ControlFlowJobBatchForFunction, ControlFlowJobBatchForClass, ControlFlowJobBatchForProject };
    void setControlFlowJobType (ControlFlowJobType controlFlowJobType);

    void requestAbort() override;

Q_SIGNALS:
    void done();

protected:
    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread *thread) override;

private:
    DUChainControlFlow *m_duchainControlFlow;
    KDevControlFlowGraphViewPlugin *m_plugin;
    ControlFlowJobType m_controlFlowJobType;
};

#endif
