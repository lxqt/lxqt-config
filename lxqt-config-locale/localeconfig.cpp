/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)GPL2+
 *
 *
 * Copyright: 2014 LXQt team
 *            2014 Sebastian Kügler <sebas@kde.org>
 * Authors:
 *   Julien Lavergne <gilir@ubuntu.com>
 *   Sebastian Kügler <sebas@kde.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *
 * END_COMMON_COPYRIGHT_HEADER
 *
 * Based on plasma-desktop/kcms/formats module
 */

#include "localeconfig.h"
#include "ui_localeconfig.h"

#include <QApplication>
#include <QComboBox>
#include <QFile>
#include <QDebug>
#include <QLocale>
#include <QStandardPaths>
#include <QTextStream>
#include <QTextCodec>
#include <QDateTime>
#include <QMessageBox>

const static QString lcLang = QStringLiteral("LANG");

const static QString lcNumeric = QStringLiteral("LC_NUMERIC");
const static QString lcTime = QStringLiteral("LC_TIME");
const static QString lcMonetary = QStringLiteral("LC_MONETARY");
const static QString lcMeasurement = QStringLiteral("LC_MEASUREMENT");
const static QString lcCollate = QStringLiteral("LC_COLLATE");
const static QString lcCtype = QStringLiteral("LC_CTYPE");

const static QString lcLanguage = QStringLiteral("LANGUAGE");

LocaleConfig::LocaleConfig(LxQt::Settings* settings, LxQt::Settings* session_settings, QWidget* parent) :
    QWidget(parent),
    mSettings(settings),
    sSettings(session_settings),
    hasChanged(new bool),
    m_ui(new Ui::LocaleConfig)
    
{
    m_ui->setupUi(this);
    m_combos << m_ui->comboGlobal
             << m_ui->comboNumbers
             << m_ui->comboTime
             << m_ui->comboCurrency
             << m_ui->comboMeasurement
             << m_ui->comboCollate;

    hasChanged = false;

    initControls();
}


LocaleConfig::~LocaleConfig()
{
    delete m_ui;
}

bool countryLessThan(const QLocale & c1, const QLocale & c2)
{
    return QString::localeAwareCompare(c1.nativeCountryName(), c2.nativeCountryName()) < 0;
}

void LocaleConfig::load()
{
    QList<QLocale> allLocales = QLocale::matchingLocales(QLocale::AnyLanguage, QLocale::AnyScript, QLocale::AnyCountry);
    qSort(allLocales.begin(), allLocales.end(), countryLessThan);
    foreach(QComboBox * combo, m_combos)
    {
        initCombo(combo, allLocales);
    }

    readConfig();

    foreach(QComboBox * combo, m_combos)
    {
        connectCombo(combo);
    }

    connect(m_ui->checkDetailed, &QAbstractButton::toggled, [ = ]()
    {
        updateExample();
        updateEnabled();
        hasChanged = true;
    });

    updateEnabled();
    updateExample();
    hasChanged = false;
}

void LocaleConfig::initCombo(QComboBox *combo, const QList<QLocale> & allLocales)
{
    combo->clear();
    const QString clabel = tr("No change");
    combo->addItem(clabel, QString());
    foreach(const QLocale & l, allLocales)
    {
        addLocaleToCombo(combo, l);
    }
}

void LocaleConfig::connectCombo(QComboBox *combo)
{
    connect(combo, &QComboBox::currentTextChanged, [ = ]()
    {
        hasChanged = true;
        updateExample();
    });
}

