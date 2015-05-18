/*
    Copyright (C) 2014  P.L. Lucas <selairi@gmail.com>
    Copyright (C) 2013  <copyright holder> <email>

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

#include "monitorsettingsdialog.h"
#include <QFrame>
#include <QLabel>
#include <QComboBox>
#include <QMessageBox>
#include <QTimer>
#include <QPushButton>
#include <QProgressBar>
#include <QInputDialog>
#include <QDebug>
#include <KScreen/Output>
#include <KScreen/SetConfigOperation>

#include "monitorwidget.h"
#include "timeoutdialog.h"
#include "monitorpicture.h"


MonitorSettingsDialog::MonitorSettingsDialog():
    QDialog(NULL, 0)
{
    timeoutDialog = NULL;
    timer = NULL;
    setConfig = NULL;
    setupUi();
}


MonitorSettingsDialog::~MonitorSettingsDialog() {
}


void MonitorSettingsDialog::configReceived(KScreen::ConfigOperation *op) {
    mConfig = qobject_cast<KScreen::GetConfigOperation*>(op)->config();

    KScreen::OutputList outputs = mConfig->outputs();
    Q_FOREACH(const KScreen::OutputPtr &output, outputs) {
        if (output->isConnected()) {
            MonitorWidget* monitor = new MonitorWidget(output, mConfig, this);
            ui.monitorList->addItem(output->name());
            ui.stackedWidget->addWidget(monitor);
        }
    }

    ui.monitorList->setMaximumWidth(ui.monitorList->sizeHintForColumn(0) + style()->pixelMetric(QStyle::PM_ScrollBarExtent) + 40);
    ui.monitorList->setCurrentRow(0);
    adjustSize();
    //TODO: ui.buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
}


void MonitorSettingsDialog::deleteTimeoutData() {
    // TODO Wipe old settings
}

/**
 * User wants to revert new settings
 */
void MonitorSettingsDialog::onCancelSettings() {

}

/**
 * Apply the settings
 */

void MonitorSettingsDialog::setMonitorsConfig() {
    qDebug() << "[MonitorSettingsDialog::setMonitorsConfig]: Changing config";
    if (!mConfig) { 
    	qDebug() << "[MonitorSettingsDialog::setMonitorsConfig]: No config";
	return;
    }
    if(KScreen::Config::canBeApplied(mConfig)) {
    	// Segment fault is emited if setConfig is deleted.
    	// if(setConfig != NULL)
	//	delete setConfig;
	if(KScreen::Config::canBeApplied(mConfig)) {
		// KDE KScreen builds a new object, too.
    		setConfig = new KScreen::SetConfigOperation(mConfig, this);
	}
    }
    //KScreen::SetConfigOperation(*mConfig);
}


void MonitorSettingsDialog::setupUi() {
    ui.setupUi(this);

    KScreen::GetConfigOperation *op = new KScreen::GetConfigOperation();
    connect(op, &KScreen::GetConfigOperation::finished, [&](KScreen::ConfigOperation *op) {
                configReceived(op);
    });

    connect(ui.buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(onDialogButtonClicked(QAbstractButton*)));
}

void MonitorSettingsDialog::accept() {
    setMonitorsConfig();
    QDialog::accept();
}

void MonitorSettingsDialog::onDialogButtonClicked(QAbstractButton* button) {
    if(ui.buttonBox->standardButton(button) == QDialogButtonBox::Apply) {
        setMonitorsConfig();
    }
}

#include <QDialogButtonBox>

void MonitorSettingsDialog::processClickedFromDialog(QDialogButtonBox::StandardButton button)
{
    qDebug() << "[MonitorSettingsDialog::processClickedFromDialog]";
    if(button == QDialogButtonBox::Apply)
	setMonitorsConfig();
}

QString MonitorSettingsDialog::getHardwareIdentifier()
{
  return hardwareIdentifier;
}
