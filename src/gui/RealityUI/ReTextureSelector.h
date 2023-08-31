/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */


#ifndef RE_TEXTURE_SELECTOR_H
#define RE_TEXTURE_SELECTOR_H

#include <QDialog>
#include <QSharedPointer>

#include "ui_reTextureSelector.h"

namespace Reality {
  class ReTexture;
  typedef QSharedPointer<ReTexture> ReTexturePtr;
  typedef QHash<QString, ReTexturePtr> ReNodeDictionary;
}


using namespace Reality;

/**
 * Class ReTextureSelector implements a simple dialog box that lists the
 * textures available for a given material. This list can be used by the 
 * user to select a texture to link to a channel. For example, the user
 * can select to re-use the diffuse texture as the basis for the hair
 * mask in the Skin material.
 */
class ReTextureSelector : public QDialog, public Ui::textureSelector {

Q_OBJECT
private:
  QString selection;
  //! Conversion table that provides human-readable names for the 
  //! channels. For example "Kd" returns "Diffuse"
  QHash<QString, QString> channelDescs;

public:
  // Constructor: ReTextureSelector
  ReTextureSelector( QWidget* parent = 0 );

  // Destructor: ReTextureSelector
  ~ReTextureSelector() {
  };

  void setTextureList( const ReNodeDictionary& textureList, const QString& skipTexture );

  const QString getSelection() const;

private slots:
  void selectTexture();

  // Respond to the signal when the selection is changed
  void selectionChanged();
};



#endif
