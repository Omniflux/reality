/**
 * Custom slider widget that has a customizable label and shows the
 * current value for the slider in another label
 */

#ifndef RESLIDER_H
#define RESLIDER_H

#include <QWidget>

#include "ui_ReSlider.h"


class ReSlider: public QWidget, public Ui::ReSlider {
  Q_OBJECT

  Q_PROPERTY(QString label READ getLabel WRITE setLabel)
  Q_PROPERTY(int value READ getValue WRITE setValue)
  Q_PROPERTY(int minimum READ getMinimum WRITE setMinimum)
  Q_PROPERTY(int maximum READ getMaximum WRITE setMaximum)
  Q_PROPERTY(int tickInterval READ getTickInterval WRITE setTickInterval)

public:
  /**
   * ctor
   */
  ReSlider( QWidget* parent = 0, const QString label = "Value" );

  //! Reimplemented
  QSize sizeHint() const;
  
protected:
  int currentValue;

protected slots:
  void updateValue( int newVal );

public slots:
  inline void setLabel( const QString newLabel ) {
    label->setText(newLabel);
  }

  inline QString getLabel() const {
    return label->text();
  }

  inline int getValue() const {
    return currentValue;
  }

  inline void setValue( const int newValue ) {
    slider->setValue(newValue);
  }

  inline void setMinimum( int newVal ) {
    slider->setMinimum(newVal);
  }
  inline int getMinimum() {
    return slider->minimum();
  }

  inline void setMaximum( int newVal ) {
    slider->setMaximum(newVal);
  }
  inline int getMaximum() {
    return slider->maximum();
  }

  inline void setTickInterval( int newVal ) {
    slider->setTickInterval(newVal);
  }
  inline int getTickInterval() {
    return slider->tickInterval();
  }


signals:

  //! Emitted when the user selects a new color
  void valueChanged( int newValue );
};

#endif
