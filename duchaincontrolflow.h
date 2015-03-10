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

#ifndef DUCHAINCONTROLFLOW_H
#define DUCHAINCONTROLFLOW_H

#include <QSet>
#include <QHash>
#include <QPair>
#include <QPointer>

#include <language/duchain/ducontext.h>
#include <util/path.h>

class QPoint;

namespace KTextEditor {
    class View;
    class Cursor;
}
namespace KDevelop {
    class Use;
    class IndexedString;
    class DUContext;
    class Declaration;
    class TopDUContext;
    class IProject;
}

class KJob;

class DotControlFlowGraph;
class ControlFlowGraphUsesCollector;

using namespace KDevelop;

class DUChainControlFlow : public QObject
{
    Q_OBJECT
public:
    DUChainControlFlow(DotControlFlowGraph *dotControlFlowGraph);
    virtual ~DUChainControlFlow();

    enum ControlFlowMode { ControlFlowFunction, ControlFlowClass, ControlFlowNamespace };
    void setControlFlowMode(ControlFlowMode controlFlowMode);

    enum ClusteringMode
    {
        ClusteringNone      = 0x0,
        ClusteringClass     = 0x1,
        ClusteringNamespace = 0x2,
        ClusteringProject   = 0x4
    };
    Q_DECLARE_FLAGS(ClusteringModes, ClusteringMode)
    void setClusteringModes(ClusteringModes clusteringModes);
    ClusteringModes clusteringModes() const;

    void generateControlFlowForDeclaration(IndexedDeclaration idefinition, IndexedTopDUContext itopContext, IndexedDUContext iuppermostExecutableContext);
    bool isLocked();
    void run();

public Q_SLOTS:
    void cursorPositionChanged(KTextEditor::View *view, const KTextEditor::Cursor &cursor);
    void processFunctionCall(Declaration *source, Declaration *target, const Use &use);

    void slotGraphElementSelected(const QList<QString> list, const QPoint& point);
    void slotEdgeHover(QString label);

    void setLocked(bool locked);
    void setUseFolderName(bool useFolderName);
    void setUseShortNames(bool useFolderName);
    void setDrawIncomingArcs(bool drawIncomingArcs);
    void setMaxLevel(int maxLevel);
    void setShowUsesOnEdgeHover(bool checked);

    void refreshGraph();
    void newGraph();

private Q_SLOTS:
    void jobDone (KJob* job);

Q_SIGNALS:
    void startingJob();
    void jobDone();

private:
    void useDeclarationsFromDefinition(Declaration *definition, TopDUContext *topContext, DUContext *context);
    Declaration *declarationFromControlFlowMode(Declaration *definitionDeclaration);
    void prepareContainers(QStringList &containers, Declaration* definition);
    QString globalNamespaceOrFolderNames(Declaration *declaration);
    QString prependFolderNames(Declaration *declaration);
    QString shortNameFromContainers(const QList<QString> &containers, const QString &qualifiedIdentifier);
    void updateToolTip(const QString &edge, const QPoint& point, QWidget *partWidget);

    QPointer<DotControlFlowGraph> m_dotControlFlowGraph;
    IndexedDUContext m_previousUppermostExecutableContext;

    KTextEditor::View *m_currentView;
    IndexedDUContext m_currentContext;
    
    IndexedDeclaration m_definition;
    IndexedTopDUContext m_topContext;
    IndexedDUContext m_uppermostExecutableContext;
    
    QSet<IndexedDeclaration> m_visitedFunctions;
    QHash<QString, IndexedDeclaration> m_identifierDeclarationMap;
    QMultiHash<QString, QPair<RangeInRevision, IndexedString> > m_arcUsesMap;
    QPointer<KDevelop::IProject> m_currentProject;
    
    int  m_currentLevel;
    int  m_maxLevel;
    bool m_locked;
    bool m_drawIncomingArcs;
    bool m_useFolderName;
    bool m_useShortNames;
    bool m_ShowUsesOnEdgeHover;

    ControlFlowMode m_controlFlowMode;
    ClusteringModes m_clusteringModes;
    
    bool m_graphThreadRunning;
    bool m_abort;
    
    QPointer<ControlFlowGraphUsesCollector> m_collector;
    KDevelop::Path::List m_includeDirectories;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DUChainControlFlow::ClusteringModes)

#endif