void LocaleConfig::addLocaleToCombo(QComboBox *combo, const QLocale &locale)
{
    const QString clabel = !locale.nativeCountryName().isEmpty() ? locale.nativeCountryName() : locale.countryToString(locale.country());
    // This needs to use name() rather than bcp47name() or later on the export will generate a non-sense locale (e.g. "it" instead of
    // "it_IT")
    // TODO: Properly handle scripts (@foo)
    QString cvalue = locale.name();
    if (!cvalue.contains('.'))
    { // explicitely add the encoding, otherwise Qt doesn't accept dead keys and garbles the output as well
        cvalue.append(QLatin1Char('.') + QTextCodec::codecForLocale()->name());
    }

    QString flagcode;
    const QStringList split = locale.name().split('_');
    if (split.count() > 1)
    {
        flagcode = split[1].toLower();
    }
    /* TODO Find a better place for flags ... */
    QString flag(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kf5/locale/countries/%1/flag.png").arg(flagcode)));
    QIcon flagIcon;
    if (!flag.isEmpty())
    {
        flagIcon = QIcon(flag);
    }

    QString itemResult;
    itemResult = QString("%1 - %2 (%3)")
                        .arg(clabel)
                        .arg(locale.nativeLanguageName())
                        .arg(locale.name());

    combo->addItem(flagIcon, itemResult, cvalue);
}

void setCombo(QComboBox *combo, const QString &key)
{
    const int ix = combo->findData(key);
    if (ix > -1)
    {
        combo->setCurrentIndex(ix);
    }
}

void LocaleConfig::readConfig()
{
    mSettings->beginGroup("Formats");

    bool useDetailed = mSettings->value("useDetailed", false).toBool();
    m_ui->checkDetailed->setChecked(useDetailed);

    setCombo(m_ui->comboGlobal, mSettings->value(lcLang, qgetenv(lcLang.toLatin1())).toString());

    setCombo(m_ui->comboNumbers, mSettings->value(lcNumeric, qgetenv(lcNumeric.toLatin1())).toString());
    setCombo(m_ui->comboTime, mSettings->value(lcTime, qgetenv(lcTime.toLatin1())).toString());
    setCombo(m_ui->comboCollate, mSettings->value(lcCollate, qgetenv(lcCollate.toLatin1())).toString());
    setCombo(m_ui->comboCurrency, mSettings->value(lcMonetary, qgetenv(lcMonetary.toLatin1())).toString());
    setCombo(m_ui->comboMeasurement, mSettings->value(lcMeasurement, qgetenv(lcMeasurement.toLatin1())).toString());

    updateEnabled();

    mSettings->endGroup();
}

void LocaleConfig::writeConfig()
{
    mSettings->beginGroup("Formats");

    // global ends up empty here when OK button is clicked from kcmshell5,
    // apparently the data in the combo is gone by the time save() is called.
    // This might be a problem in KCModule, but does not directly affect us
    // since within systemsettings, it works fine.
    // See https://bugs.kde.org/show_bug.cgi?id=334624
    if (m_ui->comboGlobal->count() == 0)
    {
        qWarning() << "Couldn't read data from UI, writing configuration failed.";
        return;
    }
    const QString global = m_ui->comboGlobal->currentData().toString();

    if (!m_ui->checkDetailed->isChecked())
    {
        // Global setting, clean up config
        mSettings->remove("useDetailed");
        if (global.isEmpty())
        {
            mSettings->remove(lcLang);
        }
        else
        {
            mSettings->setValue(lcLang, global);
        }
        mSettings->remove(lcNumeric);
        mSettings->remove(lcTime);
        mSettings->remove(lcMonetary);
        mSettings->remove(lcMeasurement);
        mSettings->remove(lcCollate);
        mSettings->remove(lcCtype);
    }
    else
    {
        // Save detailed settings
        mSettings->setValue("useDetailed", true);

        if (global.isEmpty())
        {
            mSettings->remove(lcLang);
        }
        else
        {
            mSettings->setValue(lcLang, global);
        }

        const QString numeric = m_ui->comboNumbers->currentData().toString();
        if (numeric.isEmpty())
        {
            mSettings->remove(lcNumeric);
        }
        else
        {
            mSettings->setValue(lcNumeric, numeric);
        }

        const QString time = m_ui->comboTime->currentData().toString();
        if (time.isEmpty())
        {
            mSettings->remove(lcTime);
        } 
        else
        {
            mSettings->setValue(lcTime, time);
        }

        const QString monetary = m_ui->comboCurrency->currentData().toString();
        if (monetary.isEmpty())
        {
            mSettings->remove(lcMonetary);
        }
        else
        {
            mSettings->setValue(lcMonetary, monetary);
        }

        const QString measurement = m_ui->comboMeasurement->currentData().toString();
        if (measurement.isEmpty())
        {
            mSettings->remove(lcMeasurement);
        }
        else
        {
            mSettings->setValue(lcMeasurement, measurement);
        }

        const QString collate = m_ui->comboCollate->currentData().toString();
        if (collate.isEmpty())
        {
            mSettings->remove(lcCollate);
        } 
        else
        {
            mSettings->setValue(lcCollate, collate);
        }
    }
    mSettings->endGroup();
}

void LocaleConfig::saveSettings()
{
    if (hasChanged)
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle(tr("Format Settings Changed"));
        msgBox.setText(tr("Save the settings ? (they will take effect the next time you log in)"));
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        
        int ret = msgBox.exec();
        if( ret == QMessageBox::Save )
        {
            writeConfig();
            writeExports();
        }
    }

}

