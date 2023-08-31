/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#include "RealityUI/ReTextureSelector.h"

#include <QTreeWidget>

#include "ReTexture.h"
#include "ReTextureContainer.h"

#define TL_COL_NAME      0
#define TL_COL_TYPE      1
#define TL_COL_DATA_TYPE 2
#define TL_COL_CHANNEL   3

using namespace Reality;

ReTextureSelector::ReTextureSelector( QWidget* parent ) : QDialog(parent) {
  setupUi(this);

  // Get notified when a selection is confirmed via the "Select" button
  connect(btnOK, SIGNAL(clicked()), this, SLOT(selectTexture()));

  // Get notified when an item is selected. 
  connect(textureList, SIGNAL(itemSelectionChanged()), this, SLOT(selectionChanged()));

  channelDescs["Kd"] = tr("Diffuse");
  channelDescs["Ka"] = tr("Glossy coat");
  channelDescs["Ks"] = tr("Specular");
  channelDescs["Kt"] = tr("Translucency");
  channelDescs["Bm"] = tr("Bump");
  channelDescs["Dm"] = tr("Displacement");
  channelDescs["color"] = tr("Color");
  channelDescs["float"] = tr("Numeric");
  channelDescs["alpha"] = tr("Alpha");
  btnOK->setEnabled(false);
};

void ReTextureSelector::setTextureList( const ReNodeDictionary& textures, 
                                        const QString& skipTexture ) {
  textureList->clear();
  selection.clear();

  ReNodeDictionaryIterator i(textures);

  int numItems = 0;
  while( i.hasNext() ) {
    i.next();
    QString texName = i.key();
    if (texName == skipTexture) {
      continue;
    }
    numItems++;
    ReTexturePtr tex = i.value();
    QTreeWidgetItem* item = new QTreeWidgetItem(textureList);
    item->setData(TL_COL_NAME, Qt::UserRole, texName);
    item->setText(TL_COL_NAME, texName);
    item->setText(TL_COL_TYPE, tex->getTypeAsString());
    item->setText(TL_COL_DATA_TYPE, channelDescs.value(tex->getDataTypeAsString()));
    QString channelName = tex->getParent()->identifyChannel(tex);
    item->setText(TL_COL_CHANNEL,
      channelDescs.value(channelName, channelName)
    );
  }
}

void ReTextureSelector::selectTexture() {
  QTreeWidgetItem* item = textureList->currentItem();
  selection = item->data(TL_COL_NAME, Qt::UserRole).toString();
  this->accept();
}

const QString ReTextureSelector::getSelection() const {
  return selection;
}

void ReTextureSelector::selectionChanged() {
  btnOK->setEnabled(textureList->selectedItems().count() > 0);
}

