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

#include <Job.h>

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
using namespace KDevelop;

class DUChainControlFlow : public ThreadWeaver::Job
{
    Q_OBJECT
public:
    DUChainControlFlow();
    virtual ~DUChainControlFlow();

    void run();
    
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
    
    void generateControlFlowForDeclaration(Declaration* definition, TopDUContext *topContext, DUContext *uppermostExecutableContext);
    bool isLocked();
Q_SIGNALS:
    void prepareNewGraph();
    void foundRootNode(const QStringList &containers, const QString &label);
    void foundFunctionCall(const QStringList &sourceContainers, const QString &source, const QStringList &targetContainers, const QString &target);
    void graphDone();
    void clearGraph();
    void updateToolTip(const QString &edge, const QPoint& point, QWidget *partWidget);
public Q_SLOTS:
    void cursorPositionChanged(KTextEditor::View *view, const KTextEditor::Cursor &cursor);
    void processFunctionCall(Declaration *source, Declaration *target, const Use &use);

    void slotUpdateToolTip(const QString &edge, const QPoint& point, QWidget *partWidget);
    void selectionIs(const QList<QString> list, const QPoint& point);

    void setLocked(bool locked);
    void setUseFolderName(bool useFolderName);
    void setUseShortNames(bool useFolderName);
    void setDrawIncomingArcs(bool drawIncomingArcs);
    void setMaxLevel(int maxLevel);

    void refreshGraph();
    void newGraph();
private:
    void useDeclarationsFromDefinition(Declaration *definition, TopDUContext *topContext, DUContext *context);
    Declaration *declarationFromControlFlowMode(Declaration *definitionDeclaration);
    void prepareContainers(QStringList &containers, Declaration* definition);
    QString globalNamespaceOrFolderNames(Declaration *declaration);
    QString prependFolderNames(Declaration *declaration);
    QString shortNameFromContainers(const QList<QString> &containers, const QString &qualifiedIdentifier);

    DUContext *m_previousUppermostExecutableContext;

    Declaration *m_definition;
    TopDUContext *m_topContext;
    DUContext *m_uppermostExecutableContext;
    
    QSet<Declaration *> m_visitedFunctions;
    QHash<QString, Declaration *> m_identifierDeclarationMap;
    QMultiHash<QString, QPair<Use, IndexedString> > m_arcUsesMap;
    KDevelop::IProject *m_currentProject;
    
    int  m_currentLevel;
    int  m_maxLevel;
    bool m_locked;
    bool m_drawIncomingArcs;
    bool m_useFolderName;
    bool m_useShortNames;

    ControlFlowMode m_controlFlowMode;
    ClusteringModes m_clusteringModes;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DUChainControlFlow::ClusteringModes)

#endif
