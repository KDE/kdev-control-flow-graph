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

#include "duchaincontrolflow.h"

#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <kmessagebox.h>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>

#include <language/duchain/use.h>
#include <language/duchain/duchain.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchainutils.h>
#include <language/duchain/functiondefinition.h>
#include <language/duchain/types/functiontype.h>

using namespace KDevelop;

DUChainControlFlow::DUChainControlFlow()
: m_previousUppermostExecutableContext(0), m_maxLevel(0)
{
}

DUChainControlFlow::~DUChainControlFlow()
{
}

void DUChainControlFlow::cursorPositionChanged(KTextEditor::View *view, const KTextEditor::Cursor &cursor)
{
    m_currentLevel = 1;

    if (!view->document()) return;

    DUChainReadLocker lock(DUChain::lock());
    
    TopDUContext *topContext = DUChainUtils::standardContextForUrl(view->document()->url());
    if (!topContext) return;

    DUContext *context = topContext->findContext(KDevelop::SimpleCursor(cursor));
    if (!context)
    {
	if (m_previousUppermostExecutableContext != 0)
	{
	    emit clearGraph();
	    m_previousUppermostExecutableContext = 0;
	}
	return;
    }

    if (context->type() != DUContext::Other)
    {
	if (m_previousUppermostExecutableContext != 0)
	{
	    emit clearGraph();
	    m_previousUppermostExecutableContext = 0;
	}
	return;
    }

    DUContext *uppermostExecutableContext = context;
    while (uppermostExecutableContext->parentContext()->type() == DUContext::Other)
        uppermostExecutableContext = uppermostExecutableContext->parentContext();

    if (uppermostExecutableContext == m_previousUppermostExecutableContext)
	return;
    else
      	emit clearGraph();

    m_previousUppermostExecutableContext = uppermostExecutableContext;

    Declaration* definition = 0;
    if (!uppermostExecutableContext || !uppermostExecutableContext->owner())
        return;
    else
        definition = uppermostExecutableContext->owner();

    if (!definition) return;

    m_visitedFunctions.clear();
    m_arcs.clear();
    m_identifierDeclarationMap.clear();
    emit clearGraph();
    emit foundRootNode(definition);

    if (m_maxLevel != 1)
    {
        ++m_currentLevel;
	m_visitedFunctions.insert(definition);
        useDeclarationsFromDefinition(definition, topContext, uppermostExecutableContext);
    }
    emit graphDone();
}

void DUChainControlFlow::useDeclarationsFromDefinition (Declaration *definition, TopDUContext *topContext, DUContext *context)
{
    if (!topContext) return;

    DUChainReadLocker lock(DUChain::lock());

    const Use *uses = context->uses();
    unsigned int usesCount = context->usesCount();
    QVector<DUContext *> subContexts = context->childContexts();
    QVector<DUContext *>::iterator subContextsIterator = subContexts.begin();
    QVector<DUContext *>::iterator subContextsEnd      = subContexts.end();

    m_identifierDeclarationMap[definition->qualifiedIdentifier().toString()] = definition;

    Declaration *declaration;
    for (unsigned int i = 0; i < usesCount; ++i)
    {
	declaration = topContext->usedDeclarationForIndex(uses[i].m_declarationIndex);
        if (declaration && declaration->type<KDevelop::FunctionType>())
        {
	    if (subContextsIterator != subContextsEnd)
	    {
	        if (uses[i].m_range.start < (*subContextsIterator)->range().start)
		    processFunctionCall(definition, declaration);
	        else if ((*subContextsIterator)->type() == DUContext::Other)
		{
	            useDeclarationsFromDefinition(definition, topContext, *subContextsIterator);
		    subContextsIterator++;
		    --i;
		}
	    }
	    else
		processFunctionCall(definition, declaration);
        }
    }
    while (subContextsIterator != subContextsEnd)
	if ((*subContextsIterator)->type() == DUContext::Other)
	{
            useDeclarationsFromDefinition(definition, topContext, *subContextsIterator);
	    subContextsIterator++;
	}
}

void DUChainControlFlow::processFunctionCall(Declaration *definition, Declaration *declaration)
{
    FunctionDefinition *calledFunctionDefinition;
    DUContext *calledFunctionContext;

    // For prevent duplicated arcs
    QPair<Declaration *, Declaration *> pair(definition, declaration);
    if (!m_arcs.contains(pair))
    {
	m_arcs.insert(pair);
	emit foundFunctionCall(definition, declaration);
    }

    DUChainReadLocker lock(DUChain::lock());

    calledFunctionDefinition = FunctionDefinition::definition(declaration);
    if (!calledFunctionDefinition) return;
    calledFunctionContext = calledFunctionDefinition->internalContext();
    if (calledFunctionContext && (m_currentLevel < m_maxLevel || m_maxLevel == 0))
    {
	// For prevent endless loop in recursive methods
	if (!m_visitedFunctions.contains(calledFunctionDefinition))
	{
	    ++m_currentLevel;
	    m_visitedFunctions.insert(calledFunctionDefinition);
	    useDeclarationsFromDefinition(calledFunctionDefinition, calledFunctionDefinition->topContext(), calledFunctionContext);
	}
    }
}

void DUChainControlFlow::viewDestroyed(QObject * object)
{
    Q_UNUSED(object);
    if (!ICore::self()->documentController()->activeDocument())
        emit clearGraph();
}

void DUChainControlFlow::focusIn(KTextEditor::View *view)
{
    cursorPositionChanged(view, view->cursorPosition());
}

void DUChainControlFlow::selectionIs(const QList<QString> list, const QPoint& point)
{
/*
    if (!list.isEmpty())
    {
	Declaration *definition = m_identifierDeclarationMap[list[0]];
	if (definition)
	{
	    KUrl url(definition->url().str());
	    KTextEditor::Range range = definition->range().textRange();
	    ICore::self()->documentController()->openDocument(url, range.start());
	}
    }
*/
}
