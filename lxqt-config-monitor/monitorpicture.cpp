/*
 *    Copyright (C) 2014  P.L. Lucas <selairi@gmail.com>
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License along
 *    with this program; if not, write to the Free Software Foundation, Inc.,
 *    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "monitorpicture.h"

#include <QFont>
#include <QFontMetrics>
#include <QPen>
#include <QDebug>
#include <QVector2D>
#include <QScrollBar>

#include "configure.h"

// Gets size from string rate. String rate format is "widthxheight". Example: 800x600
static QSize sizeFromString(QString str)
{
    int width = 0;
    int height = 0;
    int x = str.indexOf(QLatin1Char('x'));
    if (x > 0)
    {
        width = str.leftRef(x).toInt();
        height = str.midRef(x + 1).toInt();
    }
    return QSize(width, height);
}

MonitorPictureProxy::MonitorPictureProxy(QObject *parent, MonitorPicture *monitorPicture):QObject(parent)
{
    this->monitorPicture = monitorPicture;
}

void MonitorPictureProxy::updateSize()
{
    KScreen::OutputPtr output = monitorPicture->monitorWidget->output;
    QSize size = output->currentMode()->size();
    monitorPicture->updateSize(size);
}

void MonitorPictureProxy::updatePosition()
{
    KScreen::OutputPtr output = monitorPicture->monitorWidget->output;
    QPoint pos = output->pos();
    //qDebug() << "MonitorPictureProxy:updatePosition]" << pos;
    monitorPicture->setMonitorPosition(pos.x(), pos.y());
}

MonitorPictureDialog::MonitorPictureDialog(KScreen::ConfigPtr config, QWidget * parent, Qt::WindowFlags f) :
    QDialog(parent,f)
{
    updatingOk = false;
    firstShownOk = false;
    maxMonitorSize = 0;
    mConfig = config;
    ui.setupUi(this);
}


void MonitorPictureDialog::setScene(QList<MonitorWidget *> monitors)
{
    int monitorsWidth =0;
    int monitorsHeight = 0;
    QGraphicsScene *scene = new QGraphicsScene();
    for (MonitorWidget *monitor : monitors)
    {
        MonitorPicture *monitorPicture = new MonitorPicture(nullptr, monitor, this);
        pictures.append(monitorPicture);
        scene->addItem(monitorPicture);
        monitorsWidth += monitorPicture->rect().width();
        monitorsHeight += monitorPicture->rect().height();
        MonitorPictureProxy *proxy = new MonitorPictureProxy(this, monitorPicture);
        proxy->connect(monitor->output.data(), SIGNAL(currentModeIdChanged()), SLOT(updateSize()));
        proxy->connect(monitor->output.data(), SIGNAL(posChanged()), SLOT(updatePosition()));
    }
    // The blue rectangle is maximum size of virtual screen (framebuffer)
    scene->addRect(0, 0, mConfig->screen()->maxSize().width(), mConfig->screen()->maxSize().height(), QPen(Qt::blue, 20))->setOpacity(0.5);
    maxMonitorSize = qMax(monitorsWidth, monitorsHeight);
    ui.graphicsView->setScene(scene);
}

void MonitorPictureDialog::showEvent(QShowEvent * event)
{
    QWidget::showEvent(event);
    if( ! firstShownOk )
    {
        // Update scale and set scrollbar position.
        // Real widget size is not set, until widget is shown.
        firstShownOk = true;
        int minWidgetLength = qMin(ui.graphicsView->size().width(), ui.graphicsView->size().width()) / 1.5;
        qDebug() << "minWidgetLength" << minWidgetLength << "maxMonitorSize" << maxMonitorSize << "scale" << minWidgetLength / (float) maxMonitorSize;
        ui.graphicsView->scale(minWidgetLength / (float) maxMonitorSize, minWidgetLength / (float) maxMonitorSize);
        updateScene();
        ui.graphicsView->verticalScrollBar()->setValue(0);
        ui.graphicsView->horizontalScrollBar()->setValue(0);
    }
}

void MonitorPictureDialog::updateScene()
{
    ui.graphicsView->scene()->update();
}

void MonitorPictureDialog::updateMonitorWidgets(QString primaryMonitor)
{
    // This method update spin boxes of position.
    // If position is changed when this method is running, position is changed until buffer overflow.
    // updatingOk control that this method can not be run twice in the same position change.

    if(updatingOk)
        return;
    updatingOk = true;
    int x0, y0;
    x0 = y0 = 0;

    for (MonitorPicture *picture : qAsConst(pictures))
    {
        if (picture->monitorWidget->output->name() == primaryMonitor
            || primaryMonitor == QStringLiteral(""))
        {
            x0 = picture->originX + picture->pos().x();
            y0 = picture->originY + picture->pos().y();
            break;
        }
    }

    if( primaryMonitor == QStringLiteral("") )
    {
        for(MonitorPicture *picture : qAsConst(pictures))
        {
            int x1 = picture->originX + picture->pos().x();
            int y1 = picture->originY + picture->pos().y();
            x0 = qMin(x0, x1);
            y0 = qMin(y0, y1);
        }
    }

    for (MonitorPicture *picture : qAsConst(pictures))
    {
        int x = picture->originX + picture->pos().x() - x0;
        int y = picture->originY + picture->pos().y() - y0;
        if( x != picture->monitorWidget->ui.xPosSpinBox->value() )
            picture->monitorWidget->ui.xPosSpinBox->setValue(x);
        //else
        //    qDebug() << "x Iguales";
        if( y != picture->monitorWidget->ui.yPosSpinBox->value() )
            picture->monitorWidget->ui.yPosSpinBox->setValue(y);
        //else
        //    qDebug() << "y Iguales";
        //qDebug() << "[MonitorPictureDialog::updateMonitorWidgets]" << x << '=' <<  picture->monitorWidget->ui.xPosSpinBox->value() << ',' << y << '=' << picture->monitorWidget->ui.yPosSpinBox->value();
    }
    updatingOk = false;
}

MonitorPicture::MonitorPicture(QGraphicsItem * parent,
                               MonitorWidget *monitorWidget,
                               MonitorPictureDialog *monitorPictureDialog) :
    QGraphicsRectItem(parent)
{
    this->monitorWidget = monitorWidget;
    this->monitorPictureDialog = monitorPictureDialog;
    QSize currentSize = sizeFromString(monitorWidget->ui.resolutionCombo->currentText());
    if( monitorWidget->output->rotation() == KScreen::Output::Left || monitorWidget->output->rotation() == KScreen::Output::Right )
        currentSize.transpose();
    int x = monitorWidget->ui.xPosSpinBox->value();
    int y = monitorWidget->ui.yPosSpinBox->value();
    setAcceptedMouseButtons(Qt::LeftButton);
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges);
    originX = x;
    originY = y;
    
    setRect(x, y, currentSize.width(), currentSize.height());
    // setPen(QPen(Qt::black, 20));
    // textItem = new QGraphicsTextItem(monitorWidget->output->name(), this);
    // textItem->setX(x);
    // textItem->setY(y);
    // textItem->setParentItem(this);
    
    QSvgRenderer *renderer = new QSvgRenderer(QLatin1String(ICON_PATH "monitor.svg"));
    svgItem = new QGraphicsSvgItem();
    svgItem->setSharedRenderer(renderer);
    svgItem->setX(x);
    svgItem->setY(y);
    svgItem->setOpacity(0.7);
    svgItem->setParentItem(this);
  
  
    textItem = new QGraphicsTextItem(monitorWidget->output->name(), this);
    textItem->setDefaultTextColor(Qt::white);
    textItem->setX(x);
    textItem->setY(y);
    textItem->setParentItem(this);
    setPen(QPen(Qt::black, 20));
    

    adjustNameSize();
}

void MonitorPicture::adjustNameSize()
{
    prepareGeometryChange();
    qreal fontWidth = QFontMetrics(textItem->font()).width(monitorWidget->output->name() + QStringLiteral("  "));
    textItem->setScale((qreal) this->rect().width() / fontWidth);
    QTransform transform;
    qreal width = qAbs(this->rect().width()/svgItem->boundingRect().width());
    qreal height = qAbs(this->rect().height()/svgItem->boundingRect().height());
    qDebug() << "Width x Height" << width << "x" << height;
    transform.scale(width, height);
    svgItem->setTransform(transform);
}

void MonitorPicture::updateSize(QSize currentSize)
{
    QRectF r = rect();
    r.setSize(currentSize);
    setRect(r);
    adjustNameSize();
}

QVariant MonitorPicture::itemChange(GraphicsItemChange change, const QVariant & value)
{
    //qDebug() << "[MonitorPicture::itemChange]: ";
    //if ( change == ItemPositionChange && scene()) {
          // value is the new position.
          //QPointF newPos = value.toPointF();
          //qDebug() << "[MonitorPictureDialog::updateMonitorWidgets]: " << newPos.x() << "x" << newPos.y();
    //}
    QVariant v = QGraphicsItem::itemChange(change, value);
    //monitorPictureDialog->updateMonitorWidgets(QString());
    return v;
}

void MonitorPicture::setMonitorPosition(int x, int y)
{
    setX( x - originX );
    setY( y - originY );
}

void MonitorPicture::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    QGraphicsRectItem::mouseReleaseEvent(event);
    monitorPictureDialog->moveMonitorPictureToNearest(this);
    monitorPictureDialog->updateMonitorWidgets(QString());
}

//////////////////////////////////////////////////////////////////////////////////
// Move picture to nearest picture procedure.
// Read magnetic_attraction.html for more info about the algorithm used.
//////////////////////////////////////////////////////////////////////////////////

struct Parameters {
    float t1, t2;
    QVector2D cutPoint;
};

static Parameters segmentsCut(QVector2D p0, QVector2D p1, QVector2D s0, QVector2D s1)
{
    Parameters result;
    QVector2D v0 = p1 - p0;
    QVector2D v1 = s1 - s0;
    QVector2D P = s0 - p0;

    float det = v0.y() * v1.x() - v0.x() * v1.y();
    if (det == 0.0)
        result.t1 = result.t2 = -1.0;

    result.t1 = 1 / det * (-v1.y() * P.x() + v1.x() * P.y());
    result.t2 = 1 / det * (-v0.y() * P.x() + v0.x() * P.y());
    result.cutPoint = v0 * result.t1 + p0;
    return result;
}

static QVector2D computeCenter(MonitorPicture* monitorPicture)
{
    float x0 = monitorPicture->x() + monitorPicture->originX;
    float y0 = monitorPicture->y() + monitorPicture->originY;
    float x1 = x0 + monitorPicture->rect().width();
    float y1 = y0 + monitorPicture->rect().height();
    QVector2D p0(x0, y0);
    QVector2D p1(x1, y1);
    QVector2D center = p0 + (p1 - p0) * 0.5;
    return center;
}

struct Result_moveMonitorPictureToNearest
{
    bool ok;
    QVector2D vector;
};

static Result_moveMonitorPictureToNearest compareTwoMonitors(MonitorPicture* monitorPicture1,
                                                             MonitorPicture* monitorPicture2)
{
    Result_moveMonitorPictureToNearest result;
    QVector2D center1 = computeCenter(monitorPicture1);
    QVector2D center2 = computeCenter(monitorPicture2);
    float x0 = monitorPicture2->x() + monitorPicture2->originX;
    float y0 = monitorPicture2->y() + monitorPicture2->originY;
    float x1 = x0 + monitorPicture2->rect().width();
    float y1 = y0 + monitorPicture2->rect().height();

    QVector2D p0(x0, y0);
    QVector2D p1(x1, y1);
    QVector2D P1, P2;
    float t1 = -1.0, t2 = -1.0;

    Parameters params = segmentsCut(center1, center2, QVector2D(x0, y0), QVector2D(x1, y0));
    if (params.t1 >= 0.0 && params.t1 <= 1.0 && params.t2 >= 0.0 && params.t2 <= 1.0 && t1 < 0)
    {
        t1 = params.t1;
        P1 = params.cutPoint;
    }

    params = segmentsCut(center1, center2, QVector2D(x0, y0), QVector2D(x0, y1));
    if (params.t1 >= 0.0 && params.t1 <= 1.0 && params.t2 >= 0.0 && params.t2 <= 1.0 && t1 < 0)
    {
        t1 = params.t1;
        P1 = params.cutPoint;
    }

    params = segmentsCut(center1, center2, QVector2D(x1, y1), QVector2D(x1, y0));
    if (params.t1 >= 0.0 && params.t1 <= 1.0 && params.t2 >= 0.0 && params.t2 <= 1.0 && t1 < 0)
    {
        t1 = params.t1;
        P1 = params.cutPoint;
    }

    params = segmentsCut(center1, center2, QVector2D(x1, y1), QVector2D(x0, y1));
    if (params.t1 >= 0.0 && params.t1 <= 1.0 && params.t2 >= 0.0 && params.t2 <= 1.0 && t1 < 0)
    {
        t1 = params.t1;
        P1 = params.cutPoint;
    }

    x0 = monitorPicture1->x() + monitorPicture1->originX;
    y0 = monitorPicture1->y() + monitorPicture1->originY;
    x1 = x0 + monitorPicture1->rect().width();
    y1 = y0 + monitorPicture1->rect().height();
    p0 = QVector2D(x0, y0);
    p1 = QVector2D(x1, y1);

    params = segmentsCut(center1, center2, QVector2D(x0, y0), QVector2D(x1, y0));
    if (params.t1 >= 0.0 && params.t1 <= 1.0 && params.t2 >= 0.0 && params.t2 <= 1.0 && t2 < 0)
    {
        t2 = params.t1;
        P2 = params.cutPoint;
    }

    params = segmentsCut(center1, center2, QVector2D(x0, y0), QVector2D(x0, y1));
    if (params.t1 >= 0.0 && params.t1 <= 1.0 && params.t2 >= 0.0 && params.t2 <= 1.0 && t2 < 0)
    {
        t2 = params.t1;
        P2 = params.cutPoint;
    }

    params = segmentsCut(center1, center2, QVector2D(x1, y1), QVector2D(x1, y0));
    if (params.t1 >= 0.0 && params.t1 <= 1.0 && params.t2 >= 0.0 && params.t2 <= 1.0 && t2 < 0)
    {
        t2 = params.t1;
        P2 = params.cutPoint;
    }

    params = segmentsCut(center1, center2, QVector2D(x1, y1), QVector2D(x0, y1));
    if (params.t1 >= 0.0 && params.t1 <= 1.0 && params.t2 >= 0.0 && params.t2 <= 1.0 && t2 < 0)
    {
        t2 = params.t1;
        P2 = params.cutPoint;
    }

    // Monitor outside
    if (t1 > t2)
    {
        result.vector = P1 - P2;
        result.ok = false;
    } else
        result.ok = true;

    return result;
}

void MonitorPictureDialog::moveMonitorPictureToNearest(MonitorPicture* monitorPicture)
{
    if (!ui.magneticCheckBox->isChecked())
        return;

    QVector2D vector(0, 0);
    for (MonitorPicture *picture : qAsConst(pictures))
    {
        if (picture == monitorPicture)
            continue;

        Result_moveMonitorPictureToNearest result = compareTwoMonitors(monitorPicture, picture);
        if (result.ok)
            return;
        else if (result.vector.length() < vector.length() || vector.length() == 0.0)
            vector = result.vector;
    }

    int x = monitorPicture->x();
    int y = monitorPicture->y();
    monitorPicture->setX(x + vector.x());
    monitorPicture->setY(y + vector.y());
}
