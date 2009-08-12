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

#ifndef _CONTROLFLOWGRAPHNAVIGATIONWIDGET_H_
#define _CONTROLFLOWGRAPHNAVIGATIONWIDGET_H_

#include <QList>
#include <QPair>
#include <QString>

#include <language/duchain/navigation/abstractnavigationwidget.h>
#include <language/duchain/use.h>

using namespace KDevelop;

class ControlFlowGraphNavigationWidget : public AbstractNavigationWidget
{
    Q_OBJECT
public:
    typedef QList< QPair<Use, IndexedString> > ArcUses;
    ControlFlowGraphNavigationWidget(const ArcUses &arcUses);
};

#endif
