/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2014  <copyright holder> <email>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "keyboardlayoutconfig.h"
#include <QProcess>
#include <QFile>
#include <QHash>
#include <QDebug>
#include "selectkeyboardlayoutdialog.h"
#include <lxqt/LxQtSettings>

KeyboardLayoutConfig::KeyboardLayoutConfig(LxQt::Settings* _settings, QWidget* parent):
  QWidget(parent),
  settings(_settings) {
  ui.setupUi(this);

  loadLists();
  loadSettings();
  initControls();

  connect(ui.addLayout, SIGNAL(clicked(bool)), SLOT(onAddLayout()));
  connect(ui.removeLayout, SIGNAL(clicked(bool)), SLOT(onRemoveLayout()));
  connect(ui.moveUp, SIGNAL(clicked(bool)), SLOT(onMoveUp()));
  connect(ui.moveDown, SIGNAL(clicked(bool)), SLOT(onMoveDown()));
  connect(ui.keyboardModel, SIGNAL(currentIndexChanged(int)), SLOT(accept()));
  connect(ui.switchKey, SIGNAL(currentIndexChanged(int)), SLOT(accept()));
}

KeyboardLayoutConfig::~KeyboardLayoutConfig() {
}

void KeyboardLayoutConfig::loadSettings() {
  // load current settings from the output of setxkbmap command
  QProcess setxkbmap;
  setxkbmap.start(QLatin1String("setxkbmap -query -verbose 5"));
  setxkbmap.waitForFinished();
  if(setxkbmap.exitStatus() == QProcess::NormalExit) {
    while(!setxkbmap.atEnd()) {
      QByteArray line = setxkbmap.readLine();
      if(line.startsWith("model:")) {
        keyboardModel_ = QString::fromLatin1(line.mid(6).trimmed());
      }
      else if(line.startsWith("layout:")) {
        QList<QByteArray> items = line.mid(7).trimmed().split(',');
        Q_FOREACH(QByteArray item, items) {
          QString lang;
          QString variant;
          int p = item.indexOf('(');
          if(p >= 0) { // has variant
            int p2 = item.lastIndexOf(')');
            if(p2 >= 0)
              variant = QString::fromLatin1(item.mid(p + 1, (p2 - p - 1)));
            lang = QString::fromLatin1(item.left(p));
          }
          else
            lang = QString::fromLatin1(item);
          // add the current lang/variant parit to the list
          currentLayouts_.append(QPair<QString, QString>(lang, variant));
        }
      }
      else if(line.startsWith("options:")) {
        QList<QByteArray> options = line.mid(9).trimmed().split(',');
        Q_FOREACH(QByteArray option, options) {
          if(option.startsWith("grp:"))
            switchKey_ = QString::fromLatin1(option);
          else
            currentOptions_ << QString::fromLatin1(option);
        }
      }
    }
    setxkbmap.close();
  }
}

enum ListSection{
  NoSection,
  ModelSection,
  LayoutSection,
  VariantSection,
  OptionSection
};

void KeyboardLayoutConfig::loadLists() {
  // load known lists from xkb data files
  // FIXME: maybe we should use different files for different OSes?
  QFile file(QLatin1String("/usr/share/X11/xkb/rules/base.lst"));
  if(file.open(QIODevice::ReadOnly)) {
    ListSection section = NoSection;
    while(!file.atEnd()) {
      QByteArray line = file.readLine().trimmed();
      if(section == NoSection) {
        if(line.startsWith("! model"))
          section = ModelSection;
        else if(line.startsWith("! layout"))
          section = LayoutSection;
        else if(line.startsWith("! variant"))
          section = VariantSection;
        else if(line.startsWith("! option"))
          section = OptionSection;
      }
      else {
        if(line.isEmpty()) {
          section = NoSection;
          continue;
        }
        int sep = line.indexOf(' ');
        QString name = QString::fromLatin1(line, sep);
        while(line[sep] == ' ') // skip spaces
          ++sep;
        QString description = QString::fromUtf8(line.constData() + sep);

        switch(section) {
          case ModelSection: {
            ui.keyboardModel->addItem(description, name);
            break;
          }
          case LayoutSection:
            knownLayouts_[name] = KeyboardLayoutInfo(description);
            break;
          case VariantSection: {
            // the descriptions of variants are prefixed by their language ids
            sep = description.indexOf(": ");
            if(sep >= 0) {
              QString lang = description.left(sep);
              QMap<QString, KeyboardLayoutInfo>::iterator it = knownLayouts_.find(lang);
              if(it != knownLayouts_.end()) {
                KeyboardLayoutInfo& info = *it;
                info.variants.append(LayoutVariantInfo(name, description.mid(sep + 2)));
              }
            }
            break;
          }
          case OptionSection:
            if(line.startsWith("grp:")) { // key used to switch to another layout
              ui.switchKey->addItem(description, name);
            }
            break;
          default:;
        }
      }
    }
    file.close();
  }
}

