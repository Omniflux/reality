
#include "ReColorPicker.h"
#include <QColorDialog>
#include <QPainter>

ReColorPicker::ReColorPicker( QWidget* parent ) : QWidget(parent) {
  setupUi(this);
  setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
  currentColor = Qt::white;
  showLabel = true;
  setStyleSheet(QString("background-color: %1;").arg(currentColor.name()));
}


void ReColorPicker::setColor(const QColor newColor) {
  currentColor = newColor;
  update();
}

QColor ReColorPicker::getColor() {
  return currentColor;
}

QSize ReColorPicker::sizeHint() const {
  return QSize(120,24); 
}

void ReColorPicker::mousePressEvent(QMouseEvent* event) {
  QColor tmpColor = QColorDialog::getColor(
                      currentColor, 
                      QApplication::activeWindow(), 
                      tr("Select the color for the material")
                    );
  if (tmpColor.isValid()) {
    currentColor = tmpColor;
    emit colorChanged();
  }
  update();
}

bool ReColorPicker::isShowLabelEnabled() {
  return showLabel;
}
void ReColorPicker::setShowLabelEnabled( const bool newVal ) {
  showLabel = newVal;
  update();
}

void ReColorPicker::paintEvent(QPaintEvent* event) {
  label->setStyleSheet(QString("background-color: %1;").arg(currentColor.name()));
  if (!showLabel) {
    label->setText("");    
  }
  else {
    QColor bgColor = currentColor.toHsv();
    QString txtColor; 
    if (bgColor.value() < 130) {
      txtColor = "  #FFF";
    }
    else {
      txtColor = "#000";
    }
    label->setStyleSheet(QString("background-color: %1; color: %2;")
                         .arg(bgColor.name())
                         .arg(txtColor)
                        );    
    label->setText(QString("%1:%2:%3")
                   .arg(currentColor.red(),3,10,QLatin1Char('0'))
                   .arg(currentColor.green(),3,10,QLatin1Char('0'))
                   .arg(currentColor.blue(),3,10,QLatin1Char('0'))
                  );
  }
  QWidget::paintEvent(event);
}

