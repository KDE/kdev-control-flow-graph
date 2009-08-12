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

#include <QTextDocument>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>

#include <language/codegen/coderepresentation.h>

using namespace KDevelop;

ControlFlowGraphNavigationContext::ControlFlowGraphNavigationContext(const QString &label, const ArcUses &arcUses, TopDUContextPointer topContext, AbstractNavigationContext *previousContext)
 : AbstractNavigationContext(topContext, previousContext), m_label(label), m_arcUses (arcUses)
{
}

QString ControlFlowGraphNavigationContext::name() const
{
    return "Control flow graph navigation widget";
}

QString ControlFlowGraphNavigationContext::html(bool shorten)
{
    clear();
    m_shorten = shorten;
    modifyHtml() += "<html><body><p><small><small>";
    QStringList nodes = m_label.split("->");
    modifyHtml() += importantHighlight("Uses of ") + nodes[1] + importantHighlight(" from ") + nodes[0] + "<hr>";
    unsigned int i = m_arcUses.size()-1;
    QPair<Use, IndexedString> pair;
    QListIterator< QPair<Use, IndexedString> > iterator(m_arcUses);
    iterator.toBack();
    while (iterator.hasPrevious())
    {
	pair = iterator.previous();
      	CodeRepresentation::Ptr code = createCodeRepresentation(pair.second);
	modifyHtml() += "<a href='" + QString::number(i--) + "'>" + pair.second.toUrl().fileName() + " (" + QString::number(pair.first.m_range.start.line) + ")</a>: " + Qt::escape(code->line(pair.first.m_range.start.line).trimmed()) + "<br>";
    }
    
    modifyHtml() += "</small></small></p></body></html>";

    return currentHtml();
}

void ControlFlowGraphNavigationContext::slotAnchorClicked(const QUrl &link)
{
    int position = link.toString().toInt();
    QPair<Use, IndexedString> pair = m_arcUses[position];
    ICore::self()->documentController()->openDocument(KUrl(pair.second.toUrl()), pair.first.m_range.start.textCursor());
}
