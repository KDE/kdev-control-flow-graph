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

#include "controlflowgraphnavigationwidget.h"

#include <language/duchain/topducontext.h>

using namespace KDevelop;

ControlFlowGraphNavigationWidget::ControlFlowGraphNavigationWidget(const QString &label, const ControlFlowGraphNavigationContext::ArcUses &arcUses)
{
    initBrowser(400);
    setFocusPolicy(Qt::NoFocus);
    ControlFlowGraphNavigationContext *context = new ControlFlowGraphNavigationContext(label, arcUses, TopDUContextPointer(0));
    setContext(NavigationContextPointer(context));
    connect(m_browser, SIGNAL(anchorClicked(QUrl)), context, SLOT(slotAnchorClicked(QUrl)));
}

ControlFlowGraphNavigationWidget::~ControlFlowGraphNavigationWidget()
{
}
