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

#include "dotcontrolflowgraph.h"

#include <QTemporaryFile>

#include <language/duchain/declaration.h>

DotControlFlowGraph::DotControlFlowGraph(QTemporaryFile *tempFile) : m_tempFile(tempFile)
{
    m_gvc = gvContext();
    m_graph = 0;
}

DotControlFlowGraph::~DotControlFlowGraph()
{
}

void DotControlFlowGraph::graphDone()
{
    if (m_graph)
    {
        gvLayout(m_gvc, m_graph, "dot");
        gvRenderFilename(m_gvc, m_graph, "dot", m_tempFile->fileName().toUtf8().data());
        gvFreeLayout(m_gvc, m_graph);
        agclose(m_graph);
    }
    gvFreeContext(m_gvc);
}

void DotControlFlowGraph::foundRootNode (const Declaration *definition)
{
    m_graph = agopen(definition->qualifiedIdentifier().toString().toUtf8().data(), AGDIGRAPH);
    Agnode_t *node = agnode(m_graph, definition->qualifiedIdentifier().toString().toUtf8().data());
    agsafeset(node, "shape", "box", "");
    QColor c = colorFromQualifiedIdentifier(definition->qualifiedIdentifier());
    char color[8];
    sprintf (color, "#%02x%02x%02x", c.red(), c.green(), c.blue());
    agsafeset(node, "fillcolor", color, "");
}

void DotControlFlowGraph::foundFunctionCall (const Declaration *source, const Declaration *target)
{
    Agnode_t* src = agnode(m_graph, source->qualifiedIdentifier().toString().toUtf8().data());
    Agnode_t* tgt = agnode(m_graph, target->qualifiedIdentifier().toString().toUtf8().data());
    char color[8];
    agsafeset(src, "shape", "box", "");
    QColor c = colorFromQualifiedIdentifier(source->qualifiedIdentifier());
    sprintf (color, "#%02x%02x%02x", c.red(), c.green(), c.blue());
    agsafeset(src, "fillcolor", color, "");
    agsafeset(tgt, "shape", "box", "");
    c = colorFromQualifiedIdentifier(target->qualifiedIdentifier());
    sprintf (color, "#%02x%02x%02x", c.red(), c.green(), c.blue());
    agsafeset(tgt, "fillcolor", color, "");
    agedge(m_graph, src, tgt);
}

const QColor& DotControlFlowGraph::colorFromQualifiedIdentifier(const KDevelop::QualifiedIdentifier &qualifiedIdentifier)
{
    if (m_colorMap.contains(qualifiedIdentifier.toString().split("::")[0]))
        return m_colorMap[qualifiedIdentifier.toString().split("::")[0]];
    else
        return m_colorMap[qualifiedIdentifier.toString().split("::")[0]] = QColor::fromHsv(qrand() % 256, 255, 190);
}

