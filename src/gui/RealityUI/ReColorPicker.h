/*
 * File: ReColorPicker.cpp
 * Custom color picker widget. It implements a simple rectangular widget
 * that calls the OS' color picker. Simpler and less intrusive than the
 * one provided by Studio and re-usable for other applications, like Poser
 */

#ifndef RECOLORPICKER_H
#define RECOLORPICKER_H


#include <QWidget>
#include "ui_reColorPicker.h"

class ReColorPicker: public QWidget, public Ui::colorPicker {
  Q_OBJECT

  Q_PROPERTY(bool showLabel READ isShowLabelEnabled WRITE setShowLabelEnabled)
public:
  /**
   * ctor
   */
  ReColorPicker( QWidget* parent = 0);   
  QSize sizeHint() const;
  
protected:
  QColor currentColor;
  bool showLabel;

  void mousePressEvent(QMouseEvent* event);
  void paintEvent(QPaintEvent* event);

public slots:
  void setColor( const QColor newColor );
  QColor getColor();

  bool isShowLabelEnabled();
  void setShowLabelEnabled( const bool newVal );

signals:

  // Emitted when the user selects a new color
  void colorChanged();
};

#endif
