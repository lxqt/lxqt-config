/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt-project.org/
 *
 * Copyright: 2014 LXQt team
 * Authors:
 *   Hong Jen Yee (PCMan) <pcman.tw@gmail.com>
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

#include "styleconfig.h"
#include "ui_styleconfig.h"
#include "ui_palettes.h"
#include <QDebug>
#include <QStyleFactory>
#include <QToolBar>
#include <QSettings>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaEnum>
#include <QToolBar>
#include <QInputDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QGuiApplication>

StyleConfig::StyleConfig(LXQt::Settings* settings, QSettings* qtSettings, QWidget* parent) :
    QWidget(parent),
    ui(new Ui::StyleConfig),
    mQtSettings(qtSettings),
    mSettings(settings)
{
    ui->setupUi(this);

    initControls();

    connect(ui->qtComboBox, QOverload<int>::of(&QComboBox::activated), this, &StyleConfig::settingsChanged);
    connect(ui->toolButtonStyle, QOverload<int>::of(&QComboBox::activated), this, &StyleConfig::settingsChanged);
    connect(ui->singleClickActivate, &QAbstractButton::clicked, this, &StyleConfig::settingsChanged);

    connect(ui->winColorLabel, &ColorLabel::colorChanged, this, &StyleConfig::settingsChanged);
    connect(ui->baseColorLabel, &ColorLabel::colorChanged, this, &StyleConfig::settingsChanged);
    connect(ui->highlightColorLabel, &ColorLabel::colorChanged, this, &StyleConfig::settingsChanged);
    connect(ui->windowTextColorLabel, &ColorLabel::colorChanged, this, &StyleConfig::settingsChanged);
    connect(ui->viewTextColorLabel, &ColorLabel::colorChanged, this, &StyleConfig::settingsChanged);
    connect(ui->highlightedTextColorLabel, &ColorLabel::colorChanged, this, &StyleConfig::settingsChanged);
    connect(ui->linkColorLabel, &ColorLabel::colorChanged, this, &StyleConfig::settingsChanged);
    connect(ui->linkVisitedColorLabel, &ColorLabel::colorChanged, this, &StyleConfig::settingsChanged);
    connect(ui->tooltipColorLabel, &ColorLabel::colorChanged, this, &StyleConfig::settingsChanged);
    connect(ui->tooltipTextColorLabel, &ColorLabel::colorChanged, this, &StyleConfig::settingsChanged);

    connect(ui->defaultPaletteBtn, &QAbstractButton::clicked, [this] {
        QColor winColor(239, 239, 239);
        QPalette defaultPalette(winColor);
        ui->winColorLabel->setColor(winColor, true);
        ui->baseColorLabel->setColor(defaultPalette.color(QPalette::Active,QPalette::Base), true);
        // Qt's default highlight color may be different from that of Fusion
        ui->highlightColorLabel->setColor(QColor(60, 140, 230), true);
        ui->windowTextColorLabel->setColor(defaultPalette.color(QPalette::Active,QPalette::WindowText), true);
        ui->viewTextColorLabel->setColor(defaultPalette.color(QPalette::Active,QPalette::Text), true);
        ui->highlightedTextColorLabel->setColor(defaultPalette.color(QPalette::Active,QPalette::HighlightedText), true);
        ui->linkColorLabel->setColor(defaultPalette.color(QPalette::Active,QPalette::Link), true);
        ui->linkVisitedColorLabel->setColor(defaultPalette.color(QPalette::Active,QPalette::LinkVisited), true);
        // tooltips use the Inactive color group
        ui->tooltipColorLabel->setColor(defaultPalette.color(QPalette::Inactive,QPalette::ToolTipBase), true);
        ui->tooltipTextColorLabel->setColor(defaultPalette.color(QPalette::Inactive,QPalette::ToolTipText), true);
    });

    connect(ui->savePaletteBtn, &QAbstractButton::clicked, this, &StyleConfig::savePalette);
    connect(ui->loadPaletteBtn, &QAbstractButton::clicked, this, &StyleConfig::loadPalette);
}


StyleConfig::~StyleConfig()
{
    delete ui;
}


