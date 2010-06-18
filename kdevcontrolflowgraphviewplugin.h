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

#ifndef KDEVCONTROLFLOWGRAPHVIEWPLUGIN_H
#define KDEVCONTROLFLOWGRAPHVIEWPLUGIN_H

#include <QVariant>
#include <QList>

#include <interfaces/iplugin.h>
#include <interfaces/istatus.h>
#include <language/duchain/indexeditems.h>

#include "controlflowgraphfiledialog.h"

class KDevControlFlowGraphViewFactory;
class QAction;

namespace KDevelop
{
    class IProject;
    class IDocument;
    class ParseJob;
    class ContextMenuExtension;
}

namespace KTextEditor
{
    class View;
    class Document;
    class Cursor;
}

class ControlFlowGraphView;
class DUChainControlFlow;
class DotControlFlowGraph;
class ControlFlowGraphFileDialog;

using namespace KDevelop;

class KDevControlFlowGraphViewPlugin : public KDevelop::IPlugin, public KDevelop::IStatus
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IStatus)
public:
    explicit KDevControlFlowGraphViewPlugin(QObject *, const QVariantList & = QVariantList());
    virtual ~KDevControlFlowGraphViewPlugin();

    virtual QString statusName() const;
    virtual void unload();

    void registerToolView(ControlFlowGraphView *view);
    void unRegisterToolView(ControlFlowGraphView *view);
    QPointer<ControlFlowGraphFileDialog> exportControlFlowGraph(ControlFlowGraphFileDialog::OpeningMode mode = ControlFlowGraphFileDialog::ConfigurationButtons);

    KDevelop::ContextMenuExtension contextMenuExtension(KDevelop::Context* context);
    void generateControlFlowGraph();
    void generateClassControlFlowGraph();
    void generateProjectControlFlowGraph();
    void requestAbort();
public Q_SLOTS:
    void projectOpened(KDevelop::IProject* project);
    void projectClosed(KDevelop::IProject* project);
    void parseJobFinished(KDevelop::ParseJob* parseJob);
    void textDocumentCreated(KDevelop::IDocument *document);
    void viewCreated(KTextEditor::Document *document, KTextEditor::View *view);
    void viewDestroyed(QObject *object);
    void focusIn(KTextEditor::View *view);
    void cursorPositionChanged(KTextEditor::View *view, const KTextEditor::Cursor &cursor);

    void refreshActiveToolView();
    void slotExportControlFlowGraph(bool value);
    void slotExportClassControlFlowGraph(bool value);
    void slotExportProjectControlFlowGraph(bool value);
    void setActiveToolView(ControlFlowGraphView *activeToolView);
    void generationDone(KJob *job);
    void exportGraph();
Q_SIGNALS:
    // Implementations of IStatus signals
    void clearMessage(KDevelop::IStatus*);
    void showMessage(KDevelop::IStatus*, const QString &message, int timeout = 0);
    void hideProgress(KDevelop::IStatus*);
    void showProgress(KDevelop::IStatus*, int minimum, int maximum, int value);
    void showErrorMessage(const QString&, int);
private:
    void configureDuchainControlFlow(DUChainControlFlow *duchainControlFlow, DotControlFlowGraph *dotControlFlowGraph, ControlFlowGraphFileDialog *fileDialog);

    ControlFlowGraphView *activeToolView();
    KDevControlFlowGraphViewFactory *m_toolViewFactory;
    QList<ControlFlowGraphView *> m_toolViews;
    ControlFlowGraphView *m_activeToolView;
    QAction *m_exportControlFlowGraph;
    QAction *m_exportClassControlFlowGraph;
    QAction *m_exportProjectControlFlowGraph;
    
    IndexedDeclaration m_ideclaration;
    IProject *m_project;
    
    QPointer<DUChainControlFlow> m_duchainControlFlow;
    QPointer<DotControlFlowGraph> m_dotControlFlowGraph;

    ControlFlowGraphFileDialog *m_fileDialog;

    bool m_abort;
};

#endif
