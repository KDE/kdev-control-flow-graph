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
{
}

DUChainControlFlow::~DUChainControlFlow()
{
}

void DUChainControlFlow::controlFlowFromCurrentDefinition (unsigned int maxLevel)
{
    m_maxLevel = maxLevel;
    m_currentLevel = 1;

    IDocument *doc = ICore::self()->documentController()->activeDocument();
    if (!doc) return;

    KTextEditor::Document *textDoc = doc->textDocument();
    if (!textDoc) return;

    KTextEditor::View *view = textDoc->activeView();
    if (!view) return;

    KDevelop::DUChainReadLocker lock(DUChain::lock());

    m_topContext = DUChainUtils::standardContextForUrl(doc->url());
    if (!m_topContext) return;

    SimpleCursor cursor(view->cursorPosition());
    DUContext *context = m_topContext->findContext(cursor);
    if (!context) return;
    if (context->type() != DUContext::Other) return;
    
    DUContext *uppermostExecutableContext = context;
    while (uppermostExecutableContext->parentContext()->type() == DUContext::Other)
        uppermostExecutableContext = uppermostExecutableContext->parentContext();

    Declaration* definition = 0;
    if (!uppermostExecutableContext || !uppermostExecutableContext->owner())
        return;
    else
        definition = uppermostExecutableContext->owner();

    if (!definition) return;
    emit foundRootNode(definition);

    if (m_maxLevel != 1)
    {
        ++m_currentLevel;
        useDeclarationsFromDefinition(definition, uppermostExecutableContext);
    }
    emit graphDone();
}

void DUChainControlFlow::useDeclarationsFromDefinition (const Declaration *definition, DUContext *context)
{
    const Use *uses = context->uses();
    unsigned int usesCount = context->usesCount();
    Declaration *declaration;
    FunctionDefinition *calledFunctionDefinition;
    DUContext *calledFunctionContext;

    QVector<DUContext *> subContexts = context->childContexts();
    QVector<DUContext *>::iterator subContextsIterator = subContexts.begin();
    QVector<DUContext *>::iterator subContextsEnd      = subContexts.end();

    for (unsigned int i = 0; i < usesCount; ++i)
    {
	declaration = m_topContext->usedDeclarationForIndex(uses[i].m_declarationIndex);
        if (declaration && declaration->type<KDevelop::FunctionType>())
	    if (subContextsIterator != subContextsEnd)
	    {
	        if (uses[i].m_range.start < (*subContextsIterator)->range().start)
		{
	            emit foundFunctionCall(definition, declaration);
		    calledFunctionDefinition = FunctionDefinition::definition(declaration);
		    if (!calledFunctionDefinition)
		        continue;
		    calledFunctionContext = calledFunctionDefinition->internalContext();
		    if (m_currentLevel < m_maxLevel || m_maxLevel == 0)
		    {
                        ++m_currentLevel;
                        useDeclarationsFromDefinition(calledFunctionDefinition, calledFunctionContext);
		    }
		}
	        else if ((*subContextsIterator)->type() == DUContext::Other)
		{
	            useDeclarationsFromDefinition(definition, *subContextsIterator);
		    subContextsIterator++;
		    --i;
		}
	    }
	    else
	    {
	        emit foundFunctionCall(definition, declaration);
	        calledFunctionDefinition = FunctionDefinition::definition(declaration);
	        if (!calledFunctionDefinition)
	            continue;
	        calledFunctionContext = calledFunctionDefinition->internalContext();
		if (m_currentLevel < m_maxLevel || m_maxLevel == 0)
		{
                    ++m_currentLevel;
                    useDeclarationsFromDefinition(calledFunctionDefinition, calledFunctionContext);
		}
	    }
    }
    while (subContextsIterator != subContextsEnd)
	if ((*subContextsIterator)->type() == DUContext::Other)
	{
            useDeclarationsFromDefinition(definition, *subContextsIterator);
	    subContextsIterator++;
	}
}

