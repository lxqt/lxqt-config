/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt-project.org
 *
 * Copyright: 2012 Razor team
 * Authors:
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "lxqtthemeconfig.h"
#include "ui_lxqtthemeconfig.h"
#include <QTreeWidget>
#include <QStandardPaths>
#include <QProcess>
#include <QItemDelegate>
#include <QPainter>
#include <QMenu>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>

#include <XdgDirs>

/*!
 * \brief Simple delegate to draw system background color below decoration/icon
 * (needed by System theme, which uses widget background and therefore provides semi-transparent preview)
 */
class ThemeDecorator : public QItemDelegate
{
public:
    using QItemDelegate::QItemDelegate;
protected:
    virtual void drawDecoration(QPainter * painter, const QStyleOptionViewItem & option, const QRect & rect, const QPixmap & pixmap) const override
    {
        //Note: can't use QItemDelegate::drawDecoration, because it is ignoring pixmap,
        //if the icon is valid (and that is set in paint())
        if (pixmap.isNull() || !rect.isValid())
            return;

        QPoint p = QStyle::alignedRect(option.direction, option.decorationAlignment, pixmap.size(), rect).topLeft();
        painter->fillRect(QRect{p, pixmap.size()}, QApplication::palette().color(QPalette::Window));
        painter->drawPixmap(p, pixmap);
    }
};

LXQtThemeConfig::LXQtThemeConfig(LXQt::Settings *settings, StyleConfig *stylePage, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LXQtThemeConfig),
    mSettings(settings),
    mStylePage(stylePage)
{
    ui->setupUi(this);
    {
        QScopedPointer<QAbstractItemDelegate> p{ui->lxqtThemeList->itemDelegate()};
        ui->lxqtThemeList->setItemDelegate(new ThemeDecorator{this});
    }

    const QList<LXQt::LXQtTheme> themes = LXQt::LXQtTheme::allThemes();
    for(const LXQt::LXQtTheme &theme : themes)
    {
        QString themeName = theme.name();
        themeName[0] = themeName[0].toTitleCase();
        if (theme.path().contains(XdgDirs::dataHome(false) + QStringLiteral("/")))
        {
            themeName += QStringLiteral(" ") + tr("(User Theme)");
        }
        QTreeWidgetItem *item = new QTreeWidgetItem(QStringList(themeName));
        if (!theme.previewImage().isEmpty())
        {
            item->setIcon(0, QIcon(theme.previewImage()));
        }
        item->setSizeHint(0, QSize(42, 42)); // make icons non-cropped
        item->setData(0, Qt::UserRole, theme.name());
        ui->lxqtThemeList->addTopLevelItem(item);
    }
    ui->lxqtThemeList->sortItems(0, Qt::AscendingOrder);
    ui->lxqtThemeList->setContextMenuPolicy(Qt::CustomContextMenu);

    initControls();

    connect(ui->lxqtThemeList, &QTreeWidget::currentItemChanged, this, &LXQtThemeConfig::onCurrentItemChanged);
    connect(ui->wallpaperOverride, &QAbstractButton::clicked, this, &LXQtThemeConfig::settingsChanged);
    connect(ui->paletteOverride, &QAbstractButton::clicked, this, &LXQtThemeConfig::onPaletteOverrideChanged);

    connect(ui->lxqtThemeList, &QTreeWidget::itemDoubleClicked, this, &LXQtThemeConfig::doubleClicked);
    connect(ui->lxqtThemeList, &QWidget::customContextMenuRequested, this, &LXQtThemeConfig::contextMenu);
}


LXQtThemeConfig::~LXQtThemeConfig()
{
    delete ui;
}


