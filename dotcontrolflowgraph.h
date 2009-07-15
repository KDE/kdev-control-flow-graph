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

#ifndef _DOTCONTROLFLOWGRAPH_H_
#define _DOTCONTROLFLOWGRAPH_H_

#include <QObject>
#include <QMap>
#include <QColor>

#include <graphviz/gvc.h>

namespace KDevelop {
    class Declaration;
    class QualifiedIdentifier;
}
using namespace KDevelop;

class DotControlFlowGraph : public QObject
{
    Q_OBJECT
public:
    DotControlFlowGraph();
    virtual ~DotControlFlowGraph();
Q_SIGNALS:
    bool loadLibrary(graph_t* graph);
public Q_SLOTS:
    void foundRootNode (const QString &label);
    void foundFunctionCall (const QString &source, const QString &target);
    void graphDone();
    void clearGraph();
private:
    GVC_t *m_gvc;
    graph_t *m_graph;
    QMap<QString, QColor> m_colorMap;

    const QColor& colorFromQualifiedIdentifier(const QString &label);
};

#endif
