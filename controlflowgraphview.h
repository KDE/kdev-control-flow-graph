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

#ifndef CONTROLFLOWGRAPHVIEW_H
#define CONTROLFLOWGRAPHVIEW_H

#include "ui_controlflowgraphview.h"

#include <QPointer>

namespace KParts
{
    class ReadOnlyPart;
}
namespace KTextEditor
{
    class View;
    class Cursor;
}

class QGraphicsView;
class KDevControlFlowGraphViewPlugin;
class DUChainControlFlow;
class DotControlFlowGraph;

class ControlFlowGraphView : public QWidget, public Ui::ControlFlowGraphView
{
    Q_OBJECT
public:
    explicit ControlFlowGraphView (KDevControlFlowGraphViewPlugin *plugin, QWidget *parent = 0);
    virtual ~ControlFlowGraphView ();

    void refreshGraph();
    void newGraph();
public Q_SLOTS:
    void setProjectButtonsEnabled(bool enabled);
    void cursorPositionChanged(KTextEditor::View *view, const KTextEditor::Cursor &cursor);

    void exportControlFlowGraph();

    void updateLockIcon(bool checked);
    void setControlFlowClass(bool checked);
    void setControlFlowFunction(bool checked);
    void setControlFlowNamespace(bool checked);
    void setClusteringClass(bool checked);
    void setClusteringProject(bool checked);
    void setClusteringNamespace(bool checked);
    void setUseMaxLevel(bool checked);
    void setMaxLevel(int value);
    void setDrawIncomingArcs(bool checked);
    void setUseFolderName(bool checked);
    void setUseShortNames(bool checked);

private Q_SLOTS:
    void startingJob();
    void graphDone();

protected:
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);
private:
    KDevControlFlowGraphViewPlugin *m_plugin;
    QPointer<KParts::ReadOnlyPart>  m_part;
    QPointer<DotControlFlowGraph>   m_dotControlFlowGraph;
    QPointer<DUChainControlFlow>    m_duchainControlFlow;
    bool                            m_graphLocked;
};

#endif
