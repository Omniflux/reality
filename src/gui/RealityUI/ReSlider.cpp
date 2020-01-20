
#include "ReSlider.h"
#include <QColorDialog>
#include <QPainter>

ReSlider::ReSlider( QWidget* parent, const QString newLabel ) : QWidget(parent) {
  setupUi(this);
  setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
  currentValue = 0;
  label->setText(newLabel);
  valueText->setText(QString::number(currentValue));

  connect(slider, SIGNAL(valueChanged(int)), this, SLOT(updateValue(int)));
}

QSize ReSlider::sizeHint() const {
  return QSize(120, 48); 
}

void ReSlider::updateValue( int newVal ) {
  currentValue = newVal;
  valueText->setText(QString::number(currentValue));
  emit valueChanged(currentValue);
}
