/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef SHADER_CAT_PRESET_H
#define SHADER_CAT_PRESET_H

#include <QSharedPointer>

#include "ReAcsel.h"
#include "RePresetList.h"


namespace Reality {

//! A preset list designed to add the list of shader categories to a 
//! combobox
class ReShaderCategoryPreset : public RePresetList {

private:
  bool addPromptFlag;
public:
  //! The addPrompt parameter instructs the class whether or not add a
  //! "Select a category" prompt to the list of options. This is 
  //! something that is shown as the default item in the combobox
  //! and helps the user in the selection process.
  ReShaderCategoryPreset( const bool addPrompt ) : addPromptFlag(addPrompt) {
    initTable();
  }

  void initTable() {
    auto acsel = ReAcsel::getInstance();
    QStringList cats;
    if (addPromptFlag) {
      cats << QObject::tr("Select a category");
    }
    acsel->getCategoryList(cats);
    for (int i = 0; i < cats.count(); i++) {
      presetMap[i] = cats[i];
    }
  }
};

typedef QSharedPointer<ReShaderCategoryPreset> ReShaderCategoryPresetPtr;

} // namespace

#endif
