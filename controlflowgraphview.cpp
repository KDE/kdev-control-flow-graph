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

#include "controlflowgraphview.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QFontMetricsF>

#include <KLibLoader>
#include <KMessageBox>
#include <KParts/Part>
#include <KActionCollection>

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iprojectcontroller.h>

#include "duchaincontrolflow.h"
#include "dotcontrolflowgraph.h"
#include "controlflowgraphfiledialog.h"
#include "kdevcontrolflowgraphviewplugin.h"

using namespace KDevelop;

ControlFlowGraphView::ControlFlowGraphView(KDevControlFlowGraphViewPlugin *plugin, QWidget *parent)
:
QWidget(parent),
m_plugin(plugin),
m_part(0),
m_dotControlFlowGraph(new DotControlFlowGraph),
m_duchainControlFlow(new DUChainControlFlow(m_dotControlFlowGraph)),
m_graphLocked(false)
{
    setupUi(this);
    KLibFactory *factory = KLibLoader::self()->factory("kgraphviewerpart");
    if (factory)
    {
        m_part = factory->create<KParts::ReadOnlyPart>("kgraphviewerpart", this);
        if (m_part)
        {
            QMetaObject::invokeMethod(m_part, "setReadWrite");

            verticalLayout->addWidget(m_part->widget());

            modeFunctionToolButton->setIcon(KIcon("code-function"));
            modeClassToolButton->setIcon(KIcon("code-class"));
            modeNamespaceToolButton->setIcon(KIcon("namespace"));
            clusteringClassToolButton->setIcon(KIcon("code-class"));
            clusteringNamespaceToolButton->setIcon(KIcon("namespace"));
            clusteringProjectToolButton->setIcon(KIcon("folder-development"));
            useFolderNameToolButton->setIcon(KIcon("folder-favorites"));
            drawIncomingArcsToolButton->setIcon(KIcon("draw-arrow-down"));
            maxLevelToolButton->setIcon(KIcon("zoom-fit-height"));
            exportToolButton->setIcon(KIcon("document-export"));
            m_duchainControlFlow->setMaxLevel(2);

            birdseyeToolButton->setIcon(KIcon("edit-find"));
            usesHoverToolButton->setIcon(KIcon("input-mouse"));
            zoominToolButton->setIcon(KIcon("zoom-in"));
            zoomoutToolButton->setIcon(KIcon("zoom-out"));

            if (ICore::self()->projectController()->projectCount() > 0)
                setProjectButtonsEnabled(true);

            useShortNamesToolButton->setIcon(KIcon("application-x-arc"));
            updateLockIcon(lockControlFlowGraphToolButton->isChecked());

            // Control flow mode buttons signals
            connect(modeFunctionToolButton, SIGNAL(toggled(bool)), SLOT(setControlFlowFunction(bool)));
            connect(modeClassToolButton, SIGNAL(toggled(bool)), SLOT(setControlFlowClass(bool)));
            connect(modeNamespaceToolButton, SIGNAL(toggled(bool)), SLOT(setControlFlowNamespace(bool)));

            // Clustering buttons signals
            connect(clusteringClassToolButton, SIGNAL(toggled(bool)), SLOT(setClusteringClass(bool)));
            connect(clusteringNamespaceToolButton, SIGNAL(toggled(bool)), SLOT(setClusteringNamespace(bool)));
            connect(clusteringProjectToolButton, SIGNAL(toggled(bool)), SLOT(setClusteringProject(bool)));

            // Configuration buttons signals
            connect(maxLevelSpinBox, SIGNAL(valueChanged(int)), SLOT(setMaxLevel(int)));
            connect(maxLevelToolButton, SIGNAL(toggled(bool)), SLOT(setUseMaxLevel(bool)));
            connect(drawIncomingArcsToolButton, SIGNAL(toggled(bool)), SLOT(setDrawIncomingArcs(bool)));
            connect(useFolderNameToolButton, SIGNAL(toggled(bool)), SLOT(setUseFolderName(bool)));
            connect(useShortNamesToolButton, SIGNAL(toggled(bool)), SLOT(setUseShortNames(bool)));
            connect(lockControlFlowGraphToolButton, SIGNAL(toggled(bool)), SLOT(updateLockIcon(bool)));

            // Left buttons signals
            connect(zoomoutToolButton, SIGNAL(clicked()), m_part->actionCollection()->action("view_zoom_out"), SIGNAL(triggered()));
            connect(zoominToolButton, SIGNAL(clicked()), m_part->actionCollection()->action("view_zoom_in"), SIGNAL(triggered()));
            m_part->actionCollection()->action("view_bev_enabled")->setIcon(KIcon("edit-find.png"));
            m_part->actionCollection()->action("view_bev_enabled")->setChecked(false);
            birdseyeToolButton->setDefaultAction(m_part->actionCollection()->action("view_bev_enabled"));
            connect(m_part, SIGNAL(selectionIs(QList<QString>, QPoint&)),
                    m_duchainControlFlow, SLOT(slotGraphElementSelected(QList<QString>,QPoint)));
            connect(m_part, SIGNAL(hoverEnter(QString)), m_duchainControlFlow, SLOT(slotEdgeHover(QString)));
            connect(exportToolButton, SIGNAL(clicked()), SLOT(exportControlFlowGraph()));
            connect(usesHoverToolButton, SIGNAL(toggled(bool)), m_duchainControlFlow, SLOT(setShowUsesOnEdgeHover(bool)));

            // Make sure we have a graph before we hook up signals to act on it
            m_dotControlFlowGraph->prepareNewGraph();

            // Graph generation signals
            connect(m_dotControlFlowGraph, SIGNAL(loadLibrary(graph_t*)), m_part, SLOT(slotLoadLibrary(graph_t*)));
            connect(m_duchainControlFlow, SIGNAL(startingJob()), SLOT(startingJob()));
            connect(m_duchainControlFlow, SIGNAL(jobDone()), SLOT(graphDone()));

            m_plugin->registerToolView(this);
        }
        else
            KMessageBox::error((QWidget *) m_plugin->core()->uiController()->activeMainWindow(), i18n("Could not load the KGraphViewer kpart"));
    }
    else
        KMessageBox::error((QWidget *) m_plugin->core()->uiController()->activeMainWindow(), i18n("Could not find the KGraphViewer factory") + ": " + KLibLoader::self()->lastErrorMessage());
}

