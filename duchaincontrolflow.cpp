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
#include <interfaces/iproject.h>
#include <interfaces/iprojectcontroller.h>
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
: m_previousUppermostExecutableContext(0),
  m_maxLevel(0),
  m_locked(false),
  m_controlFlowMode(ControlFlowFunction),
  m_clusteringModes(ClusteringNone)
{
}

DUChainControlFlow::~DUChainControlFlow()
{
}

void DUChainControlFlow::cursorPositionChanged(KTextEditor::View *view, const KTextEditor::Cursor &cursor)
{
    if (m_locked) return;
    m_currentLevel = 1;

    if (!view->document()) return;

    DUChainReadLocker lock(DUChain::lock());
    
    TopDUContext *topContext = DUChainUtils::standardContextForUrl(view->document()->url());
    if (!topContext) return;

    // If cursor isn't inside a function definition
    DUContext *context = topContext->findContext(KDevelop::SimpleCursor(cursor));

    // If cursor is in a method arguments context change it to internal context
    if (context && context->type() == DUContext::Function && context->importers().size() == 1)
	context = context->importers()[0];

    Declaration *declarationUnderCursor = DUChainUtils::itemUnderCursor(view->document()->url(), KDevelop::SimpleCursor(cursor));
    if (declarationUnderCursor && (!context || context->type() != DUContext::Other) && declarationUnderCursor->internalContext())
	context = declarationUnderCursor->internalContext();

    if (!context || context->type() != DUContext::Other)
    {
	// If there is a previous graph
	if (m_previousUppermostExecutableContext != 0)
	{
	    newGraph();
	    m_previousUppermostExecutableContext = 0;
	}
	return;
    }

    // Navigate to uppermost executable context
    DUContext *uppermostExecutableContext = context;
    while (uppermostExecutableContext->parentContext()->type() == DUContext::Other)
        uppermostExecutableContext = uppermostExecutableContext->parentContext();

    // If cursor is in the same function definition
    if (uppermostExecutableContext == m_previousUppermostExecutableContext)
	return;

    m_previousUppermostExecutableContext = uppermostExecutableContext;

    // Get the definition
    Declaration* definition = 0;
    if (!uppermostExecutableContext || !uppermostExecutableContext->owner())
        return;
    else
        definition = uppermostExecutableContext->owner();

    if (!definition) return;

    newGraph();

    // Convert to a declaration in accordance with control flow mode (function, class or namespace)
    Declaration *nodeDefinition = declarationFromControlFlowMode(definition);

    QStringList containers;
    prepareContainers(containers, definition);

    emit foundRootNode(containers, (m_controlFlowMode == ControlFlowNamespace && nodeDefinition->internalContext()->type() != DUContext::Namespace) ? "Global Namespace":nodeDefinition->qualifiedIdentifier().toString());

    if (m_maxLevel != 1)
    {
        ++m_currentLevel;
	m_visitedFunctions.insert(definition);
        m_identifierDeclarationMap[nodeDefinition->qualifiedIdentifier().toString()] = nodeDefinition;
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
		    // Recursive call for sub-contexts
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
	    // Recursive call for remaining sub-contexts
            useDeclarationsFromDefinition(definition, topContext, *subContextsIterator);
	    subContextsIterator++;
	}
}

void DUChainControlFlow::processFunctionCall(Declaration *source, Declaration *target)
{
    FunctionDefinition *calledFunctionDefinition;
    DUContext *calledFunctionContext;

    DUChainReadLocker lock(DUChain::lock());

    // Convert to a declaration in accordance with control flow mode (function, class or namespace)
    Declaration *nodeSource = declarationFromControlFlowMode(source);
    Declaration *nodeTarget = declarationFromControlFlowMode(target);

    // Try to acquire the called function definition
    calledFunctionDefinition = FunctionDefinition::definition(target);

    QString sourceLabel = (m_controlFlowMode == ControlFlowNamespace && nodeSource->internalContext()->type() != DUContext::Namespace) ? "Global Namespace" : nodeSource->qualifiedIdentifier().toString();
    QString targetLabel = (m_controlFlowMode == ControlFlowNamespace && nodeTarget->internalContext()->type() != DUContext::Namespace) ? "Global Namespace" : nodeTarget->qualifiedIdentifier().toString();

    // If there is a flow (in accordance with control flow mode) emit signal
    if (targetLabel != sourceLabel ||
	m_controlFlowMode == ControlFlowFunction ||
	(calledFunctionDefinition && m_visitedFunctions.contains(calledFunctionDefinition)))
    {
	QStringList sourceContainers, targetContainers;
	prepareContainers(sourceContainers, source);
	prepareContainers(targetContainers, target);
	emit foundFunctionCall(sourceContainers, sourceLabel, targetContainers, targetLabel); 
    }

    if (calledFunctionDefinition)
	calledFunctionContext = calledFunctionDefinition->internalContext();
    else
    {
	// Store method declaration for navigation
	m_identifierDeclarationMap[nodeTarget->qualifiedIdentifier().toString()] = nodeTarget;
	return;
    }

    if (calledFunctionContext && (m_currentLevel < m_maxLevel || m_maxLevel == 0))
    {
	// For prevent endless loop in recursive methods
	if (!m_visitedFunctions.contains(calledFunctionDefinition))
	{
	    ++m_currentLevel;
	    m_visitedFunctions.insert(calledFunctionDefinition);
	    // Store method definition for navigation
	    m_identifierDeclarationMap[nodeTarget->qualifiedIdentifier().toString()] = declarationFromControlFlowMode(calledFunctionDefinition);
	    // Recursive call for method invocation
	    useDeclarationsFromDefinition(calledFunctionDefinition, calledFunctionDefinition->topContext(), calledFunctionContext);
	}
    }
}

