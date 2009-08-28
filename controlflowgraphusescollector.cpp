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

#include "controlflowgraphusescollector.h"

#include <language/duchain/use.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>

using namespace KDevelop;

ControlFlowGraphUsesCollector::ControlFlowGraphUsesCollector(IndexedDeclaration declaration)
 : UsesCollector(declaration), m_declaration(declaration)
{
}

void ControlFlowGraphUsesCollector::processUses(ReferencedTopDUContext topContext)
{
    if (topContext.data())
    {
	CodeRepresentation::Ptr code = createCodeRepresentation(topContext.data()->url());
	processContext(topContext.data(), code);
    }
}

void ControlFlowGraphUsesCollector::processContext(DUContext *context, CodeRepresentation::Ptr code)
{
    DUChainReadLocker lock(DUChain::lock());
    
    foreach (const IndexedDeclaration &declaration, declarations())
    {
	int declarationIndex = context->topContext()->indexForUsedDeclaration(declaration.data(), false);
	for (int useIndex = 0; useIndex < context->usesCount(); ++useIndex)
	    if (context->uses()[useIndex].m_declarationIndex == declarationIndex)
	    {
		// Navigate to uppermost executable context
		DUContext *uppermostExecutableContext = context;
		while (uppermostExecutableContext->parentContext() && uppermostExecutableContext->parentContext()->type() == DUContext::Other)
		    uppermostExecutableContext = uppermostExecutableContext->parentContext();

		// Get the definition
		Declaration* definition = 0;
		if (!uppermostExecutableContext || !uppermostExecutableContext->owner())
		    continue;
		else
		    definition = uppermostExecutableContext->owner();

		if (!definition) continue;

		emit processFunctionCall(definition, m_declaration.data(), context->uses()[useIndex]);
	    }
    }
    foreach (DUContext *child, context->childContexts())
	processContext(child, code);
}
