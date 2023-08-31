/*
 *  RePresetSelector.h
 *  ReGeomExport
 *
 *  Created by Paolo H Ciccone on 8/5/11.
 *  Copyright 2011 Pret-A-3D. All rights reserved.
 *
 */

#ifndef __REPRESETSELECTOR_H__
#define __REPRESETSELECTOR_H__
 
#include <QDialog>
#include <QStandardItemModel>

#include "ui_rePresetSelector.h" 


/**
 *
 */
class RePresetSelector : public QDialog, public Ui::rePresetSelector {
  Q_OBJECT

private:
  QStandardItemModel model;
  QString selectedValue;
  QString selectedText;
  QString selectedDescription;

private slots:  
  void showDescription(const QModelIndex& index);
  
public:

  RePresetSelector( QWidget* parent = 0 );

  ~RePresetSelector() {
  };

public slots:
  
  /**
   * This method sets the source of data for the preset selector.
   * The input sytring is expected to be a document in YAML format with the following structure
   * - <MenuKey>:
   *    menuText: <Visible Text>
   *    options:
   *      - value: <value to return>
   *        name: <first submenu>
   *        desc: <description>
   *        image: <image>
   *
   * Multiple entries for options are expected.
   */
  void setData( const QString data );

  /**
   * Returns the value selected in case the user pressed the "OK" button in the dialog box
   */
  QString getValue();
  QString getText();
  QString getDescription();
};

#endif
