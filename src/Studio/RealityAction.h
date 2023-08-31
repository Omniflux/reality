/**********************************************************************
  Copyright © 2010 Pret-A-3d
**********************************************************************/

#ifndef REALITY_ACTION_H
#define REALITY_ACTION_H

#include <QtGui/QKeySequence>
#include <dzaction.h>


namespace Reality {

/**
  This plugin adds a new action to start the Reality Script/Plugin.
**/

class Reality3Action : public DzAction {

  Q_OBJECT

public:

  /** Constructor **/
  Reality3Action();

  QString getActionGroup() const { return "Reality for DS"; }
  QKeySequence  getDefaultAccel() const { 
    return(QKeySequence(Qt::ALT + Qt::CTRL + Qt::Key_Y));
  }
  QString getDefaultMenuPath() const { return "&Render"; }

protected:
  /**
    We need to re-implement this virtual function to perform our 'action'.
    This gets called whenever the user clicks on our action item.
  **/
  void executeAction();

};

} // namespace

#endif
