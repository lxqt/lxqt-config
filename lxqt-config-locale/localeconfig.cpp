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
#include <QDateTime>
#include <QMessageBox>

#include <algorithm>

const static QString lcLang = QStringLiteral("LANG");

const static QString lcNumeric = QStringLiteral("LC_NUMERIC");
const static QString lcTime = QStringLiteral("LC_TIME");
const static QString lcMonetary = QStringLiteral("LC_MONETARY");
const static QString lcMeasurement = QStringLiteral("LC_MEASUREMENT");
const static QString lcCollate = QStringLiteral("LC_COLLATE");
const static QString lcCtype = QStringLiteral("LC_CTYPE");

const static QString lcLanguage = QStringLiteral("LANGUAGE");

LocaleConfig::LocaleConfig(LXQt::Settings* settings, LXQt::Settings* session_settings, QWidget* parent) :
    QWidget(parent),
    m_ui(new Ui::LocaleConfig),
    hasChanged(false),
    mSettings(settings),
    sSettings(session_settings)


{
    m_ui->setupUi(this);
    m_combos << m_ui->comboGlobal
             << m_ui->comboNumbers
             << m_ui->comboTime
             << m_ui->comboCurrency
             << m_ui->comboMeasurement
             << m_ui->comboCollate;

    initControls();
}


LocaleConfig::~LocaleConfig()
{
    delete m_ui;
}

bool countryLessThan(const QLocale & c1, const QLocale & c2)
{
    // get the strings as in addLocaleToCombo() -> clabel
    return QString::localeAwareCompare(!c1.nativeTerritoryName().isEmpty()
                                           ? c1.nativeTerritoryName()
                                           : c1.territoryToString(c1.territory()),
                                       !c2.nativeTerritoryName().isEmpty()
                                           ? c2.nativeTerritoryName()
                                           : c2.territoryToString(c2.territory())) < 0;
}

void LocaleConfig::load()
{
    QList<QLocale> allLocales = QLocale::matchingLocales(QLocale::AnyLanguage, QLocale::AnyScript, QLocale::AnyCountry);
    std::sort(allLocales.begin(), allLocales.end(), countryLessThan);
    for(QComboBox * combo : std::as_const(m_combos))
    {
        initCombo(combo, allLocales);
    }

    readConfig();

    for(QComboBox * combo : std::as_const(m_combos))
    {
        connectCombo(combo);
    }

    connect(m_ui->checkDetailed, &QGroupBox::toggled, [ = ]()
    {
        updateExample();
        hasChanged = true;
    });


    updateExample();
    hasChanged = false;
}

QString LocaleConfig::getCurrentforCombo(QComboBox *combo)
{
    QString res;
    const QString global = QString::fromLocal8Bit(qgetenv(lcLang.toLatin1().constData()));
    if (combo == m_ui->comboGlobal)
    {
        res = global;
    }
    else if (combo == m_ui->comboNumbers)
    {
        const QString numeric = QString::fromLocal8Bit(qgetenv(lcNumeric.toLatin1().constData()));
        res = numeric.isEmpty() ? global : numeric;
    }
    else if (combo == m_ui->comboTime)
    {
        const QString time = QString::fromLocal8Bit(qgetenv(lcTime.toLatin1().constData()));
        res = time.isEmpty() ? global : time;
    }
    else if (combo == m_ui->comboCurrency)
    {
        const QString monetary = QString::fromLocal8Bit(qgetenv(lcMonetary.toLatin1().constData()));
        res = monetary.isEmpty() ? global : monetary;
    }
    else if (combo == m_ui->comboMeasurement)
    {
        const QString measurement = QString::fromLocal8Bit(qgetenv(lcMeasurement.toLatin1().constData()));
        res = measurement.isEmpty() ? global : measurement;
    }
    else// if (combo == m_ui->comboCollate)
    {
        const QString collate = QString::fromLocal8Bit(qgetenv(lcCollate.toLatin1().constData()));
        res = collate.isEmpty() ? global : collate;
    }
    return res;
}

void LocaleConfig::initCombo(QComboBox *combo, const QList<QLocale> & allLocales)
{
    combo->clear();

    // for performance reason
    combo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);

    // set the first item to "Unset"
    combo->addItem(tr("Unset"), QString());

    // add the current locale
    addLocaleToCombo(combo, QLocale(getCurrentforCombo(combo)), true);

    // add all locales
    for(const QLocale & l : std::as_const(allLocales))
    {
        addLocaleToCombo(combo, l);
    }
}

