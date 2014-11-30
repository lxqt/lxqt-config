#include "monitorpicture.h"

#include <QFont>
#include <QFontMetrics>
#include <QPen>
#include <QDebug>


MonitorPictureDialog::MonitorPictureDialog(QWidget * parent, Qt::WindowFlags f): QDialog(parent,f) {
  ui.setupUi(this);
}


void MonitorPictureDialog::setScene(QList<MonitorWidget*> monitors) {
  int monitorsWidth = 100.0;
  int monitorsHeight = 100.0;
  QGraphicsScene *scene = new QGraphicsScene();
  Q_FOREACH(MonitorWidget * monitor, monitors) {
    MonitorPicture *monitorPicture = new MonitorPicture(NULL, monitor);
    pictures.append(monitorPicture);
    scene->addItem(monitorPicture);
    monitorsWidth+=monitorPicture->rect().width();
    monitorsHeight+=monitorPicture->rect().height();
  }
  ui.graphicsView->scale(200.0/(float)monitorsWidth,200.0/(float)monitorsHeight);
  ui.graphicsView->setScene(scene);
}

void MonitorPictureDialog::updateMonitorWidgets(QString primaryMonitor) {
  int x0, y0;
  x0 = y0 =0;
  Q_FOREACH(MonitorPicture * picture, pictures) {
    if( picture->monitorWidget->monitorInfo->name == primaryMonitor) {
      x0 = picture->monitorWidget->ui.xPosSpinBox->value() + picture->pos().x();
      y0 = picture->monitorWidget->ui.yPosSpinBox->value() + picture->pos().y();
    }
  }
  Q_FOREACH(MonitorPicture * picture, pictures) {
    int x = -x0 + picture->monitorWidget->ui.xPosSpinBox->value();
    int y = -y0 + picture->monitorWidget->ui.yPosSpinBox->value();
    picture->monitorWidget->ui.xPosSpinBox->setValue(x + picture->pos().x());
    picture->monitorWidget->ui.yPosSpinBox->setValue(y + picture->pos().y());
  }
}


MonitorPicture::MonitorPicture(QGraphicsItem * parent, MonitorWidget *monitorWidget):QGraphicsRectItem(parent)
{
  this->monitorWidget = monitorWidget;
  QSize currentSize = sizeFromString(monitorWidget->ui.resolutionCombo->currentText());
  int x = monitorWidget->ui.xPosSpinBox->value();
  int y = monitorWidget->ui.yPosSpinBox->value();
  setAcceptedMouseButtons(Qt::LeftButton);
  setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges);
  setRect(x, y, currentSize.width(), currentSize.height());
  setPen(QPen(Qt::black, 20));
  textItem = new QGraphicsTextItem(monitorWidget->monitorInfo->name, this);
  textItem->setX(x);
  textItem->setY(y);
  textItem->setParentItem(this);
  
  adjustNameSize();
}


void MonitorPicture::adjustNameSize() {
  qreal fontWidth = QFontMetrics(textItem->font()).width(monitorWidget->monitorInfo->name+"  "); 
  textItem->setScale((qreal)this->rect().width()/fontWidth);
}


QVariant MonitorPicture::itemChange(GraphicsItemChange change, const QVariant & value)
{
  //qDebug() << "[MonitorPicture::itemChange]: ";
  //if ( change == ItemPositionChange && scene()) {
        // value is the new position.
        //newPos = value.toPointF();
        //qDebug() << "[MonitorPictureDialog::updateMonitorWidgets]: " << newPos.x() << "x" << newPos.y();
  //}
  return QGraphicsItem::itemChange(change, value);
}


void MonitorPicture::setMonitorPosition(int x, int y)
{
  setPos(x,y);
}

