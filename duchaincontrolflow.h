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

#ifndef _DUCHAINCONTROLFLOW_H_
#define _DUCHAINCONTROLFLOW_H_

#include <QObject>
#include <QSet>
#include <QHash>
#include <QPair>

class QPoint;

namespace KTextEditor {
    class View;
    class Cursor;
};
namespace KDevelop {
    class DUContext;
    class Declaration;
    class TopDUContext;
};
using namespace KDevelop;

class DUChainControlFlow : public QObject
{
    Q_OBJECT
public:
    DUChainControlFlow();
    virtual ~DUChainControlFlow();
    enum ControlFlowMode { ControlFlowFunction, ControlFlowClass, ControlFlowNamespace };
Q_SIGNALS:
    void foundRootNode (const QString &label);
    void foundFunctionCall (const QString &source, const QString &target);
    void graphDone();
    void clearGraph();
public Q_SLOTS:
    void cursorPositionChanged(KTextEditor::View *view, const KTextEditor::Cursor &cursor);
    void viewDestroyed(QObject *object);
    void focusIn(KTextEditor::View *view);
    void selectionIs(const QList<QString> list, const QPoint& point);
private:
    void useDeclarationsFromDefinition(Declaration *definition, TopDUContext *topContext, DUContext *context);
    void processFunctionCall(Declaration *source, Declaration *target);
    Declaration *declarationFromControlFlowMode(Declaration *definitionDeclaration);
    void newGraph();

    DUContext *m_previousUppermostExecutableContext;
    QSet<Declaration *> m_visitedFunctions;
    QHash<QString, Declaration *> m_identifierDeclarationMap;
    unsigned int m_currentLevel;
    unsigned int m_maxLevel;
    bool m_navigating;
    ControlFlowMode m_controlFlowMode;
};

#endif