void LocaleConfig::connectCombo(QComboBox *combo)
{
    connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), [ = ]()
    {
        hasChanged = true;
        updateExample();
    });
}

void LocaleConfig::addLocaleToCombo(QComboBox *combo, const QLocale &locale, bool currentLocale)
{
    const QString clabel = !locale.nativeTerritoryName().isEmpty() ? locale.nativeTerritoryName() : locale.territoryToString(locale.territory());
    // This needs to use name() rather than bcp47name() or later on the export will generate a non-sense locale (e.g. "it" instead of
    // "it_IT")
    // TODO: Properly handle scripts (@foo)
    QString cvalue = locale.name();
    if (!cvalue.contains(QLatin1Char('.')))
    { // explicitly add the encoding, otherwise Qt doesn't accept dead keys and garbles the output as well
        cvalue.append(QStringLiteral(".UTF-8"));
    }

    QString flagcode;
    const QStringList split = locale.name().split(QLatin1Char('_'));
    if (split.count() > 1)
    {
        flagcode = split[1].toLower();
    }
    /* TODO Find a better place for flags ... */
    QString flag(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("iso-flags-svg/country-4x3/%1.svg").arg(flagcode)));
    QIcon flagIcon;
    if (!flag.isEmpty())
    {
        if (!mFlags.contains(flag))
        {
            flagIcon = QIcon(flag);
            mFlags.insert(flag, flagIcon);
        }
        else
        {
            flagIcon = mFlags.value(flag);
        }
    }

    QString itemResult;
    itemResult = QStringLiteral("%1 - %2 (%3)")
                        .arg(clabel,
                        locale.nativeLanguageName(),
                        locale.name());
    if (currentLocale)
    {
        itemResult = tr("Current: ") + itemResult;
    }

    combo->addItem(flagIcon, itemResult, cvalue);

    if (currentLocale)
    {
        combo->insertSeparator(combo->count());
    }
}

void setCombo(QComboBox *combo, const QString &key)
{
    const int ix = combo->findData(key);
    if (ix > -1)
    {
        combo->setCurrentIndex(ix);
    }
    else if (combo->count() > 0)
    {
        combo->setCurrentIndex(0); // the "Unset" index
    }
}

void LocaleConfig::readConfig()
{
    mSettings->beginGroup(QStringLiteral("Formats"));

    bool useDetailed = mSettings->value(QStringLiteral("useDetailed"), false).toBool();
    m_ui->checkDetailed->setChecked(useDetailed);

    // set the combos by using the current EVs as fallbacks
    setCombo(m_ui->comboGlobal, mSettings->value(lcLang, getCurrentforCombo(m_ui->comboGlobal)).toString());
    setCombo(m_ui->comboNumbers, mSettings->value(lcNumeric, getCurrentforCombo(m_ui->comboNumbers)).toString());
    setCombo(m_ui->comboTime, mSettings->value(lcTime, getCurrentforCombo(m_ui->comboTime)).toString());
    setCombo(m_ui->comboCollate, mSettings->value(lcCollate, getCurrentforCombo(m_ui->comboCollate)).toString());
    setCombo(m_ui->comboCurrency, mSettings->value(lcMonetary, getCurrentforCombo(m_ui->comboCurrency)).toString());
    setCombo(m_ui->comboMeasurement, mSettings->value(lcMeasurement, getCurrentforCombo(m_ui->comboMeasurement)).toString());

    mSettings->endGroup();
}

