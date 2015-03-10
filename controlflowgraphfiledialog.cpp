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

#include "controlflowgraphfiledialog.h"

#include <QRadioButton>

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>

#include "ui_controlflowgraphexportconfiguration.h"

using namespace KDevelop;

ControlFlowGraphFileDialog::ControlFlowGraphFileDialog(QWidget *parent, OpeningMode mode) :
    QFileDialog(parent, i18n("Export Control Flow Graph")),
    m_configurationWidget(0)
{
    QStringList mimeTypes;
    mimeTypes << "image/png"
              << "image/jpeg"
              << "image/gif"
              << "image/svg+xml"
              << "image/svg+xml-compressed"
              << "application/x-dia-diagram"
              << "image/x-xfig"
              << "application/pdf"
              << "text/vnd.graphviz";
    setMimeTypeFilters(mimeTypes);
    setAcceptMode(QFileDialog::AcceptSave);
    setConfirmOverwrite(true);
    setFileMode(QFileDialog::AnyFile);

    if (mode != NoConfigurationButtons)
    {
        m_configurationWidget = new Ui::ControlFlowGraphExportConfiguration;
        QWidget *widget = new QWidget;
        m_configurationWidget->setupUi(widget);

        m_configurationWidget->controlFlowFunctionRadioButton->setIcon(QIcon::fromTheme("flag-blue"));
        m_configurationWidget->controlFlowClassRadioButton->setIcon(QIcon::fromTheme("flag-green"));
        m_configurationWidget->controlFlowNamespaceRadioButton->setIcon(QIcon::fromTheme("flag-red"));
        m_configurationWidget->clusteringClassCheckBox->setIcon(QIcon::fromTheme("code-class"));
        m_configurationWidget->clusteringNamespaceCheckBox->setIcon(QIcon::fromTheme("namespace"));
        m_configurationWidget->clusteringProjectCheckBox->setIcon(QIcon::fromTheme("folder-development"));
        m_configurationWidget->limitMaxLevelCheckBox->setIcon(QIcon::fromTheme("zoom-fit-height"));
        m_configurationWidget->drawIncomingArcsCheckBox->setIcon(QIcon::fromTheme("draw-arrow-down"));
        m_configurationWidget->useFolderNameCheckBox->setIcon(QIcon::fromTheme("folder-favorites"));
        m_configurationWidget->useShortNamesCheckBox->setIcon(QIcon::fromTheme("application-x-arc"));

        connect(m_configurationWidget->controlFlowFunctionRadioButton, SIGNAL(toggled(bool)), SLOT(setControlFlowMode(bool)));
        connect(m_configurationWidget->controlFlowClassRadioButton, SIGNAL(toggled(bool)), SLOT(setControlFlowMode(bool)));
        connect(m_configurationWidget->controlFlowNamespaceRadioButton, SIGNAL(toggled(bool)), SLOT(setControlFlowMode(bool)));

        connect(m_configurationWidget->clusteringClassCheckBox, SIGNAL(stateChanged(int)), SLOT(setClusteringModes(int)));
        connect(m_configurationWidget->clusteringNamespaceCheckBox, SIGNAL(stateChanged(int)), SLOT(setClusteringModes(int)));
        connect(m_configurationWidget->clusteringProjectCheckBox, SIGNAL(stateChanged(int)), SLOT(setClusteringModes(int)));

        connect(m_configurationWidget->limitMaxLevelCheckBox, SIGNAL(stateChanged(int)), SLOT(slotLimitMaxLevelChanged(int)));

        if (ICore::self()->projectController()->projectCount() > 0)
        {
            m_configurationWidget->clusteringProjectCheckBox->setEnabled(true);
            m_configurationWidget->useFolderNameCheckBox->setEnabled(true);
        }

        layout()->addWidget(widget);
    }
}

ControlFlowGraphFileDialog::~ControlFlowGraphFileDialog()
{
    delete m_configurationWidget;
}

DUChainControlFlow::ControlFlowMode ControlFlowGraphFileDialog::controlFlowMode() const
{
    return (m_configurationWidget->controlFlowFunctionRadioButton->isChecked() ?
            DUChainControlFlow::ControlFlowFunction :
            ((m_configurationWidget->controlFlowClassRadioButton->isChecked()) ?
                DUChainControlFlow::ControlFlowClass : DUChainControlFlow::ControlFlowNamespace)
           );
}

DUChainControlFlow::ClusteringModes ControlFlowGraphFileDialog::clusteringModes() const
{
    DUChainControlFlow::ClusteringModes clusteringModes;

    if (m_configurationWidget->clusteringClassCheckBox->isChecked())
        clusteringModes |= DUChainControlFlow::ClusteringClass;
    if (m_configurationWidget->clusteringNamespaceCheckBox->isChecked())
        clusteringModes |= DUChainControlFlow::ClusteringNamespace;
    if (m_configurationWidget->clusteringProjectCheckBox->isChecked())
        clusteringModes |= DUChainControlFlow::ClusteringProject;

    return clusteringModes;
}

int ControlFlowGraphFileDialog::maxLevel() const
{
    if (m_configurationWidget->limitMaxLevelCheckBox->isChecked())
        return m_configurationWidget->maxLevelSpinBox->value();
    else
        return 0;
}

bool ControlFlowGraphFileDialog::useFolderName() const
{
    return m_configurationWidget->useFolderNameCheckBox->isChecked();
}

bool ControlFlowGraphFileDialog::useShortNames() const
{
    return m_configurationWidget->useShortNamesCheckBox->isChecked();
}

bool ControlFlowGraphFileDialog::drawIncomingArcs() const
{
    return m_configurationWidget->drawIncomingArcsCheckBox->isChecked();
}

void ControlFlowGraphFileDialog::setControlFlowMode(bool checked)
{
    if (checked)
    {
        QRadioButton *radioButton = qobject_cast<QRadioButton *>(sender());
        if (radioButton->objectName() == "controlFlowFunctionRadioButton")
        {
            m_configurationWidget->clusteringClassCheckBox->setEnabled(true);
            m_configurationWidget->clusteringNamespaceCheckBox->setEnabled(true);
        }
        if (radioButton->objectName() == "controlFlowClassRadioButton")
        {
            m_configurationWidget->clusteringClassCheckBox->setChecked(false);
            m_configurationWidget->clusteringClassCheckBox->setEnabled(false);
            m_configurationWidget->clusteringNamespaceCheckBox->setEnabled(true);
        }
        if (radioButton->objectName() == "controlFlowNamespaceRadioButton")
        {
            m_configurationWidget->clusteringClassCheckBox->setChecked(false);
            m_configurationWidget->clusteringClassCheckBox->setEnabled(false);
            m_configurationWidget->clusteringNamespaceCheckBox->setChecked(false);
            m_configurationWidget->clusteringNamespaceCheckBox->setEnabled(false);
        }
    }
}

void ControlFlowGraphFileDialog::setClusteringModes(int state)
{
    Q_UNUSED(state);
    m_configurationWidget->useShortNamesCheckBox->setEnabled(m_configurationWidget->clusteringClassCheckBox->isChecked() ||
                                                             m_configurationWidget->clusteringNamespaceCheckBox->isChecked() ||
                                                             m_configurationWidget->clusteringProjectCheckBox->isChecked());
}

void ControlFlowGraphFileDialog::slotLimitMaxLevelChanged(int state)
{
    m_configurationWidget->maxLevelSpinBox->setEnabled((state == Qt::Checked) ? true:false);
}
