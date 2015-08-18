/*
* Copyright (c) LXQt contributors.
*
* This file is part of the LXQt project. <http://lxqt.org>
* It is distributed under the LGPL 2.1 or later license.
* Please refer to the LICENSE file for a copy of the license, and
* the AUTHORS file for copyright and authorship information.
*/

#include "monitorpicture.h"

#include <QFont>
#include <QFontMetrics>
#include <QPen>
#include <QDebug>
#include <QVector2D>
#include "configure.h"


MonitorPictureDialog::MonitorPictureDialog(QWidget * parent, Qt::WindowFlags f): QDialog(parent,f) {
  ui.setupUi(this);
}


void MonitorPictureDialog::setScene(QList<MonitorWidget*> monitors) {
  int monitorsWidth = 100.0;
  int monitorsHeight = 100.0;
  QGraphicsScene *scene = new QGraphicsScene();
  Q_FOREACH(MonitorWidget * monitor, monitors) {
    MonitorPicture *monitorPicture = new MonitorPicture(NULL, monitor, this);
    pictures.append(monitorPicture);
    scene->addItem(monitorPicture);
    monitorsWidth+=monitorPicture->rect().width();
    monitorsHeight+=monitorPicture->rect().height();
  }
  ui.graphicsView->scale(200.0/(float)monitorsWidth,200.0/(float)monitorsHeight);
  ui.graphicsView->setScene(scene);
}

void MonitorPictureDialog::updateScene() {
  ui.graphicsView->scene()->update();
}

void MonitorPictureDialog::updateMonitorWidgets(QString primaryMonitor) {
  int x0, y0;
  x0 = y0 =0;
  Q_FOREACH(MonitorPicture * picture, pictures) {
    if( picture->monitorWidget->monitorInfo->name == primaryMonitor || primaryMonitor=="") {
      x0 = picture->monitorWidget->ui.xPosSpinBox->value() + picture->pos().x();
      y0 = picture->monitorWidget->ui.yPosSpinBox->value() + picture->pos().y();
      break;
    }
  }
  Q_FOREACH(MonitorPicture * picture, pictures) {
    int x = -x0 + picture->monitorWidget->ui.xPosSpinBox->value();
    int y = -y0 + picture->monitorWidget->ui.yPosSpinBox->value();
    picture->monitorWidget->ui.xPosSpinBox->setValue(x + picture->pos().x());
    picture->monitorWidget->ui.yPosSpinBox->setValue(y + picture->pos().y());
  }
}



MonitorPicture::MonitorPicture(QGraphicsItem * parent, MonitorWidget *monitorWidget, MonitorPictureDialog *monitorPictureDialog):QGraphicsRectItem(parent)
{
  this->monitorWidget = monitorWidget;
  this->monitorPictureDialog = monitorPictureDialog;
  //QString modeName = monitorWidget->ui.resolutionCombo->currentText();
  //int  currentSizeWidth = monitorWidget->monitorInfo->monitorModes[modeName]->width;
  //int  currentSizeHeight = monitorWidget->monitorInfo->monitorModes[modeName]->height;
  MonitorMode *monitorModeInfo = monitorWidget->ui.resolutionCombo->currentData().value<MonitorMode*>();
  int  currentSizeWidth = monitorModeInfo->width;
  int  currentSizeHeight = monitorModeInfo->height;
  int x = monitorWidget->ui.xPosSpinBox->value();
  int y = monitorWidget->ui.yPosSpinBox->value();
  setAcceptedMouseButtons(Qt::LeftButton);
  setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges);
  setRect(x, y, currentSizeWidth, currentSizeHeight);
  originX = x;
  originY = y;


  QSvgRenderer *renderer = new QSvgRenderer(QLatin1String(ICON_PATH "monitor.svg"));
  svgItem = new QGraphicsSvgItem();
  svgItem->setSharedRenderer(renderer);
  svgItem->setX(x);
  svgItem->setY(y);
  svgItem->setOpacity(0.7);
  svgItem->setParentItem(this);


  textItem = new QGraphicsTextItem(monitorWidget->monitorInfo->name, this);
  textItem->setDefaultTextColor(Qt::white);
  textItem->setX(x);
  textItem->setY(y);
  textItem->setParentItem(this);
  setPen(QPen(Qt::black, 20));

  adjustNameSize();
}


void MonitorPicture::adjustNameSize() {
  prepareGeometryChange();
  qreal fontWidth = QFontMetrics(textItem->font()).width(monitorWidget->monitorInfo->name+"  ");
  textItem->setScale((qreal)this->rect().width()/fontWidth);
  QTransform transform;
  qreal width = qAbs(this->rect().width()/svgItem->boundingRect().width());
  qreal height = qAbs(this->rect().height()/svgItem->boundingRect().height());
  transform.scale(width, height);
  svgItem->setTransform(transform);
}


QVariant MonitorPicture::itemChange(GraphicsItemChange change, const QVariant & value)
{
  //qDebug() << "[MonitorPicture::itemChange]: ";
  //if ( change == ItemPositionChange && scene()) {
        // value is the new position.
        //QPointF newPos = value.toPointF();
        //qDebug() << "[MonitorPictureDialog::updateMonitorWidgets]: " << newPos.x() << "x" << newPos.y();
  //}
  return QGraphicsItem::itemChange(change, value);
}


void MonitorPicture::setMonitorPosition(int x, int y)
{
  setPos(x,y);
}


