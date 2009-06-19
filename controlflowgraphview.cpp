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

#include "controlflowgraphview.h"

#include <QTemporaryFile>

#include <kparts/part.h>
#include <klibloader.h>
#include <kservice.h>
#include <kmessagebox.h>
#include <kactioncollection.h>

ControlFlowGraphView::ControlFlowGraphView(QWidget *parent) : QWidget(parent), m_part(0), m_tempFile(0)
{
    setupUi(this);
    KLibFactory *factory = KLibLoader::self()->factory("kgraphviewerpart");
    if (factory)
    {
        m_part = factory->create<KParts::ReadOnlyPart>(this);
  	if (m_part)
	{
	    horizontalLayout->addWidget(m_part->widget());
	    connect(zoomoutToolButton, SIGNAL(clicked()), m_part->actionCollection()->action("view_zoom_out"), SIGNAL(triggered()));
	    connect(zoominToolButton, SIGNAL(clicked()), m_part->actionCollection()->action("view_zoom_in"), SIGNAL(triggered()));
	    m_tempFile = new QTemporaryFile();
	    if (!m_tempFile->open())
	        KMessageBox::error(this, i18n("Could not create temporary file"));
	}
        else
	    KMessageBox::error(this, i18n("Could not load the KGraphViewer kpart"));
    }
    else
        KMessageBox::error(this, i18n("Could not find the KGraphViewer factory"));
}

ControlFlowGraphView::~ControlFlowGraphView()
{
    if (m_part != 0) delete m_part;
    if (m_tempFile != 0) delete m_tempFile;
}

void ControlFlowGraphView::drawGraph()
{
    m_part->openUrl(KUrl("file://" + m_tempFile->fileName()));
}

QTemporaryFile *ControlFlowGraphView::tempFile()
{
    return m_tempFile;
}
