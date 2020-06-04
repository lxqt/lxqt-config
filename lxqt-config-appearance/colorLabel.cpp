/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt.org/
 *
 * Copyright: 2020 LXQt team
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

#include "colorLabel.h"
#include <QColorDialog>

ColorLabel::ColorLabel(QWidget* parent, Qt::WindowFlags f)
    : QLabel(parent, f)
{
    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    setFixedWidth(100);
    setToolTip(tr("Click to change color."));
}

ColorLabel::~ColorLabel() {}

void ColorLabel::setColor(const QColor& color)
{
    if (!color.isValid())
        return;
    stylesheetColor_ = color;
    // ignore translucency
    stylesheetColor_.setAlpha(255);
    QString borderColor = qGray(stylesheetColor_.rgb()) < 255 / 2
                            ? QStringLiteral("white") : QStringLiteral("black");
    setStyleSheet(QStringLiteral("QLabel{background-color: rgb(%1, %2, %3); border: 1px solid %4;}")
                  .arg(color.red()).arg(color.green()).arg(color.blue()).arg(borderColor));
}

QColor ColorLabel::getColor() const
{
    if (stylesheetColor_.isValid())
        return stylesheetColor_; // the window color may be different from the stylesheet color
    return palette().color(QPalette::Window);
}

void ColorLabel::mousePressEvent(QMouseEvent* /*event*/) {
    QColor prevColor = getColor();
    QColor color = QColorDialog::getColor(prevColor, window(), tr("Select Color"));
    if (color.isValid() && color != prevColor)
    {
        emit colorChanged();
        setColor(color);
    }
}