void StyleConfig::initControls()
{

    QStringList qtThemes = QStyleFactory::keys();

    // read other widget related settings from LXQt settings.
    QByteArray tb_style = mSettings->value(QStringLiteral("tool_button_style")).toByteArray();
    // convert toolbar style name to value
    QMetaEnum me = QToolBar::staticMetaObject.property(QToolBar::staticMetaObject.indexOfProperty("toolButtonStyle")).enumerator();
    int val = me.keyToValue(tb_style.constData());
    if(val == -1)
      val = Qt::ToolButtonTextBesideIcon;
    ui->toolButtonStyle->setCurrentIndex(val);

    // activate item views with single click
    ui->singleClickActivate->setChecked( mSettings->value(QStringLiteral("single_click_activate"), false).toBool());

    // Fill Qt themes
    ui->qtComboBox->clear();
    ui->qtComboBox->addItems(qtThemes);


    // Qt style
    mQtSettings->beginGroup(QLatin1String("Qt"));
    ui->qtComboBox->setCurrentText(mQtSettings->value(QStringLiteral("style")).toString());
    mQtSettings->endGroup();

    // palette
    mSettings->beginGroup(QLatin1String("Palette"));
    QColor color;
    color = QColor::fromString(mSettings->value(QStringLiteral("window_color")).toString());
    if (!color.isValid())
        color = QGuiApplication::palette().color(QPalette::Active,QPalette::Window);
    ui->winColorLabel->setColor(color);

    color = QColor::fromString(mSettings->value(QStringLiteral("base_color")).toString());
    if (!color.isValid())
        color = QGuiApplication::palette().color(QPalette::Active,QPalette::Base);
    ui->baseColorLabel->setColor(color);

    color = QColor::fromString(mSettings->value(QStringLiteral("highlight_color")).toString());
    if (!color.isValid())
        color = QGuiApplication::palette().color(QPalette::Active,QPalette::Highlight);
    ui->highlightColorLabel->setColor(color);

    color = QColor::fromString(mSettings->value(QStringLiteral("window_text_color")).toString());
    if (!color.isValid())
        color = QGuiApplication::palette().color(QPalette::Active,QPalette::WindowText);
    ui->windowTextColorLabel->setColor(color);

    color = QColor::fromString(mSettings->value(QStringLiteral("text_color")).toString());
    if (!color.isValid())
        color = QGuiApplication::palette().color(QPalette::Active,QPalette::Text);
    ui->viewTextColorLabel->setColor(color);

    color = QColor::fromString(mSettings->value(QStringLiteral("highlighted_text_color")).toString());
    if (!color.isValid())
        color = QGuiApplication::palette().color(QPalette::Active,QPalette::HighlightedText);
    ui->highlightedTextColorLabel->setColor(color);

    color = QColor::fromString(mSettings->value(QStringLiteral("link_color")).toString());
    if (!color.isValid())
        color = QGuiApplication::palette().color(QPalette::Active,QPalette::Link);
    ui->linkColorLabel->setColor(color);

    color = QColor::fromString(mSettings->value(QStringLiteral("link_visited_color")).toString());
    if (!color.isValid())
        color = QGuiApplication::palette().color(QPalette::Active,QPalette::LinkVisited);
    ui->linkVisitedColorLabel->setColor(color);

    // tooltips use the Inactive color group
    color = QColor::fromString(mSettings->value(QStringLiteral("tooltip_base_color")).toString());
    if (!color.isValid())
        color = QGuiApplication::palette().color(QPalette::Inactive,QPalette::ToolTipBase);
    ui->tooltipColorLabel->setColor(color);

    color = QColor::fromString(mSettings->value(QStringLiteral("tooltip_text_color")).toString());
    if (!color.isValid())
        color = QGuiApplication::palette().color(QPalette::Inactive,QPalette::ToolTipText);
    ui->tooltipTextColorLabel->setColor(color);

    mSettings->endGroup();

    update();
}

