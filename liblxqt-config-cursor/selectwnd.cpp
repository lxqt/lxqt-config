/* coded by Ketmar // Vampire Avalon (psyc://ketmar.no-ip.org/~Ketmar)
 * (c)DWTFYW
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details.
 */

// 2014-04-10 modified by Hong Jen Yee (PCMan) for integration with lxqt-config-input

#include <QDebug>

#include "selectwnd.h"

#include <QKeyEvent>
#include <QMessageBox>
#include <QTimer>
#include <QWidget>
#include <QPushButton>
#include <QToolTip>

#include "cfgfile.h"
#include "crtheme.h"
#include "thememodel.h"
#include "itemdelegate.h"

#include "xcrimg.h"
#include "xcrxcur.h"
#include "xcrtheme.h"

#include <LXQt/Settings>
#include <XdgIcon>
#include <QTextStream>
#include <QProcess>

#define HOME_ICON_DIR QDir::homePath() + "/.icons"

SelectWnd::SelectWnd(LXQt::Settings* settings, QWidget *parent) : QWidget(parent), mSettings(settings)
{
    setupUi(this);

    warningLabel->hide();

    mModel = new XCursorThemeModel(this);

    int size = style()->pixelMetric(QStyle::PM_LargeIconSize);
    lbThemes->setModel(mModel);
    lbThemes->setItemDelegate(new ItemDelegate(this));
    lbThemes->setIconSize(QSize(size, size));
    lbThemes->setSelectionMode(QAbstractItemView::SingleSelection);

    // Make sure we find out about selection changes
    connect(lbThemes->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
            SLOT(currentChanged(const QModelIndex &, const QModelIndex &)));
    // display/hide warning label
    connect(mModel, SIGNAL(modelReset()),
                    this, SLOT(handleWarning()));
    connect(mModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
                    this, SLOT(handleWarning()));
    connect(mModel, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
                    this, SLOT(handleWarning()));

    connect(warningLabel, SIGNAL(showDirInfo()),
                    this, SLOT(showDirInfo()));

    // Disable the install button if we can't install new themes to ~/.icons,
    // or Xcursor isn't set up to look for cursor themes there
    btInstall->setEnabled(mModel->searchPaths().contains(HOME_ICON_DIR) && iconsIsWritable());
    // TODO/FIXME: btInstall functionality
    btInstall->hide();
    btRemove->hide();

    //QTimer::singleShot(0, this, SLOT(setCurrent()));

    handleWarning();
}


SelectWnd::~SelectWnd()
{
}

void SelectWnd::setCurrent()
{
    lbThemes->selectionModel()->clear();

    QString ct = getCurrentTheme();
    mAppliedIndex = mModel->defaultIndex();

    if (!ct.isEmpty()) mAppliedIndex = mModel->findIndex(ct);
    else mAppliedIndex = mModel->defaultIndex();

    if (mAppliedIndex.isValid())
    {
        const XCursorThemeData *theme = mModel->theme(mAppliedIndex);
        // Select the current theme
        selectRow(mAppliedIndex);
        lbThemes->scrollTo(mAppliedIndex, QListView::PositionAtCenter);
        // Update the preview widget as well
        if (theme) preview->setTheme(*theme);// else preview->clearTheme();
    }
}

bool SelectWnd::iconsIsWritable() const
{
    const QFileInfo icons = QFileInfo(HOME_ICON_DIR);
    const QFileInfo home = QFileInfo(QDir::homePath());
    return ((icons.exists() && icons.isDir() && icons.isWritable()) || (!icons.exists() && home.isWritable()));
}

/*
void SelectWnd::keyPressEvent(QKeyEvent *e)
{
  if (e->key() == Qt::Key_Escape) close();
}
*/

