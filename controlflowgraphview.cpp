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

#include <KLibLoader>
#include <KMessageBox>
#include <KActionCollection>
#include <KParts/Part>

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iuicontroller.h>

#include "kdevcontrolflowgraphviewplugin.h"
#include "duchaincontrolflow.h"
#include "dotcontrolflowgraph.h"
#include "controlflowgraphfiledialog.h"

using namespace KDevelop;

ControlFlowGraphView::ControlFlowGraphView(KDevControlFlowGraphViewPlugin *plugin, QWidget *parent)
:
QWidget(parent),
m_plugin(plugin),
m_part(0),
m_duchainControlFlow(new DUChainControlFlow),
m_dotControlFlowGraph(new DotControlFlowGraph),
m_graphLocked(false)
{
    setupUi(this);
    KLibFactory *factory = KLibLoader::self()->factory("kgraphviewerpart");
    if (factory)
    {
        m_part = factory->create<KParts::ReadOnlyPart>(this);
        if (m_part)
        {
            connect(this, SIGNAL(setReadWrite()), m_part, SLOT(setReadWrite()));
            emit setReadWrite();

            verticalLayout->addWidget(m_part->widget());

            modeFunctionToolButton->setIcon(KIcon("flag-blue"));
            modeClassToolButton->setIcon(KIcon("flag-green"));
            modeNamespaceToolButton->setIcon(KIcon("flag-red"));
            clusteringClassToolButton->setIcon(KIcon("code-class"));
            clusteringNamespaceToolButton->setIcon(KIcon("namespace"));
            clusteringProjectToolButton->setIcon(KIcon("folder-development"));
            useFolderNameToolButton->setIcon(KIcon("folder-favorites"));
            drawIncomingArcsToolButton->setIcon(KIcon("draw-arrow-down"));
            maxLevelToolButton->setIcon(KIcon("zoom-fit-height"));
            exportToolButton->setIcon(KIcon("document-export"));
            m_duchainControlFlow->setMaxLevel(2);

            birdseyeToolButton->setIcon(KIcon("edit-find"));
            zoominToolButton->setIcon(KIcon("zoom-in"));
            zoomoutToolButton->setIcon(KIcon("zoom-out"));

            if (ICore::self()->projectController()->projectCount() > 0)
                setProjectButtonsEnabled(true);

            useShortNamesToolButton->setIcon(KIcon("application-x-arc"));
            updateLockIcon(lockControlFlowGraphToolButton->isChecked());

            // Control flow mode buttons signals
            connect(modeFunctionToolButton, SIGNAL(toggled(bool)), SLOT(setControlFlowMode(bool)));
            connect(modeClassToolButton, SIGNAL(toggled(bool)), SLOT(setControlFlowMode(bool)));
            connect(modeNamespaceToolButton, SIGNAL(toggled(bool)), SLOT(setControlFlowMode(bool)));

            // Clustering buttons signals
            connect(clusteringClassToolButton, SIGNAL(toggled(bool)), SLOT(setClusteringModes(bool)));
            connect(clusteringNamespaceToolButton, SIGNAL(toggled(bool)), SLOT(setClusteringModes(bool)));
            connect(clusteringProjectToolButton, SIGNAL(toggled(bool)), SLOT(setClusteringModes(bool)));

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
            connect(m_part, SIGNAL(selectionIs(const QList<QString>, const QPoint&)),
                    m_duchainControlFlow, SLOT(slotGraphElementSelected(const QList<QString>,QPoint)));
            connect(exportToolButton, SIGNAL(clicked()), SLOT(exportControlFlowGraph()));

            // Make sure we have a graph before we hook up signals to act on it
            m_dotControlFlowGraph->prepareNewGraph();

            // Graph generation signals
            connect(m_duchainControlFlow,  SIGNAL(prepareNewGraph()),
                    m_dotControlFlowGraph, SLOT  (prepareNewGraph()));
            connect(m_duchainControlFlow,  SIGNAL(foundRootNode(const QStringList &, const QString &)),
                    m_dotControlFlowGraph, SLOT  (foundRootNode(const QStringList &, const QString &)));
            connect(m_duchainControlFlow,  SIGNAL(foundFunctionCall(const QStringList &, const QString &, const QStringList &, const QString &)),
                    m_dotControlFlowGraph, SLOT  (foundFunctionCall(const QStringList &, const QString &, const QStringList &, const QString &)));
            connect(m_duchainControlFlow,  SIGNAL(clearGraph()), m_dotControlFlowGraph, SLOT(clearGraph()));
            connect(m_duchainControlFlow,  SIGNAL(graphDone()), m_dotControlFlowGraph, SLOT(graphDone()));
            connect(m_dotControlFlowGraph, SIGNAL(loadLibrary(graph_t *)), m_part, SLOT(slotLoadLibrary(graph_t *)));

            m_plugin->registerToolView(this);
        }
        else
            KMessageBox::error((QWidget *) m_plugin->core()->uiController()->activeMainWindow(), i18n("Could not load the KGraphViewer kpart"));
    }
    else
        KMessageBox::error((QWidget *) m_plugin->core()->uiController()->activeMainWindow(), i18n("Could not find the KGraphViewer factory"));
}