void StyleConfig::applyStyle()
{
    // Qt style
    QString themeName = ui->qtComboBox->currentText();;
    mQtSettings->beginGroup(QLatin1String("Qt"));
    if(mQtSettings->value(QStringLiteral("style")).toString() != themeName)
        mQtSettings->setValue(QStringLiteral("style"), themeName);
    mQtSettings->endGroup();

    // palette
    mSettings->beginGroup(QLatin1String("Palette"));
    QColor color = ui->winColorLabel->getColor();
    QColor oldColor;
    oldColor = QColor::fromString(mSettings->value(QStringLiteral("window_color")).toString());
    if (color != oldColor)
        mSettings->setValue(QStringLiteral("window_color"), color.name());

    color = ui->baseColorLabel->getColor();
    oldColor = QColor::fromString(mSettings->value(QStringLiteral("base_color")).toString());
    if (color != oldColor)
        mSettings->setValue(QStringLiteral("base_color"), color.name());

    color = ui->highlightColorLabel->getColor();
    oldColor = QColor::fromString(mSettings->value(QStringLiteral("highlight_color")).toString());
    if (color != oldColor)
        mSettings->setValue(QStringLiteral("highlight_color"), color.name());

    color = ui->windowTextColorLabel->getColor();
    oldColor = QColor::fromString(mSettings->value(QStringLiteral("window_text_color")).toString());
    if (color != oldColor)
        mSettings->setValue(QStringLiteral("window_text_color"), color.name());

    color = ui->viewTextColorLabel->getColor();
    oldColor = QColor::fromString(mSettings->value(QStringLiteral("text_color")).toString());
    if (color != oldColor)
        mSettings->setValue(QStringLiteral("text_color"), color.name());

    color = ui->highlightedTextColorLabel->getColor();
    oldColor = QColor::fromString(mSettings->value(QStringLiteral("highlighted_text_color")).toString());
    if (color != oldColor)
        mSettings->setValue(QStringLiteral("highlighted_text_color"), color.name());

    color = ui->linkColorLabel->getColor();
    oldColor = QColor::fromString(mSettings->value(QStringLiteral("link_color")).toString());
    if (color != oldColor)
        mSettings->setValue(QStringLiteral("link_color"), color.name());

    color = ui->linkVisitedColorLabel->getColor();
    oldColor = QColor::fromString(mSettings->value(QStringLiteral("link_visited_color")).toString());
    if (color != oldColor)
        mSettings->setValue(QStringLiteral("link_visited_color"), color.name());

    color = ui->tooltipColorLabel->getColor();
    oldColor = QColor::fromString(mSettings->value(QStringLiteral("tooltip_base_color")).toString());
    if (color != oldColor)
        mSettings->setValue(QStringLiteral("tooltip_base_color"), color.name());

    color = ui->tooltipTextColorLabel->getColor();
    oldColor = QColor::fromString(mSettings->value(QStringLiteral("tooltip_text_color")).toString());
    if (color != oldColor)
        mSettings->setValue(QStringLiteral("tooltip_text_color"), color.name());

    mSettings->endGroup();

    // single click setting
    if(mSettings->value(QStringLiteral("single_click_activate")).toBool() !=  ui->singleClickActivate->isChecked()) {
        mSettings->setValue(QStringLiteral("single_click_activate"), ui->singleClickActivate->isChecked());
    }

   // tool button style
   int index = ui->toolButtonStyle->currentIndex();
    // convert style value to string
    QMetaEnum me = QToolBar::staticMetaObject.property(QToolBar::staticMetaObject.indexOfProperty("toolButtonStyle")).enumerator();
    if(index == -1)
        index = Qt::ToolButtonTextBesideIcon;
    const char* str = me.valueToKey(index);
    if(str && mSettings->value(QStringLiteral("tool_button_style")) != QString::fromUtf8(str))
    {
        mSettings->setValue(QStringLiteral("tool_button_style"), QString::fromUtf8(str));
        mSettings->sync();
        emit updateOtherSettings();
    }
}

