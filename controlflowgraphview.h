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

#ifndef _CONTROLFLOWGRAPHVIEW_H_
#define _CONTROLFLOWGRAPHVIEW_H_

#include "ui_controlflowgraphview.h"

namespace KParts
{
    class ReadOnlyPart;
};

class ControlFlowGraphView : public QWidget, public Ui::ControlFlowGraphView
{
    Q_OBJECT
public:
    ControlFlowGraphView (QWidget *parent = 0);
    ~ControlFlowGraphView ();

    void drawGraph();
private:
    KParts::ReadOnlyPart* m_part;
};

#endif
