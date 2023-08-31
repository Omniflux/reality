/**
  \file
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    

  This file implements the Python interface, via Boost, for Reality.
*/

#include <boost/python.hpp>

#include "ReLogger.h"
#include "RePoserInterface.h"
#include "ReSceneData.h"
#include "ReSceneDataGlobal.h"


namespace bpython = boost::python;

using namespace Reality;

// Converter taken from http://sire.googlecode.com/svn/python2/trunk/Qt/autoconvert_QString.cpp
/** This function convert a python string or unicode to a QString */
void QString_from_python_str_or_unicode( PyObject* obj_ptr,
                                         bpython::converter::rvalue_from_python_stage1_data* data )
{
    // First, convert the object to a python unicode object
    PyObject* unicode_object;

    if (PyUnicode_Check(obj_ptr))
    {
        unicode_object = obj_ptr;
        Py_INCREF(unicode_object);
    }
    else if (PyString_Check(obj_ptr))
    {
        // Coerce the `str' to `unicode' using UTF-8. UnicodeDecodeError
        // is thrown if the string doesn't make sense in that encoding.
        unicode_object = PyUnicode_FromEncodedObject(obj_ptr, "utf-8", "strict"); // new reference

        if (unicode_object == 0)
            boost::python::throw_error_already_set();
    }
    else
    {
        //we should never get here!!!
        return;
    }

    //now convert the Python unicode string to UTF-8
    PyObject *utf8 = PyUnicode_AsUTF8String( unicode_object );

    //we now don't need to the unicode string anymore
    Py_DECREF(unicode_object);

    if (utf8 == 0)
        boost::python::throw_error_already_set();

    //get the raw buffer from the string
    char *utf8_string = PyString_AS_STRING(utf8);

    if (utf8_string == 0)
    {
        Py_DECREF(utf8);
        boost::python::throw_error_already_set();
    }

    int utf8_nchars = PyString_GET_SIZE(utf8);

    //use Qt to convert the UTF8 string to QString unicode
    QString unicode_qstring;

    if (utf8_nchars > 0)
        unicode_qstring = QString::fromUtf8(utf8_string, utf8_nchars);

    Py_DECREF(utf8);

    void* storage = ((bpython::converter::rvalue_from_python_storage<QString>*) data)->storage.bytes;
    new (storage) QString(unicode_qstring);
    data->convertible = storage;
}

/** The actual struct used to control the conversion */
struct QString_from_python
{
    QString_from_python() {
      bpython::converter::registry::push_back( &convertible,
                                                &construct,
                                                bpython::type_id<QString>() );
    }

    /** Can the python object pointed to by 'obj_ptr' be converted
        to a QString?
    */
    static void* convertible(PyObject* obj_ptr) {
      if ( PyString_Check(obj_ptr) || PyUnicode_Check(obj_ptr) ) {
        return obj_ptr;
      }
      else {
        return 0;
      }
    }

    /** Perform the actual conversion */
    static void construct(  PyObject* obj_ptr,
                            bpython::converter::rvalue_from_python_stage1_data* data)
    {
      //use python-qt conversion function
      QString_from_python_str_or_unicode(obj_ptr, data);
    }
};

 
void initializeConverters()
{
  using namespace boost::python;
/* 
  // register the to-python converter
  to_python_converter<QString,QString_to_python_str>();
 
  // register the from-python converter
  QString_from_python_str();
*/
  QString_from_python();
}


