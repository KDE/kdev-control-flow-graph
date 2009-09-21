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
	gvLayout(m_gvc, m_rootGraph, "dot");
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
    m_rootGraph = agopen((char *) "Root_Graph", AGDIGRAPHSTRICT);
    graphDone();
}

void DotControlFlowGraph::exportGraph(const QString &fileName)
{
    if (m_rootGraph)
    {
	gvLayout(m_gvc, m_rootGraph, "dot");
	gvRenderFilename(m_gvc, m_rootGraph, fileName.right(fileName.size()-fileName.lastIndexOf('.')-1).toUtf8().data(), fileName.toUtf8().data());
	gvFreeLayout(m_gvc, m_rootGraph);
    }
}

void DotControlFlowGraph::prepareNewGraph()
{
    clearGraph();
    m_rootGraph = agopen((char *) "Root_Graph", AGDIGRAPHSTRICT);
}

void DotControlFlowGraph::foundRootNode (const QStringList &containers, const QString &label)
{
    Agraph_t *graph = m_rootGraph;
    QString absoluteContainer;
    foreach (const QString& container, containers)
    {
	absoluteContainer += container;
	graph = m_namedGraphs[absoluteContainer] = agsubg(graph, ("cluster_" + absoluteContainer).toUtf8().data());
	agsafeset(graph, (char *) "label", container.toUtf8().data(), (char *) "");
    }

    Agnode_t *node = agnode(graph, label.toUtf8().data());
    agsafeset(node, (char *) "shape", (char *) "box", (char *) "");
    QColor c = colorFromQualifiedIdentifier(label);
    char color[8];
    sprintf (color, "#%02x%02x%02x", c.red(), c.green(), c.blue());
    agsafeset(node, (char *) "style", (char *) "filled", (char *) "");
    agsafeset(node, (char *) "fillcolor", color, (char *) "");
}

void DotControlFlowGraph::foundFunctionCall (const QStringList &sourceContainers, const QString &source, const QStringList &targetContainers, const QString &target)
{
    Agraph_t *sourceGraph, *targetGraph, *previousGraph;
    sourceGraph = targetGraph = m_rootGraph;
    QString absoluteContainer;

    foreach (const QString& container, sourceContainers)
    {
	previousGraph = sourceGraph;
	absoluteContainer += container;
	if (!(sourceGraph = m_namedGraphs[absoluteContainer]))
	{
	    sourceGraph = m_namedGraphs[absoluteContainer] = agsubg(previousGraph, ("cluster_" + absoluteContainer).toUtf8().data());
	    agsafeset(sourceGraph, (char *) "label", container.toUtf8().data(), (char *) "");
	}
    }
    absoluteContainer.clear();
    foreach (const QString& container, targetContainers)
    {
	previousGraph = targetGraph;
	absoluteContainer += container;
	if (!(targetGraph = m_namedGraphs[absoluteContainer]))
	{
	    targetGraph = m_namedGraphs[absoluteContainer] = agsubg(previousGraph, ("cluster_" + absoluteContainer).toUtf8().data());
	    agsafeset(targetGraph, (char *) "label", container.toUtf8().data(), (char *) "");
	}
    }

    Agnode_t* src = agnode(sourceGraph, source.toUtf8().data());
    Agnode_t* tgt = agnode(targetGraph, target.toUtf8().data());

    char color[8];
    agsafeset(src, (char *) "shape", (char *) "box", (char *) "");
    QColor c = colorFromQualifiedIdentifier(source);
    sprintf (color, "#%02x%02x%02x", c.red(), c.green(), c.blue());
    agsafeset(src, (char *) "style", (char *) "filled", (char *) "");
    agsafeset(src, (char *) "fillcolor", color, (char *) "");
    agsafeset(tgt, (char *) "shape", (char *) "box", (char *) "");
    c = colorFromQualifiedIdentifier(target);
    sprintf (color, "#%02x%02x%02x", c.red(), c.green(), c.blue());
    agsafeset(tgt, (char *) "style", (char *) "filled", (char *) "");
    agsafeset(tgt, (char *) "fillcolor", color, (char *) "");
    Agedge_t* edge;
    if (sourceGraph == targetGraph)
	edge = agedge(sourceGraph, src, tgt);
    else
	edge = agedge(m_rootGraph, src, tgt);
    agsafeset(edge, (char *) "id", (source + "->" + target).toUtf8().data(), (char *) "");
}

const QColor& DotControlFlowGraph::colorFromQualifiedIdentifier(const QString &label)
{
    if (m_colorMap.contains(label.split("::")[0]))
        return m_colorMap[label.split("::")[0]];
    else
        return m_colorMap[label.split("::")[0]] = QColor::fromHsv(qrand() % 256, 255, 190);
}