void LocaleConfig::writeConfig()
{
    if (m_ui->comboGlobal->count() == 0)
    {
        qWarning() << "Couldn't read data from UI, writing configuration failed.";
        return;
    }

    mSettings->beginGroup(QStringLiteral("Formats"));
    const QString global = m_ui->comboGlobal->currentData().toString();

    if (!m_ui->checkDetailed->isChecked())
    {
        // Global setting, clean up config
        mSettings->remove(QStringLiteral("useDetailed"));
        if (global.isEmpty()) // not set
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
        mSettings->setValue(QStringLiteral("useDetailed"), true);

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
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("Format Settings Changed"));
        msgBox.setText(tr("Do you want to save your changes? They will take effect the next time you log in."));
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
    sSettings->beginGroup(QStringLiteral("Environment"));
    mSettings->beginGroup(QStringLiteral("Formats"));

    // add the LANG EV if set; otherwise, remove it
    if (mSettings->value(lcLang).toString().isNull()) // not set
    {
        sSettings->remove(lcLang);
    }
    else
    {
        sSettings->setValue(lcLang, mSettings->value(lcLang).toString());
    }

    if (mSettings->value(QStringLiteral("useDetailed")).toBool())
    { // add the existing detailed EVs and remove the others
        if (mSettings->value(lcNumeric).toString().isNull())
        {
            sSettings->remove(lcNumeric);
        }
        else
        {
            sSettings->setValue(lcNumeric, mSettings->value(lcNumeric).toString());
        }

        if (mSettings->value(lcTime).toString().isNull())
        {
            sSettings->remove(lcTime);
        }
        else
        {
            sSettings->setValue(lcTime, mSettings->value(lcTime).toString());
        }

        if (mSettings->value(lcCollate).toString().isNull())
        {
            sSettings->remove(lcCollate);
        }
        else
        {
            sSettings->setValue(lcCollate, mSettings->value(lcCollate).toString());
        }

        if (mSettings->value(lcMonetary).toString().isNull())
        {
            sSettings->remove(lcMonetary);
        }
        else
        {
            sSettings->setValue(lcMonetary, mSettings->value(lcMonetary).toString());
        }

        if (mSettings->value(lcMeasurement).toString().isNull())
        {
            sSettings->remove(lcMeasurement);
        }
        else
        {
            sSettings->setValue(lcMeasurement, mSettings->value(lcMeasurement).toString());
        }
    }
    else
    { // remove all detailed EVs
        sSettings->remove(lcNumeric);
        sSettings->remove(lcTime);
        sSettings->remove(lcCollate);
        sSettings->remove(lcMonetary);
        sSettings->remove(lcMeasurement);
    }

    mSettings->endGroup();
    sSettings->endGroup();
    sSettings->sync();
}

void LocaleConfig::updateExample()
{
    const bool useDetailed = m_ui->checkDetailed->isChecked();

    // if the main locale is not set and there is not detail, hide the Examples box
    bool noExample(!useDetailed && m_ui->comboGlobal->currentIndex() < 1);
    m_ui->exampleBox->setVisible(!noExample);
    if (noExample)
    {
        return;
    }

    QString nStr;
    QString tStr;
    QString cStr;
    QString mStr;

    if (useDetailed) // get the locales from the detailed combos
    {
        nStr = m_ui->comboNumbers->currentData().toString();
        tStr = m_ui->comboTime->currentData().toString();
        cStr = m_ui->comboCurrency->currentData().toString();
        mStr = m_ui->comboMeasurement->currentData().toString();
    }
    else // get the locales from the global one
    {
        if (m_ui->comboGlobal->currentIndex() == 1) // this is the current locale
        {
            nStr = getCurrentforCombo(m_ui->comboNumbers);
            tStr = getCurrentforCombo(m_ui->comboTime);
            cStr = getCurrentforCombo(m_ui->comboCurrency);
            mStr = getCurrentforCombo(m_ui->comboMeasurement);
        }
        else // another locale is selected from the list
        {
            nStr = m_ui->comboGlobal->currentData().toString();
            tStr = m_ui->comboGlobal->currentData().toString();
            cStr = m_ui->comboGlobal->currentData().toString();
            mStr = m_ui->comboGlobal->currentData().toString();
        }
    }

    const QString numberExample = nStr.isEmpty() ? QString()
        : QLocale(nStr).toString(1000.01);
    const QString timeExampleLong = tStr.isEmpty() ? QString()
        : QLocale(tStr).toString(QDateTime::currentDateTime(), QLocale::LongFormat);
    const QString timeExampleShort = tStr.isEmpty() ? QString()
        : QLocale(tStr).toString(QDateTime::currentDateTime(), QLocale::ShortFormat);
    const QString currencyExample = cStr.isEmpty() ? QString()
        : QLocale(cStr).toCurrencyString(24);

    QString measurementSetting;
    if (!mStr.isEmpty())
    {
        QLocale mLoc(mStr);
        if (mLoc.measurementSystem() == QLocale::ImperialUKSystem)
        {
            measurementSetting = tr("Imperial UK");
        }
        else if (mLoc.measurementSystem() == QLocale::ImperialUSSystem)
        {
            measurementSetting = tr("Imperial US");
        }
        else
        {
            measurementSetting = tr("Metric");
        }
    }

    m_ui->exampleNumbers->setText(numberExample);
    m_ui->exampleTimeLong->setText(timeExampleLong);
    m_ui->exampleTimeShort->setText(timeExampleShort);
    m_ui->exampleCurrency->setText(currencyExample);
    m_ui->exampleMeasurement->setText(measurementSetting);

    // this is especially needed on Wayland for preventing a truncated time label
    m_ui->exampleBox->adjustSize();
}

void LocaleConfig::initControls()
{
    load();
    hasChanged = false;
}
