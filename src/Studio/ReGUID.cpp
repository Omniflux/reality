/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
*/

#include "ReGUID.h"

#include <QUuid>
#include <dzcustomdata.h>
#include <dznode.h>
#include <dzsettings.h>
#include <dzstringproperty.h>

#include "ReLogger.h"


namespace Reality {

// Initialization of static var
QString ReGUID::GUID_value;

const char* ReGUID::InvalidGUID = "NO_GUID";

/**
 * Returns the GUID of a node, if present. A GUID is stored in a node 
 * using a DzSimpleElementData instance. Each object handled by Reality
 * has that additional piece of data. If the GUID is not found an empty
 * value is returned
 */
QString& ReGUID::getGUID( const DzNode* node ) {
  GUID_value = "";
  if (!node) {
    RE_LOG_INFO() << "Passed null pointer to getGUID";
    GUID_value = InvalidGUID;
    return GUID_value;
  }
  DzSimpleElementData* dataItem = qobject_cast<DzSimpleElementData*>(
    node->findDataItem(RE_DS_NODE_GUID)
  );
  // There is no previously attached data
  if (!dataItem) {
    // In addition to the custom data we replicate the ID as a private
    // property because Studio strips the custom data when adapting clothing 
    // from one figure to another. So we test if the private property is 
    // available before giving up.
    auto customProp = node->findPrivateProperty(RE_DS_NODE_GUID, true);
    if (customProp) {
      GUID_value = static_cast<DzStringProperty*>(customProp)->getValue();
    }
    if (GUID_value.isEmpty()) {
      GUID_value = InvalidGUID;
    }
    return GUID_value;
  } 
  auto settings = dataItem->getSettings();
  return( GUID_value = settings->getStringValue(RE_DS_NODE_GUID,""));
};

//! Returns a GUID string ready to be used to tag a node 
QString& ReGUID::getGUID() {
  GUID_value = QUuid::createUuid().toString();
  // Remove the first and last bracket
  GUID_value = GUID_value.remove(0,1);
  GUID_value.chop(1);
  return GUID_value;
}

/**
 * Assigns a GUID to a node. To make the assignment permanent the method
 * adds an instance of DzSimpleElementData to the node and uses that 
 * instance to store the GUID
 */ 
QString& ReGUID::setGUID( DzNode* node ) {
  DzSimpleElementData* dataItem = qobject_cast<DzSimpleElementData*>(
    node->findDataItem(RE_DS_NODE_GUID)
  );
  // There is no previously attached data
  if (!dataItem) {
    dataItem = new DzSimpleElementData(RE_DS_NODE_GUID, true);
    node->addDataItem(dataItem);
  } 
  auto settings = dataItem->getSettings();
  GUID_value = getGUID();
  settings->setStringValue(RE_DS_NODE_GUID, GUID_value);
  return GUID_value;
};

} // namespace
