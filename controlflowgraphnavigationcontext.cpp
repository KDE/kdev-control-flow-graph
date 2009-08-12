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

#include "controlflowgraphnavigationcontext.h"

#include <language/codegen/coderepresentation.h>

ControlFlowGraphNavigationContext::ControlFlowGraphNavigationContext(const QList< QPair<Use, IndexedString> > &arcUses, TopDUContextPointer topContext, AbstractNavigationContext *previousContext)
 : AbstractNavigationContext(topContext, previousContext), m_arcUses (arcUses)
{
}

QString ControlFlowGraphNavigationContext::name() const
{
    return "Name ???";
}

QString ControlFlowGraphNavigationContext::html(bool shorten)
{
    clear();
    m_shorten = shorten;
    modifyHtml() += "<html><body><p><small><small>";
    
    QPair<Use, IndexedString> pair;
    foreach (pair, m_arcUses)
    {
      	CodeRepresentation::Ptr code = createCodeRepresentation(pair.second);
	modifyHtml() += pair.second.toUrl().toLocalFile() + " (" + QString::number(pair.first.m_range.start.line) + "): " + code->line(pair.first.m_range.start.line).trimmed() + "<br>";
    }
    
    modifyHtml() += "</small></small></p></body></html>";

    return currentHtml();
}
