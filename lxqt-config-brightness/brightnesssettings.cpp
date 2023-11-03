/*
    Copyright (C) 2016  P.L. Lucas <selairi@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "brightnesssettings.h"
#include "outputwidget.h"
#include <QMessageBox>
#include <QPushButton>

BrightnessSettings::BrightnessSettings(LXQt::Settings *settings, QWidget *parent):QDialog(parent)
{
    ui = new Ui::BrightnessSettings();
    ui->setupUi(this);

    mBrightness = new XRandrBrightness();
    mMonitors = mBrightness->getMonitorsInfo();
    mMonitorsInitial = mBrightness->getMonitorsInfo();
    mBacklight = new LXQt::Backlight(this);

    // Get minimum backlight
    mSettings = settings;
    mSettings->beginGroup(QStringLiteral("Backlight"));
    mMinBacklightValue = mSettings->value(QStringLiteral("min"), -1).toInt();
    if(mMinBacklightValue < 0) {
        // Set the minimum to 5% of the maximum to prevent a black screen
        mMinBacklightValue = qMax(qRound((qreal)(mBacklight->getMaxBacklight())*0.05), 1);
    }
    mInitialMinBacklightValue = mMinBacklightValue;

    ui->minBacklightSlider->setEnabled(mBacklight->isBacklightAvailable() || mBacklight->isBacklightOff());
    ui->backlightSlider->setEnabled(mBacklight->isBacklightAvailable() || mBacklight->isBacklightOff());
    ui->backlightGroupBox->setEnabled(mBacklight->isBacklightAvailable() || mBacklight->isBacklightOff());
    if(mBacklight->isBacklightAvailable()) {
        setBacklightSliderValue(mBacklight->getBacklight());

        mInitialBacklightValue = mLastBacklightValue = mBacklight->getBacklight();
        connect(ui->backlightSlider, &QSlider::valueChanged, this, &BrightnessSettings::setBacklight);

        connect(ui->backlightDownButton, &QToolButton::clicked,
            [this](bool){ ui->backlightSlider->setValue(ui->backlightSlider->value()-1); });
        connect(ui->backlightUpButton, &QToolButton::clicked,
            [this](bool){ ui->backlightSlider->setValue(ui->backlightSlider->value()+1); });

        ui->minBacklightSlider->setMinimum(0);
        ui->minBacklightSlider->setMaximum(mBacklight->getMaxBacklight());
        ui->minBacklightSlider->setValue(mMinBacklightValue);

        connect(ui->minBacklightSlider, &QSlider::valueChanged, this,
            [this](int){
                mMinBacklightValue = ui->minBacklightSlider->value();
                setBacklightSliderValue(mLastBacklightValue);
            });

        connect(ui->minBacklightDownButton, &QToolButton::clicked,
            [this](bool){
                ui->minBacklightSlider->setValue(ui->minBacklightSlider->value()-1);
                mMinBacklightValue = ui->minBacklightSlider->value();
                setBacklightSliderValue(mLastBacklightValue);
            });
        connect(ui->minBacklightUpButton, &QToolButton::clicked,
            [this](bool){
                ui->minBacklightSlider->setValue(ui->minBacklightSlider->value()+1);
                mMinBacklightValue = ui->minBacklightSlider->value();
                setBacklightSliderValue(mLastBacklightValue);
            });

        connect(ui->checkMinBacklightButton, &QPushButton::pressed,
            [this](){ mBacklight->setBacklight(ui->minBacklightSlider->value()); });
        connect(ui->checkMinBacklightButton, &QPushButton::released,
            [this](){ mBacklight->setBacklight(mLastBacklightValue); });
    }

    for(const MonitorInfo &monitor: qAsConst(mMonitors))
    {
        OutputWidget *output = new OutputWidget(monitor, this);
        ui->layout->addWidget(output);
        output->show();
        connect(output, &OutputWidget::changed, this, &BrightnessSettings::monitorSettingsChanged);
        connect(this, &BrightnessSettings::monitorReverted, output, &OutputWidget::setRevertedValues);
    }

    mConfirmRequestTimer.setSingleShot(true);
    mConfirmRequestTimer.setInterval(1000);
    connect(&mConfirmRequestTimer, &QTimer::timeout, this, &BrightnessSettings::requestConfirmation);

    connect(ui->buttonBox, &QDialogButtonBox::clicked,
        [this](QAbstractButton *button) {
            if(ui->buttonBox->button(QDialogButtonBox::Reset) == button) {
                revertValues();
            } else if(ui->buttonBox->button(QDialogButtonBox::Close) == button) {
                close();
            } else if(ui->buttonBox->button(QDialogButtonBox::Save) == button) {
                mSettings->setValue(QStringLiteral("min"), mMinBacklightValue);
                 QMessageBox msgBox;
                 msgBox.setText(tr("Backlight minimum value has been saved."));
                 msgBox.exec();
            }
        } );
}

BrightnessSettings::~BrightnessSettings()
{
    delete ui;
    ui = nullptr;

    delete mBrightness;
    mBrightness = nullptr;
}

void BrightnessSettings::setBacklight()
{
    int value = ui->backlightSlider->value();
    mBacklight->setBacklight(value);

    if (ui->confirmCB->isChecked())
        mConfirmRequestTimer.start();
}

void BrightnessSettings::monitorSettingsChanged(MonitorInfo monitor)
{
    mBrightness->setMonitorsSettings(QList<MonitorInfo>{}  << monitor);
    if (ui->confirmCB->isChecked())
        mConfirmRequestTimer.start();
    else {
        for (auto & m : mMonitors) {
            if (m.id() == monitor.id() && m.name() == monitor.name()) {
                m.setBacklight(monitor.backlight());
                m.setBrightness(monitor.brightness());
            }
        }
    }
}

void BrightnessSettings::requestConfirmation()
{
    QMessageBox msg{QMessageBox::Question, tr("Brightness settings changed")
        , tr("Confirmation required. Are the settings correct?")
        , QMessageBox::Yes | QMessageBox::No};
    int timeout = 5; // seconds
    QString no_text = msg.button(QMessageBox::No)->text();
    no_text += QStringLiteral("(%1)");
    msg.setButtonText(QMessageBox::No, no_text.arg(timeout));
    msg.setDefaultButton(QMessageBox::No);

    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(false);
    timeoutTimer.setInterval(1000);
    connect(&timeoutTimer, &QTimer::timeout, [&] {
        msg.setButtonText(QMessageBox::No, no_text.arg(--timeout));
        if (timeout == 0)
        {
            timeoutTimer.stop();
            msg.reject();
        }
    });
    timeoutTimer.start();

    if (QMessageBox::Yes == msg.exec())
    {
        // re-read current values
        if(mBacklight->isBacklightAvailable())
            mLastBacklightValue = mBacklight->getBacklight();

        mMonitors = mBrightness->getMonitorsInfo();
    } else
    {
        // revert the changes
        if(mBacklight->isBacklightAvailable()) {
            disconnect(ui->backlightSlider, &QSlider::valueChanged, this, &BrightnessSettings::setBacklight);
                mBacklight->setBacklight(mLastBacklightValue);
                setBacklightSliderValue(mLastBacklightValue);
            connect(ui->backlightSlider, &QSlider::valueChanged, this, &BrightnessSettings::setBacklight);
        }

        mBrightness->setMonitorsSettings(mMonitors);
        for (const auto & monitor : qAsConst(mMonitors))
            emit monitorReverted(monitor);
    }
}

void BrightnessSettings::revertValues()
{
    if(mBacklight->isBacklightAvailable()) {
        disconnect(ui->backlightSlider, &QSlider::valueChanged, this, &BrightnessSettings::setBacklight);
            mBacklight->setBacklight(mInitialBacklightValue);
            setBacklightSliderValue(mInitialBacklightValue);
            ui->minBacklightSlider->setValue(mInitialMinBacklightValue);
        connect(ui->backlightSlider, &QSlider::valueChanged, this, &BrightnessSettings::setBacklight);
    }

    mBrightness->setMonitorsSettings(mMonitorsInitial);
    for (const auto & monitor : qAsConst(mMonitorsInitial))
            emit monitorReverted(monitor);
}


void BrightnessSettings::setBacklightSliderValue(int value)
{
    int minBacklight = mMinBacklightValue;
    int maxBacklight = mBacklight->getMaxBacklight();

    ui->backlightSlider->setMaximum(maxBacklight);
    ui->backlightSlider->setMinimum(minBacklight);
    ui->backlightSlider->setValue(value);
}