void StyleConfig::savePalette()
{
    bool ok;
    QString name = QInputDialog::getText(this, tr("Save Palette"), tr("Palette name:"), QLineEdit::Normal, QString(), &ok);
    if (!ok || name.isEmpty())
        return;
    name = name.replace(QLatin1String("/"), QLatin1String(" ")).simplified();
    const QString paletteFile = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                                + QLatin1String("/lxqt/palettes/") + name;
    if (QFile::exists(paletteFile))
    {
        QMessageBox::StandardButton btn =
        QMessageBox::question(this,
                              tr("Save Palette"),
                              tr("A palette with the same name exists.\nDo you want to replace it?"));
        if (btn != QMessageBox::Yes)
            return;
    }

    QSettings settings(paletteFile, QSettings::IniFormat);
    settings.beginGroup(QStringLiteral("Palette"));

    QColor color = ui->winColorLabel->getColor();
    settings.setValue(QStringLiteral("window_color"), color.name());

    color = ui->baseColorLabel->getColor();
    settings.setValue(QStringLiteral("base_color"), color.name());

    color = ui->highlightColorLabel->getColor();
    settings.setValue(QStringLiteral("highlight_color"), color.name());

    color = ui->windowTextColorLabel->getColor();
    settings.setValue(QStringLiteral("window_text_color"), color.name());

    color = ui->viewTextColorLabel->getColor();
    settings.setValue(QStringLiteral("text_color"), color.name());

    color = ui->highlightedTextColorLabel->getColor();
    settings.setValue(QStringLiteral("highlighted_text_color"), color.name());

    color = ui->linkColorLabel->getColor();
    settings.setValue(QStringLiteral("link_color"), color.name());

    color = ui->linkVisitedColorLabel->getColor();
    settings.setValue(QStringLiteral("link_visited_color"), color.name());

    color = ui->tooltipColorLabel->getColor();
    settings.setValue(QStringLiteral("tooltip_base_color"), color.name());

    color = ui->tooltipTextColorLabel->getColor();
    settings.setValue(QStringLiteral("tooltip_text_color"), color.name());

    settings.endGroup();
}

void StyleConfig::loadPalette()
{
    class PalettesDialog : public QDialog {
    public:
        explicit PalettesDialog(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags())
            : QDialog(parent, f) {
            ui.setupUi(this);
            ui.listWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
            ui.listWidget->setSortingEnabled(true);
            ui.listWidget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
            updateList();
            connect(ui.listWidget, &QListWidget::itemDoubleClicked, this, &QDialog::accept);
            connect(ui.removeButton, &QAbstractButton::clicked, this, &PalettesDialog::removeSelectedItems);
            connect(ui.lineEdit, &QLineEdit::textChanged, this, &PalettesDialog::filter);
            if (parent)
                resize(QSize (parent->size().width() / 2, 3 * parent->size().height() / 4));
        }

        // these are needed for translations
        void setQuestionTitle(const QString &title) {
            qTitle = title;
        }
        void setQuestion(const QString &text) {
            qText = text;
        }

        QString currentPalette() {
            if (auto cur = ui.listWidget->currentItem())
            {
                if (cur->isSelected())
                    return cur->data(Qt::UserRole).toString();
            }
            return QString();
        }

    private slots:
        void removeSelectedItems() {
            const auto items = ui.listWidget->selectedItems();
            if (items.isEmpty())
                return;
            QMessageBox::StandardButton btn = QMessageBox::question(this, qTitle, qText);
            if (btn != QMessageBox::Yes)
                return;
            QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)
                           + QLatin1String("/lxqt/palettes/");
            for (const auto &item : items)
                QFile::remove(path + item->text());
            updateList();
        }

        void filter() {
            QString filterStr = ui.lineEdit->text();
            int I = -1;
            for (int i = 0; i < ui.listWidget->count(); ++i)
            {
                if (auto item = ui.listWidget->item(i))
                {
                    bool visible(item->text().contains(filterStr, Qt::CaseInsensitive));
                    if (I < 0 && visible)
                        I  = i;
                    item->setHidden(!visible);
                }
            }
            if (I > -1) // select the first visible item
                ui.listWidget->setCurrentRow(I);
        }

    private:
        void updateList() {
            ui.listWidget->clear();
            auto paths = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
            paths.removeDuplicates();
            QString filterStr = ui.lineEdit->text();
            QStringList names;
            for (const auto &path : std::as_const(paths))
            {
                QDir dir(path + QLatin1String("/lxqt/palettes"));
                if (dir.exists())
                {
                    const auto entries = dir.entryList(QDir::Files);
                    for (const auto &entry : entries)
                    {
                        if (names.contains(entry)) // already added with higher priority
                            continue;
                        names << entry;
                        auto *item = new QListWidgetItem(entry, ui.listWidget);
                        QString palettePath = path + QStringLiteral("/lxqt/palettes/") + entry;
                        item->setData(Qt::UserRole, palettePath);
                        item->setHidden(!entry.contains(filterStr, Qt::CaseInsensitive));
                        ui.listWidget->addItem(item);
                    }
                }
            }
            ui.listWidget->setCurrentRow(0);
        }

        Ui::PalettesDialog ui;
        QString qTitle;
        QString qText;
    };

    PalettesDialog dialog(this);
    dialog.setQuestionTitle(tr("Remove Palettes"));
    dialog.setQuestion(tr("Do you really want to remove selected palette(s)?\nRoot palettes will remain intact if existing."));

    if (dialog.exec() == QDialog::Accepted)
    {
        loadPaletteFile(dialog.currentPalette());
    }
}

