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

#include <cstdio>

#include <language/duchain/declaration.h>

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

QMutex DotControlFlowGraph::mutex;

DotControlFlowGraph::DotControlFlowGraph()
    : m_rootGraph(nullptr)
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
        if (mutex.tryLock())
        {
            gvLayout(m_gvc, m_rootGraph, SUFFIX);
            gvFreeLayout(m_gvc, m_rootGraph);
            mutex.unlock();
            emit loadLibrary(m_rootGraph);
        }
    }
}

void DotControlFlowGraph::clearGraph()
{
    if (m_rootGraph)
    {
        gvFreeLayout(m_gvc, m_rootGraph);
        agclose(m_rootGraph);
        m_rootGraph = nullptr;
    }

    m_namedGraphs.clear();
    m_rootGraph = agopen(GRAPH_NAME, Agdirected, nullptr);
    graphDone();
}

void DotControlFlowGraph::exportGraph(const QString &fileName)
{
    if (m_rootGraph)
    {
        gvLayout(m_gvc, m_rootGraph, SUFFIX);
        gvRenderFilename(m_gvc, m_rootGraph, fileName.right(fileName.size()-fileName.lastIndexOf(QLatin1Char('.'))-1).toUtf8().data(), fileName.toUtf8().data());
        gvFreeLayout(m_gvc, m_rootGraph);
    }
}

void DotControlFlowGraph::prepareNewGraph()
{
    clearGraph();
}

void DotControlFlowGraph::foundRootNode(const QStringList &containers, const QString &label)
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
        graph = m_namedGraphs[absoluteContainer] = agsubg(graph, (QLatin1String("cluster_") + absoluteContainer).toUtf8().data(), 1);
        agsafeset(graph, LABEL, container.toUtf8().data(), EMPTY);
    }

    Agnode_t *node = agnode(graph, (containers.join(QString()) + label).toUtf8().data(), 1);
    agsafeset(node, SHAPE, BOX, EMPTY);
    QColor c = colorFromQualifiedIdentifier(label);
    char color[8];
    std::sprintf (color, "#%02x%02x%02x", c.red(), c.green(), c.blue());
    agsafeset(node, STYLE, FILLED, EMPTY);
    agsafeset(node, FILLCOLOR, color, EMPTY);
    agsafeset(node, LABEL, label.toUtf8().data(), EMPTY);
}

void DotControlFlowGraph::foundFunctionCall(const QStringList &sourceContainers, const QString &source, const QStringList &targetContainers, const QString &target)
{
    if (!m_rootGraph) {
        // This shouldn't happen, as the graph should be generated before this function
        // is connected.
        Q_ASSERT(false);
        return;
    }
    Agraph_t *sourceGraph, *targetGraph, *newGraph;
    sourceGraph = targetGraph = m_rootGraph;
    QString absoluteContainer;

    foreach (const QString& container, sourceContainers)
    {
        Agraph_t *previousGraph = sourceGraph;
        absoluteContainer += container;
        if (!m_namedGraphs.contains(absoluteContainer))
        {
            newGraph = agsubg(previousGraph, (QLatin1String("cluster_") + absoluteContainer).toUtf8().data(), 1);
            m_namedGraphs.insert(absoluteContainer, newGraph);
            agsafeset(newGraph, LABEL, container.toUtf8().data(), EMPTY);
        }
        sourceGraph = m_namedGraphs[absoluteContainer];
    }
    absoluteContainer.clear();
    foreach (const QString& container, targetContainers)
    {
        Agraph_t *previousGraph = targetGraph;
        absoluteContainer += container;
        if (!m_namedGraphs.contains(absoluteContainer))
        {
            Agraph_t *newGraph = agsubg(previousGraph, (QLatin1String("cluster_") + absoluteContainer).toUtf8().data(), 1);
            m_namedGraphs.insert(absoluteContainer, newGraph);
            agsafeset(newGraph, LABEL, container.toUtf8().data(), EMPTY);
        }
        targetGraph = m_namedGraphs[absoluteContainer];
    }

    Agnode_t* src = agnode(sourceGraph, (sourceContainers.join(QString()) + source).toUtf8().data(), 1);
    Agnode_t* tgt = agnode(targetGraph, (targetContainers.join(QString()) + target).toUtf8().data(), 1);

    char color[8];
    char ID[] = "id";

    QColor c = colorFromQualifiedIdentifier(source);
    std::sprintf (color, "#%02x%02x%02x", c.red(), c.green(), c.blue());
    agsafeset(src, STYLE, FILLED, EMPTY);
    agsafeset(src, FILLCOLOR, color, EMPTY);
    agsafeset(src, SHAPE, BOX, EMPTY);
    agsafeset(src, LABEL, source.toUtf8().data(), EMPTY);

    c = colorFromQualifiedIdentifier(target);
    std::sprintf (color, "#%02x%02x%02x", c.red(), c.green(), c.blue());
    agsafeset(tgt, STYLE, FILLED, EMPTY);
    agsafeset(tgt, FILLCOLOR, color, EMPTY);
    agsafeset(tgt, SHAPE, BOX, EMPTY);
    agsafeset(tgt, LABEL, target.toUtf8().data(), EMPTY);

    Agedge_t* edge;
    if (sourceGraph == targetGraph)
        edge = agedge(sourceGraph, src, tgt, nullptr, 1);
    else
        edge = agedge(m_rootGraph, src, tgt, nullptr, 1);
    agsafeset(edge, ID, (source + QLatin1String("->") + target).toUtf8().data(), EMPTY);
}

const QColor& DotControlFlowGraph::colorFromQualifiedIdentifier(const QString &label)
{
    const QString qid = label.split(QLatin1String("::")).value(0);
    auto it = m_colorMap.find(qid);
    if (it == m_colorMap.end()) {
        it = m_colorMap.insert(qid, QColor::fromHsv(qrand() % 256, 255, 190));
    }
    return *it;
}