BOOST_PYTHON_MODULE(Reality)
{
  using namespace boost::python;

  enum_<HostAppID>("HostAppID")
    .value("Poser", Poser)
    .value("DAZStudio", DAZStudio)
    .value("RealityPro", RealityPro)
    ;

  enum_<ReMaterialType>("ReMaterialType")
    .value("MatGlass", MatGlass)
    .value("MatGlossy", MatGlossy)
    .value("MatHair", MatHair)
    .value("MatMatte", MatMatte)
    .value("MatMetal", MatMetal)
    .value("MatMirror", MatMirror)
    .value("MatMix", MatMix)
    .value("MatNull", MatNull)
    .value("MatSkin", MatSkin)
    .value("MatVelvet", MatVelvet)
    .value("MatWater", MatWater)
    .value("MatLight", MatLight)
    .value("MatCloth", MatCloth)
    .value("MatUndefined", MatUndefined)
    ;
  /**
   * Interface to RealityBase
   */
  def("cameraAdded",             RePoserBase::cameraAdded);
  def("cameraDataChanged",       RePoserBase::cameraDataChanged);
  def("cameraRenamed",           RePoserBase::cameraRenamed);
  def("hostCameraSelected",      RePoserBase::hostCameraSelected);
  def("closeGUI",                RePoserBase::closeGUI);
  def("commandStackPop",         RePoserBase::commandStackPop);
  def("exportFinished",          RePoserBase::exportFinished);
  def("exportingObject",         RePoserBase::exportingObject);
  def("exportStarted",           RePoserBase::exportStarted);
  def("findHostAppID",           RePoserBase::findHostAppID);
  def("getLibraryVersion",       RePoserBase::getLibraryVersion);
  def("getNumCommands",          RePoserBase::getNumCommands);
  def("lightAdded",              RePoserBase::lightAdded);
  def("lightDeleted",            RePoserBase::lightDeleted);
  def("lightRenamed",            RePoserBase::lightRenamed);
  def("lightUpdated",            RePoserBase::lightUpdated);
  def("materialSelected",        RePoserBase::materialSelected);
  def("objectAdded",             RePoserBase::objectAdded);
  def("objectDeleted",           RePoserBase::objectDeleted);
  def("objectRenamed",           RePoserBase::objectRenamed);
  def("pauseMaterialPreview",    RePoserBase::pauseMaterialPreview);
  def("renderDimensionsNotSet",  RePoserBase::renderDimensionsNotSet);
  def("renderLuxQueue",          RePoserBase::renderLuxQueue);
  def("runGUI",                  RePoserBase::runGUI);
  def("runGuiLux",               RePoserBase::runGuiLux);
  def("setHostAppID",            RePoserBase::setHostAppID);
  def("setHostVersion",          RePoserBase::setHostVersion);
  def("setLibraryPaths",         RePoserBase::setLibraryPaths);
  def("setNewScene",             RePoserBase::setNewScene);
  def("startHostSideServices",   RePoserBase::startHostSideServices);
  def("stopHostSideServices",    RePoserBase::stopHostSideServices);
  def("writeToLog",              RePoserBase::writeToLog);
  def("getConfigValue",          RePoserBase::getConfigValue);
  def("startACSELCaching",       RePoserBase::startACSELCaching);
  def("stopACSELCaching",        RePoserBase::stopACSELCaching);
  def("sanitizeMaterialName",    RePoserBase::sanitizeMaterialName);
  def("updateAnimationLimits",   RePoserBase::updateAnimationLimits);

  /**
   * Interface to ReSceneData
   */
  class_<RePoserSceneData>("Scene")
    .def("addCamera",                 &RePoserSceneData::addCamera)
    .def("updateCamera",              &RePoserSceneData::updateCamera)
    .def("addMaterial",               &RePoserSceneData::addMaterial)
    .def("updateMaterial",            &RePoserSceneData::updateMaterial)
    .def("changeMaterialType",        &RePoserSceneData::changeMaterialType)
    .def("getSceneFileName",          &RePoserSceneData::getSceneFileName)
    .def("restoreScene",              &RePoserSceneData::restoreScene)
    .def("addLight",                  &RePoserSceneData::addLight)
    .def("updateLight",               &RePoserSceneData::updateLight)
    .def("setLightMatrix",            &RePoserSceneData::setLightMatrix)
    .def("setIsHostAreaLight",        &RePoserSceneData::setIsHostAreaLight)
    .def("renderScene",               &RePoserSceneData::renderScene)
    .def("exportScene",               &RePoserSceneData::exportScene)
    .def("getWidth",                  &RePoserSceneData::getWidth)
    .def("getHeight",                 &RePoserSceneData::getHeight)

    .def("addObject",                 &RePoserSceneData::addObject)
    .def("renameObject",              &RePoserSceneData::renameObject)
    .def("renameObjectID",            &RePoserSceneData::renameObjectID)
    .def("hasObject",                 &RePoserSceneData::hasObject)
    .def("deleteObject",              &RePoserSceneData::deleteObject)
    .def("getObjectName",             &RePoserSceneData::getObjectName)
    .def("getObjectIDs",              &RePoserSceneData::getObjectIDs)
    .def("getMaterialShortType",      &RePoserSceneData::getMaterialShortType)
    .def("findObjectByName",          &RePoserSceneData::findObjectByName)
    .def("markObjectAsLight",         &RePoserSceneData::markObjectAsLight)
    .def("markObjectsForDeletion",    &RePoserSceneData::markObjectsForDeletion)
    .def("clearObjectDeletionFlag",   &RePoserSceneData::clearObjectDeletionFlag)
    .def("deleteOrphanedObjects",     &RePoserSceneData::deleteOrphanedObjects)
    .def("setObjectVisible",          &RePoserSceneData::setObjectVisible)
    .def("getNumObjects",             &RePoserSceneData::getNumObjects)

    .def("getNumMaterials",           &RePoserSceneData::getNumMaterials)
    .def("restoreMaterial",           &RePoserSceneData::restoreMaterial)
    .def("getMaterialNames",          &RePoserSceneData::getMaterialNames)
    .def("newGeometryBuffer",         &RePoserSceneData::newGeometryBuffer)
    .def("copyVertexData",            &RePoserSceneData::copyVertexData)
    .def("copyUVData",                &RePoserSceneData::copyUVData)
    .def("copyPolygonData",           &RePoserSceneData::copyPolygonData)
    // .def("exportObjectBegin",         &RePoserSceneData::exportObjectBegin)
    // .def("exportObjectEnd",           &RePoserSceneData::exportObjectEnd)
    // .def("exportMaterial",            &RePoserSceneData::exportMaterial)
    .def("newScene",                  &RePoserSceneData::newScene)
    .def("renameCamera",              &RePoserSceneData::renameCamera)
    .def("removeCamera",              &RePoserSceneData::removeCamera)
    .def("selectCamera",              &RePoserSceneData::selectCamera)
    .def("renameLight",               &RePoserSceneData::renameLight)
    .def("deleteLight",               &RePoserSceneData::deleteLight)
    .def("setLightType",              &RePoserSceneData::setLightType)
    .def("restoreLight",              &RePoserSceneData::restoreLight)
    .def("setIBLRotation",            &RePoserSceneData::setIBLRotation)
    .def("setIBLImage",               &RePoserSceneData::setIBLImage)
    .def("setLightIntensity",         &RePoserSceneData::setLightIntensity)
    .def("getLightIDFromMaterial",    &RePoserSceneData::getLightIDFromMaterial)
    .def("restoreCamera",             &RePoserSceneData::restoreCamera)
    .def("removeAllLights",           &RePoserSceneData::removeAllLights)
    .def("backupLights",              &RePoserSceneData::backupLights)
    .def("restoreLightsFromBackup",   &RePoserSceneData::restoreLightsFromBackup)
    .def("setFrameSize",              &RePoserSceneData::setFrameSize)
    .def("getNumThreads",             &RePoserSceneData::getNumThreads)
    .def("needsSaving",               &RePoserSceneData::needsSaving)
    .def("isMaterialVisible",         &RePoserSceneData::isMaterialVisible)
    .def("renderSceneStart",          &RePoserSceneData::renderSceneStart)
    .def("renderSceneExportMaterial", &RePoserSceneData::renderSceneExportMaterial)
    .def("renderSceneFinish",         &RePoserSceneData::renderSceneFinish)
    .def("renderSceneObjectBegin",    &RePoserSceneData::renderSceneObjectBegin)
    .def("renderSceneObjectEnd",      &RePoserSceneData::renderSceneObjectEnd)
    .def("renderSceneCustomData",     &RePoserSceneData::renderSceneCustomData)
    .def("getSceneResourceObjectPath",&RePoserSceneData::getSceneResourceObjectPath)
    .def("getSceneResourceRelativePath",&RePoserSceneData::getSceneResourceRelativePath)
    .def("getRenderCameraPosition",   &RePoserSceneData::getRenderCameraPosition)

    ;
    
  initializeConverters();
  RE_LOG_INFO() << "Started Python extension";
}

/*
#include <fstream>
__attribute__((destructor))
static void finalizer() {
  ofstream of("/Users/paolo/debug.txt", std::ofstream::app);
  of << QDateTime::currentDateTime()
          .toString("yy-M-dd hh:mm:ss").toAscii().data()
     << "Reality Poser extension closing: " << __FILE__ << " -- " << __FUNCTION__ << endl;
  of.close();
}
*/