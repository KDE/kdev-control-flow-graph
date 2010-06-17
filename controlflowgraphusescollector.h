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

#ifndef CONTROLFLOWGRAPHUSESCOLLECTOR_H
#define CONTROLFLOWGRAPHUSESCOLLECTOR_H

#include <language/duchain/navigation/usescollector.h>

#include <language/codegen/coderepresentation.h>

using namespace KDevelop;

class ControlFlowGraphUsesCollector : public UsesCollector
{
    Q_OBJECT
public:
    ControlFlowGraphUsesCollector(IndexedDeclaration declaration);
    virtual ~ControlFlowGraphUsesCollector();
Q_SIGNALS:
    void processFunctionCall(Declaration *source, Declaration *target, const Use &use);
private:
    virtual void processUses(ReferencedTopDUContext topContext);
    void processContext(DUContext *context, CodeRepresentation::Ptr code);
protected:
    IndexedDeclaration m_declaration;
};

#endif