void StyleConfig::loadPaletteFile(const QString& paletteFile)
{ // set color labels
    if (paletteFile.isEmpty())
        return;

    QSettings settings(paletteFile, QSettings::IniFormat);
    settings.beginGroup(QStringLiteral("Palette"));

    QColor color;
    color = QColor::fromString(settings.value(QStringLiteral("window_color")).toString());
    if (!color.isValid())
        color = QGuiApplication::palette().color(QPalette::Active,QPalette::Window);
    ui->winColorLabel->setColor(color, true);

    color = QColor::fromString(settings.value(QStringLiteral("base_color")).toString());
    if (!color.isValid())
        color = QGuiApplication::palette().color(QPalette::Active,QPalette::Base);
    ui->baseColorLabel->setColor(color, true);

    color = QColor::fromString(settings.value(QStringLiteral("highlight_color")).toString());
    if (!color.isValid())
        color = QGuiApplication::palette().color(QPalette::Active,QPalette::Highlight);
    ui->highlightColorLabel->setColor(color, true);

    color = QColor::fromString(settings.value(QStringLiteral("window_text_color")).toString());
    if (!color.isValid())
        color = QGuiApplication::palette().color(QPalette::Active,QPalette::WindowText);
    ui->windowTextColorLabel->setColor(color, true);

    color = QColor::fromString(settings.value(QStringLiteral("text_color")).toString());
    if (!color.isValid())
        color = QGuiApplication::palette().color(QPalette::Active,QPalette::Text);
    ui->viewTextColorLabel->setColor(color, true);

    color = QColor::fromString(settings.value(QStringLiteral("highlighted_text_color")).toString());
    if (!color.isValid())
        color = QGuiApplication::palette().color(QPalette::Active,QPalette::HighlightedText);
    ui->highlightedTextColorLabel->setColor(color, true);

    color = QColor::fromString(settings.value(QStringLiteral("link_color")).toString());
    if (!color.isValid())
        color = QGuiApplication::palette().color(QPalette::Active,QPalette::Link);
    ui->linkColorLabel->setColor(color, true);

    color = QColor::fromString(settings.value(QStringLiteral("link_visited_color")).toString());
    if (!color.isValid())
        color = QGuiApplication::palette().color(QPalette::Active,QPalette::LinkVisited);
    ui->linkVisitedColorLabel->setColor(color, true);

    // tooltips use the Inactive color group
    color = QColor::fromString(settings.value(QStringLiteral("tooltip_base_color")).toString());
    if (!color.isValid())
        color = QGuiApplication::palette().color(QPalette::Inactive,QPalette::ToolTipBase);
    ui->tooltipColorLabel->setColor(color, true);

    color = QColor::fromString(settings.value(QStringLiteral("tooltip_text_color")).toString());
    if (!color.isValid())
        color = QGuiApplication::palette().color(QPalette::Inactive,QPalette::ToolTipText);
    ui->tooltipTextColorLabel->setColor(color, true);

    settings.endGroup();
}