void MonitorPicture::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
  QGraphicsRectItem::mouseReleaseEvent(event);
  monitorPictureDialog->moveMonitorPictureToNearest(this);
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
  float det = v0.y()*v1.x() - v0.x()*v1.y();
  if( det == 0.0 ) {
    result.t1 = result.t2 = -1.0;
  }
  result.t1 = 1/det * ( -v1.y()*P.x() + v1.x()*P.y() );
  result.t2 = 1/det * ( -v0.y()*P.x() + v0.x()*P.y() );
  result.cutPoint = v0*result.t1 + p0;
  return result;
}

static QVector2D computeCenter(MonitorPicture* monitorPicture)
{
  float x0 = monitorPicture->x() + monitorPicture->originX;
  float y0 = monitorPicture->y() + monitorPicture->originY;
  float x1 = x0 + monitorPicture->rect().width();
  float y1 = y0 + monitorPicture->rect().height();
  QVector2D p0(x0,y0);
  QVector2D p1(x1,y1);
  QVector2D center = p0 + (p1-p0)*0.5;
  return center;
}

struct Result_moveMonitorPictureToNearest
{
  bool ok;
  QVector2D vector;
};

static Result_moveMonitorPictureToNearest compareTwoMonitors(MonitorPicture* monitorPicture1, MonitorPicture* monitorPicture2)
{
  Result_moveMonitorPictureToNearest result;
  QVector2D center1 = computeCenter(monitorPicture1);
  QVector2D center2 = computeCenter(monitorPicture2);
  float x0 = monitorPicture2->x()  + monitorPicture2->originX;
  float y0 = monitorPicture2->y()  + monitorPicture2->originY;
  float x1 = x0 + monitorPicture2->rect().width();
  float y1 = y0 + monitorPicture2->rect().height();
  QVector2D p0(x0,y0);
  QVector2D p1(x1,y1);
  QVector2D P1, P2;
  float t1=-1.0, t2=-1.0;
  Parameters params = segmentsCut(center1, center2, QVector2D(x0,y0), QVector2D(x1,y0));
  if(params.t1>=0.0 && params.t1<=1.0 && params.t2>=0.0 && params.t2<=1.0) {
    if(t1<0) {t1 = params.t1; P1 = params.cutPoint;}
  }
  params = segmentsCut(center1, center2, QVector2D(x0,y0), QVector2D(x0,y1));
  if(params.t1>=0.0 && params.t1<=1.0 && params.t2>=0.0 && params.t2<=1.0) {
    if(t1<0) {t1 = params.t1; P1 = params.cutPoint;}
  }
  params = segmentsCut(center1, center2, QVector2D(x1,y1), QVector2D(x1,y0));
  if(params.t1>=0.0 && params.t1<=1.0 && params.t2>=0.0 && params.t2<=1.0) {
    if(t1<0) {t1 = params.t1; P1 = params.cutPoint;}
  }
  params = segmentsCut(center1, center2, QVector2D(x1,y1), QVector2D(x0,y1));
  if(params.t1>=0.0 && params.t1<=1.0 && params.t2>=0.0 && params.t2<=1.0) {
    if(t1<0) {t1 = params.t1; P1 = params.cutPoint;}
  }
  x0 = monitorPicture1->x() + monitorPicture1->originX;
  y0 = monitorPicture1->y() + monitorPicture1->originY;
  x1 = x0 + monitorPicture1->rect().width();
  y1 = y0 + monitorPicture1->rect().height();
  p0 = QVector2D(x0,y0);
  p1 = QVector2D(x1,y1);
  params = segmentsCut(center1, center2, QVector2D(x0,y0), QVector2D(x1,y0));
  if(params.t1>=0.0 && params.t1<=1.0 && params.t2>=0.0 && params.t2<=1.0) {
    if(t2<0) {t2 = params.t1; P2 = params.cutPoint;}
  }
  params = segmentsCut(center1, center2, QVector2D(x0,y0), QVector2D(x0,y1));
  if(params.t1>=0.0 && params.t1<=1.0 && params.t2>=0.0 && params.t2<=1.0) {
    if(t2<0) {t2 = params.t1; P2 = params.cutPoint;}
  }
  params = segmentsCut(center1, center2, QVector2D(x1,y1), QVector2D(x1,y0));
  if(params.t1>=0.0 && params.t1<=1.0 && params.t2>=0.0 && params.t2<=1.0) {
    if(t2<0) {t2 = params.t1; P2 = params.cutPoint;}
  }
  params = segmentsCut(center1, center2, QVector2D(x1,y1), QVector2D(x0,y1));
  if(params.t1>=0.0 && params.t1<=1.0 && params.t2>=0.0 && params.t2<=1.0) {
    if(t2<0) {t2 = params.t1; P2 = params.cutPoint;}
  }

  if(t1>t2) { //Monitor outside
    result.vector = P1-P2;
    result.ok = false;
  } else {
    result.ok = true;
  }

  return result;
}

void MonitorPictureDialog::moveMonitorPictureToNearest(MonitorPicture* monitorPicture)
{
  if(!ui.magneticCheckBox->isChecked())
    return;
  QVector2D vector(0,0);
  foreach(MonitorPicture* picture, pictures) {
    if(picture==monitorPicture) continue;
    Result_moveMonitorPictureToNearest result = compareTwoMonitors(monitorPicture, picture);
    if(result.ok) {
      return;
    } else {
      if(result.vector.length()<vector.length() || vector.length()==0.0)
        vector = result.vector;
    }
  }
  int x = monitorPicture->x();
  int y = monitorPicture->y();
  monitorPicture->setX( x + vector.x() );
  monitorPicture->setY( y + vector.y() );
}
