/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef RePresetList_H
#define RePresetList_H

#include <QtCore>
#include <QMap>
#include <QComboBox>
#include <QListWidget>

#include "ReLogger.h"


namespace Reality {


/**
 * A class to store a list of values and their identifiers. This class is 
 * usually employed in feeding and using Combo-boxes.
 */
class RePresetList {

protected:

  //! A preset list. Simply a list of strings associated with a numeric
  //! identifier.
  typedef QMap<int, QString> RePresetMap;

  RePresetMap presetMap;

  QComboBox* cbb;
public:
  // Constructor: RePresetList
  RePresetList() {
    cbb = NULL;
  };
  // Destructor: RePresetList
  virtual ~RePresetList() {
  };

  virtual void initTable() = 0; 

  //! Returns the ID of a given value stored in the map. 
  //! If the value is not found -1 is returned
  int getID( const QString value ) {
    return presetMap.key(value, -1);
  }

  int getIndex( const int key ) {
    if (!cbb) {
      return -1;
    }
    return cbb->findText(presetMap.value(key));
  }
  
  QString getValue( const int ID ) {
    return presetMap.value(ID);
  }
  
  void initComboBox( QComboBox* _cbb ) {
    cbb = _cbb;
    cbb->clear();
    QMapIterator<int, QString> i(presetMap);
    while( i.hasNext() ) {
      i.next();
      cbb->addItem(i.value(), QVariant(i.key()));
    }
    cbb->setCurrentIndex(0);
  }

  void initWidget( QListWidget* lw ) {
    lw->clear();
    QMapIterator<int, QString> i(presetMap);
    while( i.hasNext() ) {
      i.next();
      QListWidgetItem* item = new QListWidgetItem();
      item->setText(i.value());
      item->setData(Qt::UserRole, i.key());
      lw->addItem(item);
    }
  }
};

} // namespace


#endif
