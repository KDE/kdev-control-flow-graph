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

#ifndef DOTCONTROLFLOWGRAPH_H
#define DOTCONTROLFLOWGRAPH_H

#include <QObject>
#include <QMap>
#include <QHash>
#include <QColor>

#include <graphviz/gvc.h>

namespace KDevelop {
    class Declaration;
    class QualifiedIdentifier;
}
using namespace KDevelop;

class QTemporaryFile;
class KUrl;

class DotControlFlowGraph : public QObject
{
    Q_OBJECT
public:
    DotControlFlowGraph();
    virtual ~DotControlFlowGraph();
Q_SIGNALS:
    bool openUrl(const KUrl &url);
public Q_SLOTS:
    void foundRootNode (const QStringList &containers, const QString &label);
    void foundFunctionCall (const QStringList &sourceContainers, const QString &source, const QStringList &targetContainers, const QString &target);
    void graphDone();
    void clearGraph();
    void exportGraph(const QString &fileName);
private:
    GVC_t *m_gvc;
    Agraph_t *m_rootGraph;
    QMap<QString, QColor> m_colorMap;
    QHash<QString, Agraph_t *> m_namedGraphs;
    QTemporaryFile *m_tempFile;

    const QColor& colorFromQualifiedIdentifier(const QString &label);
};

#endif
