/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2013 Christian Surlykke
 *            2014 Luís Pereira <luis.artur.pereira.gmail.com>
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

#include <QListWidgetItem>
#include <QDebug>
#include <QIcon>
#include <QPixmap>
#include <QListWidget>
#include <QMimeDatabase>
#include <QMimeType>
#include <QDateTime>
#include <QFileInfo>

#include <XdgIcon>
#include <XdgDesktopFile>
#include <XdgDirs>
#include <LXQt/Settings>


#include "mimetypeviewer.h"
#include "ui_mimetypeviewer.h"

#include "applicationchooser.h"


enum ItemTypeEntries {
    GroupType = 1001,
    EntrieType = 1002
};

static bool mimeTypeLessThan(const QMimeType& m1, const QMimeType& m2)
{
    return m1.name() < m2.name();
}

void MimetypeViewer::loadAllMimeTypes()
{
    mediaTypes.clear();
    mGroupItems.clear();
    mItemList.clear();
    QStringList selectedMimeTypes;

    QMimeDatabase db;
    QList<QMimeType> mimetypes = db.allMimeTypes();

    qSort(mimetypes.begin(), mimetypes.end(), mimeTypeLessThan);
    foreach (const QMimeType &mt, mimetypes) {
        const QString mimetype = mt.name();
        const int i = mimetype.indexOf(QLatin1Char('/'));
        const QString mediaType = mimetype.left(i);
        const QString subType = mimetype.mid(i + 1);

        MimeTypeData* data = new MimeTypeData(mt);

        if (!mediaTypes.contains(mediaType)) { // A new type of media
            mediaTypes.append(mediaType);
            QTreeWidgetItem *item = new QTreeWidgetItem(widget.mimetypeTreeWidget, GroupType);
            item->setText(0, mediaType);
            widget.mimetypeTreeWidget->insertTopLevelItem(0, item);
            mGroupItems.insert(mediaType, item);
        }
        QTreeWidgetItem *item = new QTreeWidgetItem(mGroupItems.value(mediaType), EntrieType);
        QVariant v;
        v.setValue(*data);
        item->setData(0, Qt::UserRole, v);
        item->setText(0, subType);
        mItemList.append(item);
    }

    widget.mimetypeTreeWidget->resizeColumnToContents(1);
    widget.mimetypeTreeWidget->show();
}


MimetypeViewer::MimetypeViewer(QWidget *parent)
    : QDialog(parent)
{
    widget.setupUi(this);
    addSearchIcon();
    widget.searchTermLineEdit->setEnabled(false);

    connect(widget.searchTermLineEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(filter(const QString&)));

    connect(widget.chooseApplicationsButton, SIGNAL(clicked()), this, SLOT(chooseApplication()));
    connect(widget.dialogButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(dialogButtonBoxClicked(QAbstractButton*)));

    QString mimeappsListPath(XdgDirs::dataHome(true) + "/applications/mimeapps.list");
    mDefaultsList = new QSettings(mimeappsListPath, XdgDesktopFileCache::desktopFileSettingsFormat(), this);
    mSettingsCache = new LxQt::SettingsCache(mDefaultsList);
    mSettingsCache->loadFromSettings();
    initializeMimetypeTreeView();
    loadAllMimeTypes();

    connect(widget.mimetypeTreeWidget, SIGNAL(itemSelectionChanged()),
            this, SLOT(currentMimetypeChanged()));
}

MimetypeViewer::~MimetypeViewer()
{
}

void MimetypeViewer::addSearchIcon()
{
    QIcon searchIcon = QIcon::fromTheme("system-search");
    if (searchIcon.isNull())
        return;

    widget.searchTermLineEdit->setTextMargins(0, 0, 30, 0);
    QHBoxLayout *hBoxLayout = new QHBoxLayout(widget.searchTermLineEdit);
    hBoxLayout->setContentsMargins(0,0,0,0);
    widget.searchTermLineEdit->setLayout(hBoxLayout);
    QLabel *searchIconLabel = new QLabel(widget.searchTermLineEdit);
    searchIconLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    searchIconLabel->setMinimumHeight(30);
    searchIconLabel->setMinimumWidth(30);

    searchIconLabel->setPixmap(searchIcon.pixmap(QSize(20,20)));
    hBoxLayout->addWidget(searchIconLabel, 0, Qt::AlignRight | Qt::AlignVCenter);
}


