#include "monitorpicture.h"

#include <QFont>
#include <QFontMetrics>
#include <QPen>
#include <QDebug>

MonitorPicture::MonitorPicture(QGraphicsItem * parent, MonitorInfo *monitorInfo):QGraphicsRectItem(parent)
{
  this->monitorInfo = monitorInfo;
  setAcceptedMouseButtons(Qt::LeftButton);
  setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges);
  setRect(monitorInfo->xPos, monitorInfo->yPos,monitorInfo->currentSize().width(), monitorInfo->currentSize().height());
  setPen(QPen(Qt::black, 20));
  textItem = new QGraphicsTextItem(monitorInfo->name, this);
  textItem->setX(monitorInfo->xPos);
  textItem->setY(monitorInfo->yPos);
  textItem->setParentItem(this);
  
  adjustNameSize();
}

void MonitorPicture::adjustNameSize() {
  qreal fontWidth = QFontMetrics(textItem->font()).width(monitorInfo->name+"  "); 
  textItem->setScale((qreal)this->rect().width()/fontWidth);
}

QVariant MonitorPicture::itemChange(GraphicsItemChange change, const QVariant & value)
{
  qDebug() << "[MonitorPicture::itemChange]: ";
  if ( change == ItemPositionChange && scene()) {
        // value is the new position.
        QPointF newPos = value.toPointF();
        monitorInfo->setPos(newPos.x(), newPos.y());
        qDebug() << "[MonitorPicture::itemChange]: " << newPos.x() << "," << newPos.y(); 
    }
    return QGraphicsItem::itemChange(change, value);
}

void MonitorPicture::setMonitorPosition(int x, int y)
{
  setPos(x,y);
}


MonitorPictureQObject::MonitorPictureQObject(MonitorPicture *monitorPicture, QObject *parent): QObject(parent) {
  this->monitorPicture = monitorPicture;
}

void MonitorPictureQObject::setXMonitorPosition(int x) {
  monitorPicture->setX(x);
}

void MonitorPictureQObject::setYMonitorPosition(int y) {
  monitorPicture->setY(y);
}

void MonitorPictureQObject::setSize(QSize size) {
  QRectF _rect = monitorPicture->rect();
  _rect.setWidth(size.width());
  _rect.setHeight(size.height());
  monitorPicture->setRect(_rect);
  qDebug() << "[MonitorPictureQObject::setSize]" << _rect.width() << _rect.height();
  monitorPicture->adjustNameSize();
}

void MonitorPictureQObject::setSize(const QString rate) {
  qDebug() << "[MonitorPictureQObject::setSize] Rate: " << rate;
  setSize(sizeFromString(rate));
}