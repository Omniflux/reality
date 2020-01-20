/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef RE_ACTION_H
#define RE_ACTION_H

#include <QAction.h>

/**
 * Base class for all Reality actions. Actions inherit from QAction but 
 * extend the concept by becoming self-contained. Instead of being used
 * to trigger a slot in another class, the ReAction class is completely
 * self contained, executing all the necessary steps from inside the
 * execute() method
 */
class ReAction : public QAction {
  
  Q_OBJECT

public:
  //! Constructor
  ReAction() : QAction(0) {
    connect(this, SIGNAL(triggered()), this, SLOT(execute()));
  };

public slots:
  virtual void execute() = 0;  
};

#endif
