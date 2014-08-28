/*
 * <one line to give the library's name and an idea of what it does.>
 * Copyright (C) 2014  PCMan <email>
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

#include "timeoutdialog.h"
#include <QTimer>

TimeoutDialog::TimeoutDialog(QWidget* parent, Qt::WindowFlags f) {
  ui.setupUi(this);

  QIcon icon = style()->standardIcon(QStyle::SP_MessageBoxQuestion);
  int size = style()->pixelMetric(QStyle::PM_MessageBoxIconSize);
  ui.icon->setPixmap(icon.pixmap(QSize(size, size)));

  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
  adjustSize();
}

TimeoutDialog::~TimeoutDialog() {
}

void TimeoutDialog::showEvent(QShowEvent* e) {
  timer->start(1000);
  QDialog::showEvent(e);
}

void TimeoutDialog::onTimeout() {
  int time = ui.progressBar->value() + 1;
  if(time >= 10) { // If time is finished, settings are restored.
    timer->stop();
    reject();
  }
  else {
    ui.progressBar->setValue(time);
    ui.progressBar->setFormat(tr("%1 second(s) remaining").arg(10 - time));
  }
}
