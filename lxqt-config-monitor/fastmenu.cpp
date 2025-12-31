/*
    Copyright (C) 2016  P.L. Lucas <selairi@gmail.com>

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

#include "fastmenu.h"
#include "timeoutdialog.h"
#include "kscreenutils.h"

#include <QComboBox>
#include <QPoint>

#include <KScreen/Output>
#include <KScreen/Mode>
#include <KScreen/SetConfigOperation>

#include <algorithm>

enum Options
{
    None=0, Extended=1, Unified=2, OnlyFirst=3, OnlySecond=4
};

FastMenu::FastMenu(KScreen::ConfigPtr config, QWidget* parent) :
    QGroupBox(parent)
{
    this->mConfig = config;
    this->mOldConfig = mConfig->clone();

    ui.setupUi(this);

    connect(ui.comboBox, &QComboBox::currentIndexChanged, this, &FastMenu::onSelectionChanged);
}

FastMenu::~FastMenu()
{
}

static bool sizeBiggerThan(const QSize &sizeA, const QSize &sizeB)
{
    return sizeA.width() * sizeA.height() > sizeB.width() * sizeB.height();
}

void FastMenu::unified()
{
    const KScreen::OutputList outputs = mConfig->outputs();
    // Find the common sizes
    QList<QSize> commonSizes;
    for (const KScreen::OutputPtr &output : outputs) {
        if (!output->isConnected())
            continue;
        QList<QSize> sizes;
        const auto modes = output->modes();
        for (const KScreen::ModePtr &mode : modes) {
            if (commonSizes.isEmpty() // this is the first connected output
                || commonSizes.contains(mode->size())) // the size existed in previous outputs
            {
                sizes.append(mode->size());
            }
        }
        if (sizes.isEmpty()) // there is no common size
            return;
        commonSizes = sizes;
    }
    if (commonSizes.isEmpty())
        return;
    // Sort the common sizes from the largest to the smallest
    std::sort(commonSizes.begin(), commonSizes.end(), sizeBiggerThan);
    // Select the largest common size
    QSize largestSize = commonSizes.at(0);
    // Put all monitors at (0,0)
    QPoint orig(0, 0);
    for (const KScreen::OutputPtr &output : outputs) {
        if (!output->isConnected())
            continue;
        output->setPos(orig);
        output->setEnabled(true);
        // Select the mode with the largest size and the maximum refresh rate
        float maxRefreshRate = 0.0;
        const auto outputModes = output->modes();
        for (const KScreen::ModePtr &mode : outputModes) {
            if (mode->size() == largestSize && maxRefreshRate < mode->refreshRate()) {
                output->setCurrentModeId(mode->id());
                maxRefreshRate = mode->refreshRate();
            }
        }
    }
}

void FastMenu::onlyFirst()
{
    bool foundOk = false;
    const KScreen::OutputList outputs = mConfig->outputs();
    for (const KScreen::OutputPtr &output : outputs) {
        if (!output->isConnected())
            continue;
        output->setPos(QPoint(0, 0));
        output->setEnabled(!foundOk);
        foundOk = true;
    }
}

void FastMenu::onlySecond()
{
    bool foundOk = true;
    const KScreen::OutputList outputs = mConfig->outputs();
    for (const KScreen::OutputPtr &output : outputs) {
        if (!output->isConnected())
            continue;
        output->setPos(QPoint(0, 0));
        output->setEnabled(!foundOk);
        foundOk = false;
    }
}

void FastMenu::onSelectionChanged(int index)
{
    switch((Options) index) {
    case Extended:
        KScreenUtils::extended(mConfig);
        break;
    case Unified:
        unified();
        break;
    case OnlyFirst:
        onlyFirst();
        break;
    case OnlySecond:
        onlySecond();
        break;
    case None:
        return;
        break;
    };

    if( KScreenUtils::applyConfig(mConfig, mOldConfig) )
        mOldConfig = mConfig->clone();
}