ControlFlowGraphView::~ControlFlowGraphView()
{
    m_plugin->unRegisterToolView(this);
    delete m_duchainControlFlow;
    delete m_dotControlFlowGraph;
    delete m_part;
}

void ControlFlowGraphView::refreshGraph()
{
    m_duchainControlFlow->refreshGraph();
}

void ControlFlowGraphView::newGraph()
{
    m_duchainControlFlow->newGraph();
}

void ControlFlowGraphView::setProjectButtonsEnabled(bool enabled)
{
    useFolderNameToolButton->setEnabled(enabled);
    clusteringProjectToolButton->setEnabled(enabled);
}

void ControlFlowGraphView::cursorPositionChanged(KTextEditor::View *view, const KTextEditor::Cursor &cursor)
{
    m_duchainControlFlow->cursorPositionChanged(view, cursor);
}

void ControlFlowGraphView::startingJob()
{
    setEnabled(false);
}

void ControlFlowGraphView::graphDone()
{
    setEnabled(true);
}

void ControlFlowGraphView::exportControlFlowGraph()
{
    QPointer<ControlFlowGraphFileDialog> fileDialog;
    if ((fileDialog = m_plugin->exportControlFlowGraph(ControlFlowGraphFileDialog::NoConfigurationButtons)))
    {
        m_dotControlFlowGraph->exportGraph(fileDialog->selectedFile());
        KMessageBox::information(this, i18n("Control flow graph exported"), i18n("Export Control Flow Graph"));
    }
}

