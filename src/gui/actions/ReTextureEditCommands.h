/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_TEXTURE_EDIT_ACTIONS_H
#define RE_TEXTURE_EDIT_ACTIONS_H

#include <QUndoCommand>

#include "ReTextureChannelDataModel.h"


using namespace Reality;

/**
 * Base class for all the undoable actions that modify textures.
 */
template <typename T> class ReTextureEditCommand : public QUndoCommand {

private:
  ReTextureChannelDataModelPtr model;
  //! The key used in the communication with the data model
  QString valueName;

  T oldValue, newValue;

  //! Used to keep track if the redo() call is the first one, 
  //! the one that perform the action, or it is the one called
  //! from the Redo menu. During the very first call we don't
  //! need to invalidate the model. When called as a proper
  //! Redo command the we invalidate the model to force a
  //! refresh of the UI.
  quint8 iterations;

public:

  //! Constructor
  ReTextureEditCommand( ReTextureChannelDataModelPtr model,
                       const QString valueName,
                       const QString uiString,
                       T value ) :
    model(model),
    valueName(valueName),
    newValue(value)
  {
    setText(uiString);
    oldValue = model->getNamedValue(valueName).value<T>();
    iterations = 0;
  }

  // Destructor
 ~ReTextureEditCommand() {
  };

  void redo() {
    model->setNamedValue(valueName, newValue);
    if (!iterations) {
      iterations++;
    }
    else {
      model->reset();
    }
  };
  void undo(){
    model->setNamedValue(valueName, oldValue);
    model->reset();    
  };

};

/**
 * Specialization for the ImageMap Command
 */
template <typename T> class IME_Command : public ReTextureEditCommand<T> {

private:
  QString sectionName;

public:

  IME_Command( ReTextureChannelDataModelPtr model,
              const QString valueName,
              const QString uiString,
              T value ) : 
    ReTextureEditCommand<T>(model, valueName, uiString, value),
    sectionName("ImageMapEditor") 
  {

  }  

};


/**
 * Cloud Editor Command
 */
template <typename T> class CE_Command : public ReTextureEditCommand<T> {

private:
  QString sectionName;

public:

  CE_Command(ReTextureChannelDataModelPtr model,
            const QString valueName,
            const QString uiString,
            T value) : 
    ReTextureEditCommand<T>(model, valueName, uiString, value),
    sectionName("CloudEditor") 
  {

  }
};

/**
 * Band Editor Command
 */
template <typename T> class Band_Command : public ReTextureEditCommand<T> {

private:
  QString sectionName;

public:

  Band_Command( ReTextureChannelDataModelPtr model,
                const QString valueName,
                const QString uiString,
                T value ) : 
    ReTextureEditCommand<T>( model, valueName, uiString, value ),
    sectionName("BandEditor") 
  {

  }
};

/**
 * Marble Editor Command
 */
template <typename T> class MA_Command : public ReTextureEditCommand<T> {

private:
  QString sectionName;

public:

  MA_Command(ReTextureChannelDataModelPtr model,
             const QString valueName,
             const QString uiString,
             T value) : 
    ReTextureEditCommand<T>(model, valueName, uiString, value),
    sectionName("MarbleEditor") 
  {

  }  

};


/**
 * Brick Editor Command
 */
template <typename T> class BE_Command : public ReTextureEditCommand<T> {

private:
  QString sectionName;

public:

  BE_Command(ReTextureChannelDataModelPtr model,
             const QString valueName,
             const QString uiString,
             T value) : 
    ReTextureEditCommand<T>(model, valueName, uiString, value),
    sectionName("BrickEditor") 
  {
  
  }
};

/**
 * Wood Editor Command
 */
template <typename T> class WE_Command : public ReTextureEditCommand<T> {

private:
  QString sectionName;

public:

  WE_Command(ReTextureChannelDataModelPtr model,
            const QString valueName,
            const QString uiString,
            T value) : 
    ReTextureEditCommand<T>(model, valueName, uiString, value),
    sectionName("WoodEditor") 
  {
  
  }
};

/**
 * Distorted Nosie Editor Command
 */
template <typename T> class DN_Command : public ReTextureEditCommand<T> {

private:
  QString sectionName;

public:

  DN_Command(ReTextureChannelDataModelPtr model,
             const QString valueName,
             const QString uiString,
             T value) : 
    ReTextureEditCommand<T>(model, valueName, uiString, value),
    sectionName("DistortedNoiseEditor") 
  {
  
  }
};


/**
 * FBM Editor Command
 */
template <typename T> class FBME_Command : public ReTextureEditCommand<T> {

private:
  QString sectionName;

public:

  FBME_Command(ReTextureChannelDataModelPtr model,
               const QString valueName,
               const QString uiString,
               T value) : 
    ReTextureEditCommand<T>(model, valueName, uiString, value),
    sectionName("FBMEditor") 
  {
  
  }
};


/**
 * 3DMapping Command
 */
template <typename T> class M3D_Command : public ReTextureEditCommand<T> {

private:
  QString sectionName;

public:

  M3D_Command(ReTextureChannelDataModelPtr model,
            const QString valueName,
            const QString uiString,
            T value) : 
    ReTextureEditCommand<T>(model, valueName, uiString, value),
    sectionName("Mapping3D") 
  {

  }
};


/**
 * Color Editor Command
 */
template <typename T> class ColorEdit_Command : public ReTextureEditCommand<T> {

private:
  QString sectionName;

public:

  ColorEdit_Command(ReTextureChannelDataModelPtr model,
                   const QString valueName,
                   const QString uiString,
                   T value) : 
    ReTextureEditCommand<T>(model, valueName, uiString, value),
    sectionName("ColorEditor") 
  {

  }
};


#endif