void SelectWnd::selectRow(int row) const
{
    // Create a selection that stretches across all columns
    QModelIndex from = mModel->index(row, 0);
    QModelIndex to = mModel->index(row, mModel->columnCount()-1);
    QItemSelection selection(from, to);
    lbThemes->selectionModel()->select(selection, QItemSelectionModel::Select);
    lbThemes->selectionModel()->setCurrentIndex(mAppliedIndex, QItemSelectionModel::NoUpdate);
}

void SelectWnd::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous)
    if (current.isValid()) {
        const XCursorThemeData *theme = mModel->theme(current);
        if (theme) {
            preview->setTheme(*theme);
            btRemove->setEnabled(theme->isWritable());
        } else {
            preview->clearTheme();
        }

        // directly apply the current settings
        applyCurrent();
    } else {
        preview->clearTheme();
    }
   //emit changed(mAppliedIndex != current);
}

void SelectWnd::on_btInstall_clicked()
{
    qDebug() << "'install' clicked";
}

void SelectWnd::applyCurrent()
{
    //qDebug() << "'set' clicked";
    const XCursorThemeData *theme = mModel->theme(lbThemes->currentIndex());
    if (!theme) return;
    applyTheme(*theme);
    fixXDefaults(theme->name());

    // call xrdb to merge the new settings in ~/.Xdefaults
    // FIXME: need to check if we're running in X?
    QProcess xrdb;
    xrdb.start("xrdb -merge " + QDir::home().path() + "/.Xdefaults");
    xrdb.waitForFinished();

    // old razor-qt and lxqt versions use $XCURSOR_THEME environment variable
    // for this, but it's less flexible and more problematic. Let's deprecate its use.
    mSettings->beginGroup("Environment");
    mSettings->remove("XCURSOR_THEME"); // ensure that we're not using XCURSOR_THEME
    mSettings->endGroup();
    // save to Mouse/cursor_theme instead
    mSettings->beginGroup("Mouse");
    mSettings->setValue("cursor_theme", theme->name());
    mSettings->endGroup();

    // The XCURSOR_THEME environment varialbe does not work sometimes.
    // Besides, XDefaults values are not used by Qt.
    // So, let's write the new theme name to ~/.icons/default/index.theme.
    // This is the most reliable way.
    // QSettings will encode the group name "Icon Theme" to "Icon%20Theme" and there is no way to turn it off.
    // So let's not use it here. :-(
    QString dirPath = HOME_ICON_DIR + "/default";
    QDir().mkpath(dirPath); // ensure the existence of the ~/.icons/default dir
    QFile indexTheme(dirPath + "/index.theme");
    if(indexTheme.open(QIODevice::WriteOnly|QIODevice::Truncate))
    {
        QTextStream(&indexTheme) <<
        "# Written by lxqt-config-appearance\n" <<
        "[Icon Theme]\n" <<
        "Name=Default\n" <<
        "Comment=Default cursor theme\n" <<
        "Inherits=" << theme->name() << "\n";
        indexTheme.close();
    }
}

void SelectWnd::on_btRemove_clicked()
{
    qDebug() << "'remove' clicked";
    const XCursorThemeData *theme = mModel->theme(lbThemes->currentIndex());
    if (!theme) return;
    QString ct = getCurrentTheme();
    if (ct == theme->name())
    {
        QMessageBox::warning(this, tr("XCurTheme error"),
                             tr("You can't remove active theme!"), QMessageBox::Ok, QMessageBox::Ok);
        return;
    }
    QDir d(theme->path());
    preview->clearTheme();
    mModel->removeTheme(lbThemes->currentIndex());
    removeXCursorTheme(d);
}

void SelectWnd::handleWarning()
{
        bool empty = mModel->rowCount();
        warningLabel->setVisible(!empty);
        preview->setVisible(empty);
        infoLabel->setVisible(empty);
}

void SelectWnd::showDirInfo()
{
        QToolTip::showText(mapToGlobal(warningLabel->buttonPos()), mModel->searchPaths().join("\n"));
}
