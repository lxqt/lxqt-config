/*
    Copyright (C) 2014  P.L. Lucas <selairi@gmail.com>
    Copyright (C) 2014  Hong Jen Yee (PCMan) <pcman.tw@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "monitorwidget.h"
#include "monitor.h"
#include <QDebug>
#include <QDialogButtonBox>
#include <KScreen/EDID>


QString modeToString(KScreen::ModePtr mode) {
    // mode->name() can be anything, not just widthxheight. eg if added with cvt.
    return QString("%1x%2").arg(mode->size().width()).arg(mode->size().height());
}

MonitorWidget::MonitorWidget(KScreen::OutputPtr output, KScreen::ConfigPtr config, QWidget* parent):
    QGroupBox(parent)
{
    this->output = output;

    ui.setupUi(this);

    ui.xPosSpinBox->setValue(output->pos().x());
    ui.yPosSpinBox->setValue(output->pos().y());

    ui.enabledCheckbox->setChecked(output->isEnabled());
    ui.isPrimaryCheckbox->setChecked(output->isPrimary());

    // Add the preferred mode at the top of the list
    KScreen::ModePtr preferredMode = output->preferredMode();
    if (preferredMode) {
        ui.resolutionCombo->addItem(modeToString(preferredMode), preferredMode->id());
        // Make it bold, for good measure
        QFont font = ui.resolutionCombo->font();
        font.setBold(true);
        ui.resolutionCombo->setItemData(0, font, Qt::FontRole);
    }

    // Add each mode to the list
    Q_FOREACH(const KScreen::ModePtr &mode, output->modes()) {
        // TODO better check for duplicates
        if (mode == preferredMode) continue;
        ui.resolutionCombo->addItem(modeToString(mode), mode->id());
    }
    connect(ui.resolutionCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onResolutionChanged(int)));

    if (output->currentMode()) {
        // Set the current mode in dropdown
        int idx = ui.resolutionCombo->findData(output->currentMode()->id());
        ui.resolutionCombo->setCurrentIndex(idx);
    }
    updateRefreshRates();

    /* XXX setting gamma not possible yet
    KScreen::Edid* edid = output->edid();
    if (edid && edid->isValid()) {}*/

    if (config->connectedOutputs().count() == 1) {
        setOnlyMonitor(true);
    } else {
        Q_FOREACH(KScreen::OutputPtr clone, config->connectedOutputs()) {
            // We can't clone ourselves, or an output that already clones another
            if (clone == output || clone->clones().count()) continue;
            ui.clonesCombo->addItem(clone->name(), clone->id());
        }
    }
}

void MonitorWidget::onResolutionChanged(int index) {
    qDebug() << "Set id to" << ui.resolutionCombo->currentData();

    updateRefreshRates();
    // TODO enable Apply button
}


void MonitorWidget::updateRefreshRates() {
    ui.rateCombo->clear();
    KScreen::ModePtr selectedMode = output->currentMode(); // XXX That's wrong
    Q_FOREACH(const KScreen::ModePtr &mode, output->modes()) {
        if (mode->size() == selectedMode->size()) {
            ui.rateCombo->addItem(tr("%1 Hz").arg(mode->refreshRate()), mode->id());
        }
    }
}


void MonitorWidget::setOnlyMonitor(bool isOnlyMonitor) {
    qDebug() << "set only monitor" << isOnlyMonitor;
    ui.xPosSpinBox->setVisible(!isOnlyMonitor);
    ui.yPosSpinBox->setVisible(!isOnlyMonitor);
    ui.xPosLabel->setVisible(!isOnlyMonitor);
    ui.yPosLabel->setVisible(!isOnlyMonitor);
    ui.extendsRadio->setVisible(!isOnlyMonitor);
    ui.clonesRadio->setVisible(!isOnlyMonitor);
    ui.clonesCombo->setVisible(!isOnlyMonitor);
    ui.enabledCheckbox->setEnabled(!isOnlyMonitor);
    ui.enabledCheckbox->setEnabled(!isOnlyMonitor);
}