void LXQtThemeConfig::initControls()
{
    QString currentTheme = mSettings->value(QStringLiteral("theme")).toString();

    QTreeWidgetItemIterator it(ui->lxqtThemeList);
    while (*it) {
        if ((*it)->data(0, Qt::UserRole).toString() == currentTheme)
        {
            ui->lxqtThemeList->setCurrentItem((*it));
            break;
        }
        ++it;
    }

    ui->wallpaperOverride->setChecked(mSettings->value(QStringLiteral("wallpaper_override")).toBool());
    ui->paletteOverride->setChecked(mSettings->value(QStringLiteral("palette_override")).toBool());

    update();
}

void LXQtThemeConfig::applyLxqtTheme()
{
    QTreeWidgetItem* item = ui->lxqtThemeList->currentItem();
    if (!item)
        return;

    LXQt::LXQtTheme currentTheme{mSettings->value(QStringLiteral("theme")).toString()};
    QVariant themeName = item->data(0, Qt::UserRole);
    if(mSettings->value(QStringLiteral("theme")) != themeName)
        mSettings->setValue(QStringLiteral("theme"), themeName);

    if (ui->wallpaperOverride->isChecked())
    { // set the wallpaper
        LXQt::LXQtTheme theme(themeName.toString());
        if(theme.isValid()) {
            QString wallpaper = theme.desktopBackground();
            if(!wallpaper.isEmpty()) {
                // call pcmanfm-qt to update wallpaper
                QStringList args;
                args << QStringLiteral("--set-wallpaper") << wallpaper;
                QProcess::startDetached(QStringLiteral("pcmanfm-qt"), args);
            }
        }
    }

    if(mSettings->value(QStringLiteral("wallpaper_override")) != ui->wallpaperOverride->isChecked())
        mSettings->setValue(QStringLiteral("wallpaper_override"), ui->wallpaperOverride->isChecked());

    if(mSettings->value(QStringLiteral("palette_override")) != ui->paletteOverride->isChecked())
        mSettings->setValue(QStringLiteral("palette_override"), ui->paletteOverride->isChecked());
}

void LXQtThemeConfig::doubleClicked(QTreeWidgetItem *item, int /*column*/)
{
    if (!item)
        return;

    LXQt::LXQtTheme theme{item->data(0, Qt::UserRole).toString()};
    if (!theme.isValid())
        return;

    // first try "qtxdg-mat"; fall back to QDesktopServices if we are not inside an LXQt session
    if (!QProcess::startDetached(QStringLiteral("qtxdg-mat"), QStringList() << QStringLiteral("open") << theme.path()))
    {
        QDesktopServices::openUrl(QUrl(theme.path()));
    }
}

void LXQtThemeConfig::contextMenu(const QPoint& p)
{
    if (!ui->lxqtThemeList->itemAt(p))
        return;

    QMenu menu;
    QAction *a = menu.addAction(tr("Open theme folder"));
    connect(a, &QAction::triggered, [this, p] {
        doubleClicked(ui->lxqtThemeList->itemAt(p), 0);
    });
}

void LXQtThemeConfig::loadThemePalette()
{
    QTreeWidgetItem* current = ui->lxqtThemeList->currentItem();
    if (!ui->paletteOverride->isChecked() || !mStylePage || !current)
        return;
    QString themeName = current->data(0, Qt::UserRole).toString();
    if (themeName.isEmpty())
        return;
    themeName[0] = themeName[0].toTitleCase(); // palette names should be as they appear in GUI
    auto paths = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    paths.removeDuplicates();
    for (const auto &path : std::as_const(paths))
    {
        QDir dir(path + QLatin1String("/lxqt/palettes"));
        if (dir.exists())
        {
            if (dir.exists(themeName))
            {
                const QString palettePath = path + QStringLiteral("/lxqt/palettes/") + themeName;
                mStylePage->loadPaletteFile(palettePath);
                break;
            }
        }
    }
}

void LXQtThemeConfig::onPaletteOverrideChanged(bool checked)
{
    emit settingsChanged();
    if (checked)
        loadThemePalette();
}

void LXQtThemeConfig::onCurrentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)
{
    emit settingsChanged();
    if (ui->paletteOverride->isChecked())
        loadThemePalette();
}
