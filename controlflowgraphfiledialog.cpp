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
    m_configurationWidget(nullptr)
{
    const QStringList mimeTypes {
        QStringLiteral("image/png"),
        QStringLiteral("image/jpeg"),
        QStringLiteral("image/gif"),
        QStringLiteral("image/svg+xml"),
        QStringLiteral("image/svg+xml-compressed"),
        QStringLiteral("application/x-dia-diagram"),
        QStringLiteral("image/x-xfig"),
        QStringLiteral("application/pdf"),
        QStringLiteral("text/vnd.graphviz"),
    };
    setMimeTypeFilters(mimeTypes);
    setAcceptMode(QFileDialog::AcceptSave);
    setFileMode(QFileDialog::AnyFile);

    if (mode != NoConfigurationButtons)
    {
        m_configurationWidget = new Ui::ControlFlowGraphExportConfiguration;
        QWidget *widget = new QWidget;
        m_configurationWidget->setupUi(widget);

        m_configurationWidget->controlFlowFunctionRadioButton->setIcon(QIcon::fromTheme(QStringLiteral("flag-blue")));
        m_configurationWidget->controlFlowClassRadioButton->setIcon(QIcon::fromTheme(QStringLiteral("flag-green")));
        m_configurationWidget->controlFlowNamespaceRadioButton->setIcon(QIcon::fromTheme(QStringLiteral("flag-red")));
        m_configurationWidget->clusteringClassCheckBox->setIcon(QIcon::fromTheme(QStringLiteral("code-class")));
        m_configurationWidget->clusteringNamespaceCheckBox->setIcon(QIcon::fromTheme(QStringLiteral("namespace")));
        m_configurationWidget->clusteringProjectCheckBox->setIcon(QIcon::fromTheme(QStringLiteral("folder-development")));
        m_configurationWidget->limitMaxLevelCheckBox->setIcon(QIcon::fromTheme(QStringLiteral("zoom-fit-height")));
        m_configurationWidget->drawIncomingArcsCheckBox->setIcon(QIcon::fromTheme(QStringLiteral("draw-arrow-down")));
        m_configurationWidget->useFolderNameCheckBox->setIcon(QIcon::fromTheme(QStringLiteral("folder-favorites")));
        m_configurationWidget->useShortNamesCheckBox->setIcon(QIcon::fromTheme(QStringLiteral("application-x-arc")));

        connect(m_configurationWidget->controlFlowFunctionRadioButton, &QRadioButton::toggled,
                this, &ControlFlowGraphFileDialog::setControlFlowMode);
        connect(m_configurationWidget->controlFlowClassRadioButton, &QRadioButton::toggled,
                this, &ControlFlowGraphFileDialog::setControlFlowMode);
        connect(m_configurationWidget->controlFlowNamespaceRadioButton, &QRadioButton::toggled,
                this, &ControlFlowGraphFileDialog::setControlFlowMode);

        connect(m_configurationWidget->clusteringClassCheckBox, &QCheckBox::stateChanged,
                this, &ControlFlowGraphFileDialog::setClusteringModes);
        connect(m_configurationWidget->clusteringNamespaceCheckBox, &QCheckBox::stateChanged,
                this, &ControlFlowGraphFileDialog::setClusteringModes);
        connect(m_configurationWidget->clusteringProjectCheckBox, &QCheckBox::stateChanged,
                this, &ControlFlowGraphFileDialog::setClusteringModes);

        connect(m_configurationWidget->limitMaxLevelCheckBox, &QCheckBox::stateChanged,
                this, &ControlFlowGraphFileDialog::slotLimitMaxLevelChanged);

        if (ICore::self()->projectController()->projectCount() > 0)
        {
            m_configurationWidget->clusteringProjectCheckBox->setEnabled(true);
            m_configurationWidget->useFolderNameCheckBox->setEnabled(true);
        }
        // TODO: care about native non-Qt filedialog
        if (layout()) {
            layout()->addWidget(widget);
        } else {
            qWarning() << "No access to layout of QFileDialog, cannot add custom widget";
        }
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
        if (radioButton->objectName() == QLatin1String("controlFlowFunctionRadioButton")) {
            m_configurationWidget->clusteringClassCheckBox->setEnabled(true);
            m_configurationWidget->clusteringNamespaceCheckBox->setEnabled(true);
        }
        if (radioButton->objectName() == QLatin1String("controlFlowClassRadioButton")) {
            m_configurationWidget->clusteringClassCheckBox->setChecked(false);
            m_configurationWidget->clusteringClassCheckBox->setEnabled(false);
            m_configurationWidget->clusteringNamespaceCheckBox->setEnabled(true);
        }
        if (radioButton->objectName() == QLatin1String("controlFlowNamespaceRadioButton")) {
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
