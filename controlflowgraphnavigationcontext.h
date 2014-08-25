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

#ifndef CONTROLFLOWGRAPHNAVIGATIONCONTEXT_H
#define CONTROLFLOWGRAPHNAVIGATIONCONTEXT_H

#include <QString>

#include <language/duchain/use.h>
#include <language/duchain/navigation/abstractnavigationcontext.h>

using namespace KDevelop;

class ControlFlowGraphNavigationContext : public AbstractNavigationContext
{
    Q_OBJECT
public:
    typedef QList< QPair<RangeInRevision, IndexedString> > ArcUses;

    ControlFlowGraphNavigationContext(const QString &label, const ArcUses &arcUses, TopDUContextPointer topContext, AbstractNavigationContext *previousContext = 0);
    virtual ~ControlFlowGraphNavigationContext();

    virtual QString name() const;
    virtual QString html(bool shorten = false);
public Q_SLOTS:
    void slotAnchorClicked(const QUrl &link);
private:
    const QString &m_label;
    QList< QPair<RangeInRevision, IndexedString> > m_arcUses;
};

#endif