ControlFlowGraphView::~ControlFlowGraphView()
{
    m_plugin->unRegisterToolView(this);
    delete m_duchainControlFlow;
    delete m_dotControlFlowGraph;
    delete m_part;
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

void ControlFlowGraphView::refreshGraph()
{
    m_duchainControlFlow->refreshGraph();
}

void ControlFlowGraphView::newGraph()
{
    m_duchainControlFlow->newGraph();
}

void ControlFlowGraphView::exportControlFlowGraph()
{
    QPointer<ControlFlowGraphFileDialog> fileDialog;
    if ((fileDialog = m_plugin->exportControlFlowGraph(ControlFlowGraphFileDialog::NoConfigurationButtons)))
    {
        m_dotControlFlowGraph->exportGraph(fileDialog->selectedFile());
        KMessageBox::information(this, i18n("Control flow graph exported."), i18n("Export Control Flow Graph"));
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

void ControlFlowGraphView::setControlFlowMode(bool checked)
{
    if (checked)
    {
        QToolButton *toolButton = qobject_cast<QToolButton *>(sender());
        if (toolButton->objectName() == "modeFunctionToolButton")
        {
            m_duchainControlFlow->setControlFlowMode(DUChainControlFlow::ControlFlowFunction);
            m_duchainControlFlow->refreshGraph();
            clusteringClassToolButton->setEnabled(true);
            clusteringNamespaceToolButton->setEnabled(true);
        }
        if (toolButton->objectName() == "modeClassToolButton")
        {
            m_duchainControlFlow->setControlFlowMode(DUChainControlFlow::ControlFlowClass);
            m_duchainControlFlow->refreshGraph();
            clusteringClassToolButton->setChecked(false);
            clusteringClassToolButton->setEnabled(false);
            clusteringNamespaceToolButton->setEnabled(true);
        }
        if (toolButton->objectName() == "modeNamespaceToolButton")
        {
            m_duchainControlFlow->setControlFlowMode(DUChainControlFlow::ControlFlowNamespace);
            m_duchainControlFlow->refreshGraph();
            clusteringClassToolButton->setChecked(false);
            clusteringClassToolButton->setEnabled(false);
            clusteringNamespaceToolButton->setChecked(false);
            clusteringNamespaceToolButton->setEnabled(false);
        }
    }
}

void ControlFlowGraphView::setClusteringModes(bool checked)
{
    Q_UNUSED(checked);
    QToolButton *toolButton = qobject_cast<QToolButton *>(sender());
    if (toolButton->objectName() == "clusteringClassToolButton")
        m_duchainControlFlow->setClusteringModes(m_duchainControlFlow->clusteringModes() ^ DUChainControlFlow::ClusteringClass);
    if (toolButton->objectName() == "clusteringNamespaceToolButton")
        m_duchainControlFlow->setClusteringModes(m_duchainControlFlow->clusteringModes() ^ DUChainControlFlow::ClusteringNamespace);
    if (toolButton->objectName() == "clusteringProjectToolButton")
        m_duchainControlFlow->setClusteringModes(m_duchainControlFlow->clusteringModes() ^ DUChainControlFlow::ClusteringProject);

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
