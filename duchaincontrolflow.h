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

namespace KDevelop {
    class DUContext;
    class Declaration;
    class TopDUContext;
}
using namespace KDevelop;

class DUChainControlFlow : public QObject
{
    Q_OBJECT
public:
    DUChainControlFlow();
    virtual ~DUChainControlFlow();

    void controlFlowFromCurrentDefinition (unsigned int maxLevel = 0);
Q_SIGNALS:
    void foundRootNode (const Declaration *definition);
    void foundFunctionCall (const Declaration *source, const Declaration *target);
    void graphDone();
    void clearGraph();
private:
    void useDeclarationsFromDefinition(const Declaration *definition, DUContext *context);
    TopDUContext *m_topContext;
    DUContext *m_previousUppermostExecutableContext;
    unsigned int m_currentLevel;
    unsigned int m_maxLevel;
};

#endif