void LocaleConfig::writeExports()
{
    sSettings->beginGroup("Environment");
    mSettings->beginGroup("Formats");
    if (!mSettings->value(lcLang).toString().isNull())
    {
        sSettings->setValue(lcLang, mSettings->value(lcLang).toString());

        if (mSettings->value("useDetailed").toBool())
        {
            if (!mSettings->value(lcNumeric).toString().isNull())
            {
                sSettings->setValue(lcNumeric, mSettings->value(lcNumeric).toString());
            }
            if (!mSettings->value(lcTime).toString().isNull())
            {
                sSettings->setValue(lcTime, mSettings->value(lcTime).toString());
            }
            if (!mSettings->value(lcCollate).toString().isNull())
            {
                sSettings->setValue(lcCollate, mSettings->value(lcCollate).toString());
            }
            if (!mSettings->value(lcMonetary).toString().isNull())
            {
                sSettings->setValue(lcMonetary, mSettings->value(lcMonetary).toString());
            }
            if (!mSettings->value(lcMeasurement).toString().isNull())
            {
                sSettings->setValue(lcMeasurement, mSettings->value(lcMeasurement).toString());
            }
        }
        else
        {
            sSettings->setValue(lcNumeric, mSettings->value(lcLang).toString());
            sSettings->setValue(lcTime, mSettings->value(lcLang).toString());
            sSettings->setValue(lcCollate, mSettings->value(lcLang).toString());
            sSettings->setValue(lcMonetary, mSettings->value(lcLang).toString());
            sSettings->setValue(lcMeasurement, mSettings->value(lcLang).toString());
        }
    }
    mSettings->endGroup();
    sSettings->endGroup();
    sSettings->sync();
}

void LocaleConfig::defaults()
{
    m_ui->checkDetailed->setChecked(false);

    // restore user defaults from env vars
    setCombo(m_ui->comboGlobal, qgetenv(lcLang.toLatin1()));
    setCombo(m_ui->comboNumbers, qgetenv(lcNumeric.toLatin1()));
    setCombo(m_ui->comboTime, qgetenv(lcTime.toLatin1()));
    setCombo(m_ui->comboCollate, qgetenv(lcCollate.toLatin1()));
    setCombo(m_ui->comboCurrency, qgetenv(lcMonetary.toLatin1()));
    setCombo(m_ui->comboMeasurement, qgetenv(lcMeasurement.toLatin1()));

    updateEnabled();
}

void LocaleConfig::updateEnabled()
{
    const bool enabled = m_ui->checkDetailed->isChecked();

    m_ui->labelNumbers->setEnabled(enabled);
    m_ui->labelTime->setEnabled(enabled);
    m_ui->labelCurrency->setEnabled(enabled);
    m_ui->labelMeasurement->setEnabled(enabled);
    m_ui->labelCollate->setEnabled(enabled);
    m_ui->comboNumbers->setEnabled(enabled);
    m_ui->comboTime->setEnabled(enabled);
    m_ui->comboCurrency->setEnabled(enabled);
    m_ui->comboMeasurement->setEnabled(enabled);
    m_ui->comboCollate->setEnabled(enabled);
}

void LocaleConfig::updateExample()
{
    const bool useDetailed = m_ui->checkDetailed->isChecked();

    QLocale nloc;
    QLocale tloc;
    QLocale cloc;
    QLocale mloc;

    if (useDetailed)
    {
        nloc = QLocale(m_ui->comboNumbers->currentData().toString());
        tloc = QLocale(m_ui->comboTime->currentData().toString());
        cloc = QLocale(m_ui->comboCurrency->currentData().toString());
        mloc = QLocale(m_ui->comboMeasurement->currentData().toString());
    } 
    else
    {
        nloc = QLocale(m_ui->comboGlobal->currentData().toString());
        tloc = QLocale(m_ui->comboGlobal->currentData().toString());
        cloc = QLocale(m_ui->comboGlobal->currentData().toString());
        mloc = QLocale(m_ui->comboGlobal->currentData().toString());
    }

    const QString numberExample = nloc.toString(1000.01);
    const QString timeExample = tloc.toString(QDateTime::currentDateTime());
    const QString currencyExample = cloc.toCurrencyString(24);

    QString measurementSetting;
    if (mloc.measurementSystem() == QLocale::ImperialUKSystem)
    {
        measurementSetting = tr("Imperial UK");
    }
    else if (mloc.measurementSystem() == QLocale::ImperialUSSystem)
    {
        measurementSetting = tr("Imperial US");
    }
    else
    {
        measurementSetting = tr("Metric");
    }

    m_ui->exampleNumbers->setText(numberExample);
    m_ui->exampleTime->setText(timeExample);
    m_ui->exampleCurrency->setText(currencyExample);
    m_ui->exampleMeasurement->setText(measurementSetting);
}

void LocaleConfig::initControls()
{
    defaults();
    load();
    hasChanged = false;
}