void KeyboardLayoutConfig::initControls() {
  QList<QPair<QString, QString> >::iterator it;
  for(it = currentLayouts_.begin(); it != currentLayouts_.end(); ++it) {
    QString name = it->first;
    QString variant = it->second;
    addLayout(name, variant);
  }
  
  int n = ui.keyboardModel->count();
  int row;
  for(row = 0; row < n; ++row) {
    if(ui.keyboardModel->itemData(row, Qt::UserRole).toString() == keyboardModel_) {
      ui.keyboardModel->setCurrentIndex(row);
      break;
    }
  }

  n = ui.switchKey->count();
  for(row = 0; row < n; ++row) {
    if(ui.switchKey->itemData(row, Qt::UserRole).toString() == switchKey_) {
      ui.switchKey->setCurrentIndex(row);
      break;
    }
  }
  
}

void KeyboardLayoutConfig::addLayout(QString name, QString variant) {
  qDebug() << "add" << name << variant;
  const KeyboardLayoutInfo& info = knownLayouts_.value(name);
  QTreeWidgetItem* item = new QTreeWidgetItem();
  item->setData(0, Qt::DisplayRole, info.description);
  item->setData(0, Qt::UserRole, name);
  const LayoutVariantInfo* vinfo = info.findVariant(variant);
  if(vinfo) {
    item->setData(1, Qt::DisplayRole, vinfo->description);
    item->setData(1, Qt::UserRole, variant);
  }
  ui.layouts->addTopLevelItem(item);
}

void KeyboardLayoutConfig::reset() {
  ui.layouts->clear();
  initControls();
  accept();
}

void KeyboardLayoutConfig::accept() {
  // call setxkbmap to apply the changes
  QProcess setxkbmap;
  // clear existing options
  setxkbmap.start("setxkbmap -option");
  setxkbmap.waitForFinished();
  setxkbmap.close();

  QString command = "setxkbmap";
  // set keyboard model
  QString model;
  int cur_model = ui.keyboardModel->currentIndex();
  if(cur_model >= 0) {
    model = ui.keyboardModel->itemData(cur_model, Qt::UserRole).toString();
    command += " -model ";
    command += model;
  }
  
  // set keyboard layout
  int n = ui.layouts->topLevelItemCount();
  QString layouts, variants;
  if(n > 0) {
    for(int row = 0; row < n; ++row) {
      QTreeWidgetItem* item = ui.layouts->topLevelItem(row);
      layouts += item->data(0, Qt::UserRole).toString();
      variants += item->data(1, Qt::UserRole).toString();
      if(row < n - 1) { // not the last row
        layouts += ',';
        variants += ',';
      }
    }
    command += " -layout ";
    command += layouts;
    command += " -variant ";
    command += variants;
  }

  Q_FOREACH(QString option, currentOptions_) {
    command += " -option ";
    command += option;
  }

  QString switchKey;
  int cur_switch_key = ui.switchKey->currentIndex();
  if(cur_switch_key > 0) { // index 0 is "None"
    switchKey = ui.switchKey->itemData(cur_switch_key, Qt::UserRole).toString();
    command += " -option ";
    command += switchKey;
  }

  qDebug() << command;

  // execute the command line
  setxkbmap.start(command);
  setxkbmap.waitForFinished();

  // save to lxqt-session config file.
  settings->beginGroup("Keyboard");
  settings->setValue("layout", layouts);
  settings->setValue("variant", variants);
  settings->setValue("model", model);
  if(switchKey.isEmpty() && currentOptions_ .isEmpty())
    settings->remove("options");
  else
    settings->setValue("options", switchKey.isEmpty() ? currentOptions_ : (currentOptions_ << switchKey));
  settings->endGroup();
}

void KeyboardLayoutConfig::onAddLayout() {
  SelectKeyboardLayoutDialog dlg(knownLayouts_, this);
  if(dlg.exec() == QDialog::Accepted) {
    addLayout(dlg.selectedLayout(), dlg.selectedVariant());
    accept();
  }
}

void KeyboardLayoutConfig::onRemoveLayout() {
  if(ui.layouts->topLevelItemCount() > 1) {
    QTreeWidgetItem* item = ui.layouts->currentItem();
    if(item) {
      delete item;
      accept();
    }
  }
}

void KeyboardLayoutConfig::onMoveDown() {
  QTreeWidgetItem* item = ui.layouts->currentItem();
  if(!item)
    return;
  int pos = ui.layouts->indexOfTopLevelItem(item);
  if(pos < ui.layouts->topLevelItemCount() - 1) { // not the last item
    ui.layouts->takeTopLevelItem(pos);
    ui.layouts->insertTopLevelItem(pos + 1, item);
    ui.layouts->setCurrentItem(item);
    accept();
  }
}

void KeyboardLayoutConfig::onMoveUp() {
  QTreeWidgetItem* item = ui.layouts->currentItem();
  if(!item)
    return;
  int pos = ui.layouts->indexOfTopLevelItem(item);
  if(pos > 0) { // not the first item
    ui.layouts->takeTopLevelItem(pos);
    ui.layouts->insertTopLevelItem(pos - 1, item);
    ui.layouts->setCurrentItem(item);
    accept();
  }
}

