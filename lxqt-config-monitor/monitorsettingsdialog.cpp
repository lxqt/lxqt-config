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

#include "monitorwidget.h"
#include "timeoutdialog.h"

#include <KScreen/Output>

MonitorSettingsDialog::MonitorSettingsDialog() :
    QDialog(nullptr, 0)
{
    ui.setupUi(this);

    KScreen::GetConfigOperation *operation = new KScreen::GetConfigOperation();
    connect(operation, &KScreen::GetConfigOperation::finished, [this, operation] (KScreen::ConfigOperation *op) {
        KScreen::GetConfigOperation *configOp = qobject_cast<KScreen::GetConfigOperation *>(op);
        if (configOp)
        {
            mOldConfig = configOp->config()->clone();
            loadConfiguration(configOp->config());
            operation->deleteLater();
        }
    });

    connect(ui.buttonBox, &QDialogButtonBox::clicked, [&] (QAbstractButton *button) {
        if (ui.buttonBox->standardButton(button) == QDialogButtonBox::Apply)
            applyConfiguration();
    });
}

MonitorSettingsDialog::~MonitorSettingsDialog()
{
}

void MonitorSettingsDialog::loadConfiguration(KScreen::ConfigPtr config)
{
    if (mConfig == config)
        return;

    mConfig = config;

    KScreen::OutputList outputs = mConfig->outputs();
    for (const KScreen::OutputPtr &output : outputs)
    {
        if (output->isConnected())
        {
            MonitorWidget *monitor = new MonitorWidget(output, mConfig, this);
            ui.monitorList->addItem(output->name());
            ui.stackedWidget->addWidget(monitor);
        }
    }

    ui.monitorList->setCurrentRow(0);
    adjustSize();
}


static QSize screenSize(const KScreen::ConfigPtr &config) 
{
    QRect rect;
    Q_FOREACH(const KScreen::OutputPtr &output, config->outputs()) {
        if (!output->isConnected() || !output->isEnabled())
            continue;

        const KScreen::ModePtr currentMode = output->currentMode();
        const QRect outputGeom = output->geometry();
        rect = rect.united(outputGeom);
    }

    const QSize size = QSize(rect.width(), rect.height());
    return size;
}


static int screenArea(QSize screenSize)
{
    return screenSize.width()*screenSize.height();
}

/**
 * Apply the settings
 */
void MonitorSettingsDialog::applyConfiguration()
{
    if (mConfig && KScreen::Config::canBeApplied(mConfig))
    {
        // FIXME: This is a hack to set right framebuffer size 
        QSize currentScreenSize = screenSize(mOldConfig);
        QSize newScreenSize = screenSize(mConfig);
        if(screenArea(currentScreenSize) > screenArea(newScreenSize))
            KScreen::SetConfigOperation(mConfig).exec();
        else
        {
            // Clone config and change position of outputs in order to force set framebuffer size
            KScreen::ConfigPtr cloneConfig = mConfig->clone();
            for (const KScreen::OutputPtr &output : cloneConfig->outputs())
            {
                 if(output->isEnabled())
                 {
                     QPoint pos = output->pos();
                     output->setPos(QPoint(pos.x()+10, pos.y()+10));
                     KScreen::SetConfigOperation(cloneConfig).exec();
                     // Apply right settings
                     KScreen::SetConfigOperation(mConfig).exec();
                     break;
                 }
            }
        }

        TimeoutDialog mTimeoutDialog;
        if (mTimeoutDialog.exec() == QDialog::Rejected)
            KScreen::SetConfigOperation(mOldConfig).exec();
        else
            mOldConfig = mConfig->clone();
    }
}

void MonitorSettingsDialog::accept()
{
    applyConfiguration();
    QDialog::accept();
}

void MonitorSettingsDialog::reject()
{
    QDialog::reject();
}