void MimetypeViewer::initializeMimetypeTreeView()
{
    currentMimetypeChanged();
    widget.mimetypeTreeWidget->setColumnCount(2);
    widget.mimetypeTreeWidget->setFocus();
    widget.searchTermLineEdit->setEnabled(true);
}

void MimetypeViewer::currentMimetypeChanged()
{
    widget.iconLabel->hide();
    widget.descriptionLabel->setText(tr("None"));
    widget.mimetypeGroupBox->setEnabled(false);

    widget.patternsLabel->clear();
    widget.patternsGroupBox->setEnabled(false);

    widget.appIcon->hide();
    widget.applicationLabel->clear();
    widget.applicationsGroupBox->setEnabled(false);

    QTreeWidgetItem *sel = widget.mimetypeTreeWidget->currentItem();

    if (!sel || sel->type() == GroupType) {
        return;
    }

    MimeTypeData mimeData = sel->data(0, Qt::UserRole).value<MimeTypeData>();

    QMimeDatabase db;
    XdgMimeType mt = db.mimeTypeForName(mimeData.name());
    if (mt.name().isEmpty())
        return;

    m_CurrentMime = mt;

    widget.descriptionLabel->setText(mimeData.comment());

    QIcon icon = m_CurrentMime.icon();
    if (! icon.isNull())
    {
        widget.iconLabel->setPixmap(icon.pixmap(widget.iconLabel->size()));
        widget.iconLabel->show();
    }

    widget.mimetypeGroupBox->setEnabled(true);
    widget.patternsLabel->setText(mimeData.patterns());
    widget.patternsGroupBox->setEnabled(true);

    XdgDesktopFile* defaultApp = XdgDesktopFileCache::getDefaultApp(m_CurrentMime.name());
    if (defaultApp && defaultApp->isValid())
    {
        QString nonLocalizedName = defaultApp->value("Name").toString();
        QString localizedName = defaultApp->localizedValue("Name", nonLocalizedName).toString();
        QIcon appIcon = defaultApp->icon();
        widget.appIcon->setPixmap(appIcon.pixmap(widget.iconLabel->size()));
        widget.appIcon->show();
        widget.applicationLabel->setText(localizedName);
        widget.chooseApplicationsButton->setText(tr("&Change..."));
    }
    else
    {
        widget.applicationLabel->setText(tr("None"));
        widget.chooseApplicationsButton->setText(tr("&Choose..."));
    }

    widget.applicationsGroupBox->setEnabled(true);

}

void MimetypeViewer::filter(const QString& pattern)
{
    QMimeDatabase db;
    MimeTypeData mimeData;
    const int count = widget.mimetypeTreeWidget->topLevelItemCount();

    for (int i = 0; i < widget.mimetypeTreeWidget->topLevelItemCount(); ++i) {
        widget.mimetypeTreeWidget->topLevelItem(i)->setHidden(true);
    }

    foreach(QTreeWidgetItem* it, mItemList) {
        mimeData = it->data(0, Qt::UserRole).value<MimeTypeData>();
        if (pattern.isEmpty() || mimeData.matches(pattern)) {
            const int i = mimeData.name().indexOf(QLatin1Char('/'));
            const QString mediaType = mimeData.name().left(i);
            QTreeWidgetItem* groupItem = mGroupItems.value(mediaType);
            Q_ASSERT(groupItem);
            if (groupItem) {
                groupItem->setHidden(false);
                it->setHidden(false);
            }
        } else {
            it->setHidden(true);
        }
    }
}


void MimetypeViewer::chooseApplication()
{
    ApplicationChooser applicationChooser(m_CurrentMime);
    if (applicationChooser.exec() == QDialog::Accepted && applicationChooser.DefaultApplication())
    {
        QString fileNameNoPath = QFileInfo(applicationChooser.DefaultApplication()->fileName()).fileName();
        mDefaultsList->beginGroup("Default Applications");
        mDefaultsList->setValue(m_CurrentMime.name(), fileNameNoPath);
        mDefaultsList->endGroup();
        currentMimetypeChanged();
    }
    widget.mimetypeTreeWidget->setFocus();
}

void MimetypeViewer::dialogButtonBoxClicked(QAbstractButton* button)
{
    QDialogButtonBox::ButtonRole role = widget.dialogButtonBox->buttonRole(button);
    if (role == QDialogButtonBox::ResetRole)
    {
        mSettingsCache->loadToSettings();
        currentMimetypeChanged();
    }
    else
    {
        close();
    }
}