void DUChainControlFlow::newGraph()
{
    m_visitedFunctions.clear();
    m_identifierDeclarationMap.clear();
    emit clearGraph();
}

void DUChainControlFlow::viewDestroyed(QObject * object)
{
    Q_UNUSED(object);
    if (!ICore::self()->documentController()->activeDocument())
        newGraph();
}

void DUChainControlFlow::focusIn(KTextEditor::View *view)
{
    cursorPositionChanged(view, view->cursorPosition());
}

void DUChainControlFlow::selectionIs(const QList<QString> list, const QPoint& point)
{
    if (!list.isEmpty())
    {
	Declaration *declaration = m_identifierDeclarationMap[list[0]];
	if (declaration)
	    ICore::self()->documentController()->openDocument(KUrl(declaration->url().str()),
							      declaration->range().textRange().start());
    }
}

Declaration *DUChainControlFlow::declarationFromControlFlowMode(Declaration *definitionDeclaration)
{
    Declaration *nodeDeclaration = definitionDeclaration;

    if (m_controlFlowMode != ControlFlowFunction && nodeDeclaration->identifier().toString() != "main")
    {
	if (nodeDeclaration->isDefinition())
	    nodeDeclaration = DUChainUtils::declarationForDefinition(nodeDeclaration, nodeDeclaration->topContext());
	if (!nodeDeclaration || !nodeDeclaration->context() || !nodeDeclaration->context()->owner()) return definitionDeclaration;
	while (nodeDeclaration->context() &&
	       nodeDeclaration->context()->owner() &&
	       ((m_controlFlowMode == ControlFlowClass && nodeDeclaration->context()->type() == DUContext::Class) ||
	        (m_controlFlowMode == ControlFlowNamespace && (
							      nodeDeclaration->context()->type() == DUContext::Class ||
							      nodeDeclaration->context()->type() == DUContext::Namespace)
	      )))
	    nodeDeclaration = nodeDeclaration->context()->owner();
    }
    return nodeDeclaration;
}

void DUChainControlFlow::setControlFlowMode(ControlFlowMode controlFlowMode)
{
    m_controlFlowMode = controlFlowMode;
    if(ICore::self()->documentController()->activeDocument() &&
       ICore::self()->documentController()->activeDocument()->textDocument() &&
       ICore::self()->documentController()->activeDocument()->textDocument()->activeView())
    {
	m_previousUppermostExecutableContext = 0;
	focusIn(ICore::self()->documentController()->activeDocument()->textDocument()->activeView());
    }
}

void DUChainControlFlow::setLocked(bool locked)
{
    m_locked = locked;
}

void DUChainControlFlow::setClusteringModes(ClusteringModes clusteringModes)
{
    m_clusteringModes = clusteringModes;
    if(ICore::self()->documentController() &&
       ICore::self()->documentController()->activeDocument() &&
       ICore::self()->documentController()->activeDocument()->textDocument() &&
       ICore::self()->documentController()->activeDocument()->textDocument()->activeView())
    {
	m_previousUppermostExecutableContext = 0;
	focusIn(ICore::self()->documentController()->activeDocument()->textDocument()->activeView());
    }
}

DUChainControlFlow::ClusteringModes DUChainControlFlow::clusteringModes()
{
    return m_clusteringModes;
}

void DUChainControlFlow::prepareContainers(QStringList &containers, Declaration* definition)
{
    ControlFlowMode originalControlFlowMode = m_controlFlowMode;

    // Handling project clustering
    if (m_clusteringModes.testFlag(ClusteringProject) && ICore::self()->projectController()->findProjectForUrl(definition->url().str()))
	containers << ICore::self()->projectController()->findProjectForUrl(definition->url().str())->name();

    // Handling namespace clustering
    if (m_clusteringModes.testFlag(ClusteringNamespace))
    {
	m_controlFlowMode = ControlFlowNamespace;
	Declaration *namespaceDefinition = declarationFromControlFlowMode(definition);
	containers << ((namespaceDefinition->internalContext()->type() != DUContext::Namespace) ? "Global Namespace":namespaceDefinition->qualifiedIdentifier().toString());
    }

    // Handling class clustering
    if (m_clusteringModes.testFlag(ClusteringClass))
    {
	m_controlFlowMode = ControlFlowClass;
	Declaration *classDefinition = declarationFromControlFlowMode(definition);
	if (classDefinition->internalContext() && classDefinition->internalContext()->type() == DUContext::Class)
	    containers << classDefinition->qualifiedIdentifier().toString();
    }

    m_controlFlowMode = originalControlFlowMode;
}