void ControlFlowGraphView::updateLockIcon(bool checked)
{
    lockControlFlowGraphToolButton->setIcon(KIcon(checked ? "document-encrypt":"document-decrypt"));
    lockControlFlowGraphToolButton->setToolTip(checked ? i18n("Unlock control flow graph"):i18n("Lock control flow graph"));
    m_duchainControlFlow->setLocked(checked);
    m_graphLocked = checked;
    if (!checked)
        m_duchainControlFlow->refreshGraph();
}

void ControlFlowGraphView::setControlFlowClass(bool checked)
{
    if (checked)
    {
        m_duchainControlFlow->setControlFlowMode(DUChainControlFlow::ControlFlowClass);
        m_duchainControlFlow->refreshGraph();
        clusteringClassToolButton->setChecked(false);
        clusteringClassToolButton->setEnabled(false);
        clusteringNamespaceToolButton->setEnabled(true);
    }
}

void ControlFlowGraphView::setControlFlowFunction(bool checked)
{
    if (checked)
    {
        m_duchainControlFlow->setControlFlowMode(DUChainControlFlow::ControlFlowFunction);
        m_duchainControlFlow->refreshGraph();
        clusteringClassToolButton->setEnabled(true);
        clusteringNamespaceToolButton->setEnabled(true);
    }
}

void ControlFlowGraphView::setControlFlowNamespace(bool checked)
{
    if (checked)
    {
        m_duchainControlFlow->setControlFlowMode(DUChainControlFlow::ControlFlowNamespace);
        m_duchainControlFlow->refreshGraph();
        clusteringClassToolButton->setChecked(false);
        clusteringClassToolButton->setEnabled(false);
        clusteringNamespaceToolButton->setChecked(false);
        clusteringNamespaceToolButton->setEnabled(false);
    }
}

void ControlFlowGraphView::setClusteringClass(bool checked)
{
    Q_UNUSED(checked);
    m_duchainControlFlow->setClusteringModes(m_duchainControlFlow->clusteringModes() ^ DUChainControlFlow::ClusteringClass);
    m_duchainControlFlow->refreshGraph();
    useShortNamesToolButton->setEnabled(m_duchainControlFlow->clusteringModes() ? true:false);
}

void ControlFlowGraphView::setClusteringProject(bool checked)
{
    Q_UNUSED(checked);
    m_duchainControlFlow->setClusteringModes(m_duchainControlFlow->clusteringModes() ^ DUChainControlFlow::ClusteringProject);
    m_duchainControlFlow->refreshGraph();
    useShortNamesToolButton->setEnabled(m_duchainControlFlow->clusteringModes() ? true:false);
}

void ControlFlowGraphView::setClusteringNamespace(bool checked)
{
    Q_UNUSED(checked);
    m_duchainControlFlow->setClusteringModes(m_duchainControlFlow->clusteringModes() ^ DUChainControlFlow::ClusteringNamespace);
    m_duchainControlFlow->refreshGraph();
    useShortNamesToolButton->setEnabled(m_duchainControlFlow->clusteringModes() ? true:false);
}

void ControlFlowGraphView::setUseMaxLevel(bool checked)
{
    maxLevelSpinBox->setEnabled(checked);
    setMaxLevel(checked ? maxLevelSpinBox->value():0);
}

void ControlFlowGraphView::setMaxLevel(int value)
{
    m_duchainControlFlow->setMaxLevel(value);
    m_duchainControlFlow->refreshGraph();
}

void ControlFlowGraphView::setDrawIncomingArcs(bool checked)
{
    m_duchainControlFlow->setDrawIncomingArcs(checked);
    m_duchainControlFlow->refreshGraph();
}

void ControlFlowGraphView::setUseFolderName(bool checked)
{
    m_duchainControlFlow->setUseFolderName(checked);
    m_duchainControlFlow->refreshGraph();
}

void ControlFlowGraphView::setUseShortNames(bool checked)
{
    m_duchainControlFlow->setUseShortNames(checked);
    m_duchainControlFlow->refreshGraph();
}

void ControlFlowGraphView::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    m_plugin->setActiveToolView(this);
}

void ControlFlowGraphView::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);
    m_plugin->setActiveToolView(0);
}
