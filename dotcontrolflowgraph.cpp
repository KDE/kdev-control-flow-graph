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

#include "dotcontrolflowgraph.h"

#include <language/duchain/declaration.h>
#include <cstdio>

namespace {
    // C interface takes char*, so to avoid deprecated cast and/or undefined behaviour,
    // defined the needed constants here.
    static char SUFFIX[] = "dot";
    static char GRAPH_NAME[] = "Root_Graph";
    static char LABEL[] = "label";
    static char EMPTY[] = "";
    static char FILLED[] = "filled";
    static char FILLCOLOR[] = "fillcolor";
    static char SHAPE[] = "shape";
    static char STYLE[] = "style";
    static char BOX[] = "box";
}

DotControlFlowGraph::DotControlFlowGraph() : m_rootGraph(0)
{
    m_gvc = gvContext();
}

DotControlFlowGraph::~DotControlFlowGraph()
{
    gvFreeContext(m_gvc);
}

void DotControlFlowGraph::graphDone()
{
    if (m_rootGraph)
    {
        gvLayout(m_gvc, m_rootGraph, SUFFIX);
        gvFreeLayout(m_gvc, m_rootGraph);
        emit loadLibrary(m_rootGraph);
    }
}

void DotControlFlowGraph::clearGraph()
{
    if (m_rootGraph)
    {
        gvFreeLayout(m_gvc, m_rootGraph);
        agclose(m_rootGraph);
        m_rootGraph = 0;
    }

    m_namedGraphs.clear();
    m_rootGraph = agopen(GRAPH_NAME, AGDIGRAPHSTRICT);
    graphDone();
}

void DotControlFlowGraph::exportGraph(const QString &fileName)
{
    if (m_rootGraph)
    {
        gvLayout(m_gvc, m_rootGraph, SUFFIX);
        gvRenderFilename(m_gvc, m_rootGraph, fileName.right(fileName.size()-fileName.lastIndexOf('.')-1).toUtf8().data(), fileName.toUtf8().data());
        gvFreeLayout(m_gvc, m_rootGraph);
    }
}

void DotControlFlowGraph::prepareNewGraph()
{
    clearGraph();
}

void DotControlFlowGraph::foundRootNode (const QStringList &containers, const QString &label)
{
    Agraph_t *graph = m_rootGraph;
    if (!m_rootGraph) {
        // This shouldn't happen, as the graph should be generated before this function
        // is connected.
        Q_ASSERT(false);
        return;
    }
    QString absoluteContainer;
    foreach (const QString& container, containers)
    {
        absoluteContainer += container;
        graph = m_namedGraphs[absoluteContainer] = agsubg(graph, ("cluster_" + absoluteContainer).toUtf8().data());
        agsafeset(graph, LABEL, container.toUtf8().data(), EMPTY);
    }

    Agnode_t *node = agnode(graph, label.toUtf8().data());
    agsafeset(node, SHAPE, BOX, EMPTY);
    QColor c = colorFromQualifiedIdentifier(label);
    char color[8];
    std::sprintf (color, "#%02x%02x%02x", c.red(), c.green(), c.blue());
    agsafeset(node, STYLE, FILLED, EMPTY);
    agsafeset(node, FILLCOLOR, color, EMPTY);
}

void DotControlFlowGraph::foundFunctionCall (const QStringList &sourceContainers, const QString &source, const QStringList &targetContainers, const QString &target)
{
    if (!m_rootGraph) {
        // This shouldn't happen, as the graph should be generated before this function
        // is connected.
        Q_ASSERT(false);
        return;
    }
    Agraph_t *sourceGraph, *targetGraph;
    sourceGraph = targetGraph = m_rootGraph;
    QString absoluteContainer;

    foreach (const QString& container, sourceContainers)
    {
        Agraph_t *previousGraph = sourceGraph;
        absoluteContainer += container;
        if (!(sourceGraph = m_namedGraphs[absoluteContainer]))
        {
            sourceGraph = m_namedGraphs[absoluteContainer] = agsubg(previousGraph, ("cluster_" + absoluteContainer).toUtf8().data());
            agsafeset(sourceGraph, LABEL, container.toUtf8().data(), EMPTY);
        }
    }
    absoluteContainer.clear();
    foreach (const QString& container, targetContainers)
    {
        Agraph_t *previousGraph = targetGraph;
        absoluteContainer += container;
        if (!(targetGraph = m_namedGraphs[absoluteContainer]))
        {
            targetGraph = m_namedGraphs[absoluteContainer] = agsubg(previousGraph, ("cluster_" + absoluteContainer).toUtf8().data());
            agsafeset(targetGraph, LABEL, container.toUtf8().data(), EMPTY);
        }
    }

    Agnode_t* src = agnode(sourceGraph, source.toUtf8().data());
    Agnode_t* tgt = agnode(targetGraph, target.toUtf8().data());

    char color[8];
    char ID[] = "id";
    agsafeset(src, SHAPE, BOX, EMPTY);
    QColor c = colorFromQualifiedIdentifier(source);
    std::sprintf (color, "#%02x%02x%02x", c.red(), c.green(), c.blue());
    agsafeset(src, STYLE, FILLED, EMPTY);
    agsafeset(src, FILLCOLOR, color, EMPTY);
    agsafeset(tgt, SHAPE, BOX, EMPTY);
    c = colorFromQualifiedIdentifier(target);
    std::sprintf (color, "#%02x%02x%02x", c.red(), c.green(), c.blue());
    agsafeset(tgt, STYLE, FILLED, EMPTY);
    agsafeset(tgt, FILLCOLOR, color, EMPTY);
    Agedge_t* edge;
    if (sourceGraph == targetGraph)
        edge = agedge(sourceGraph, src, tgt);
    else
        edge = agedge(m_rootGraph, src, tgt);
    agsafeset(edge, ID, (source + "->" + target).toUtf8().data(), EMPTY);
}

const QColor& DotControlFlowGraph::colorFromQualifiedIdentifier(const QString &label)
{
    if (m_colorMap.contains(label.split("::")[0]))
        return m_colorMap[label.split("::")[0]];
    else
        return m_colorMap[label.split("::")[0]] = QColor::fromHsv(qrand() % 256, 255, 190);
}

