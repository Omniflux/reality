/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef RE_UI_CONTAINER_H
#define RE_UI_CONTAINER_H

#include <QAction>

namespace Reality {

/**
 * Interface for the RealityPanel. This interface defines a container 
 * that holds an instance of the RealityPanel class. Through this interface
 * the RealityPanel class can interact and perform operations like installing
 * actins in the applicatio's menu 
 */

class ReUiContainer {

public:
  //! Constructor: ReUiContainer
  ReUiContainer() {
  };

  //! Destructor: ReUiContainer
 ~ReUiContainer() {
  };

  //! Adds a predefined action designed to convert a material to a different
  //! type to the menu of the container.
  virtual void addConvertActionToMenu( QAction* action ) = 0;

  //! Set the title for the container. 
  virtual void setTitle( const QString& title ) = 0;

  //! Show a message, typically in the status bar
  virtual void showMessage( const QString& msg ) = 0;
};


} // namespace

#endif
