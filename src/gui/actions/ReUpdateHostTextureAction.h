/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef UPDATE_HOST_TEXTURE_ACTION_H
#define UPDATE_HOST_TEXTURE_ACTION_H

#include "ReAction.h"
#include "RealityBase.h"
#include "RealityDataRelay.h"

#include <QtCore>
#include <QFileInfo>

namespace Reality {


/**
 * Action to update a texture in the host side from the version that is 
 * held by the Reality UI.
 */
class ReUpdateHostTextureAction : public ReAction {
private:
  QString objID, matName, propName;
  ReTexturePtr tex;
  QVariant newVal;

public:
  //! Constructor: ReUpdateHostTextureAction
  //! \param objID The unique ID 
  ReUpdateHostTextureAction( const QString& objID, 
                             const QString& matName,
                             const QString& propName,
                             const ReTexturePtr tex,
                             QVariant _newVal = QVariant() ) :
    objID(objID),
    matName(matName),
    propName(propName),
    tex(tex)
  {
    newVal = _newVal.isValid() ? _newVal : tex->getNamedValue(propName);
  };

  inline void setValue( const QString& _propName, const QVariant& _value ) {
    propName = _propName;
    newVal   = _value;
  }

  inline void execute() {
    // Communicate to the host-side the change
    QVariantMap args;
    // Prepare the data to be sent to the server
    args["objectID"]     = objID;
    args["materialName"] = matName;
    args["textureName"]  = tex->getName();
    args["propertyName"] = propName;
    args["value"]        = newVal;
    realityDataRelay->sendMessageToServer(UI_MATERIAL_EDITED, &args);
  }

};

} // namespace

#endif
