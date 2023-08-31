/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef RE_OPENCL_H
#define RE_OPENCL_H

#if defined(__APPLE__)
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include <QHash>
#include <QLibrary>
#include <QSharedPointer>

#include "reality_lib_export.h"


/**
 * This file defines functions to help discovering if OpenCL is enabled on
 * the users machine and what OCL devices are available
 */

namespace Reality {

/**
 * Describes an OpenCL device
 */
class REALITY_LIB_EXPORT ReOCLDevice {
public:
  //! The type of this device
  enum DeviceType {
    RE_OCL_CPU, RE_OCL_GPU, RE_OCL_ACCELERATOR
  };


private:
  //! This is the original, raw, name reported by OpenCL for this device
  QString name;

  //! This is the name used to show the device to the user. It can be the
  //! same as name or it can be a formatted, sanitized version
  QString label;

  //! This is the device ID reported by OpenCL for this device
  unsigned int deviceNumber;

  DeviceType devType;

public:
  //! Constructor: ReOCLDevice
  ReOCLDevice() : devType(RE_OCL_GPU) {
  };

  ReOCLDevice( const QString& name, 
               const QString& label, 
               const unsigned int id,
               const DeviceType devType ) :
    name(name),
    label(label),
    deviceNumber(id),
    devType(devType)
  {
  }

  ~ReOCLDevice() {
    // RE_LOG_INFO() << "OCL Device " << name.toStdString() << " is getting deleted";
  }

  /**
   * Returns the value of property name
   */
  inline const QString getName() const {
    return name;
  };

  /**
   * Sets the value of property Name
   */
  inline void setName( QString newVal ) {
    name = newVal;
  };

  /**
   * Returns the value of property label
   */
  inline const QString getLabel() const {
    return label;
  };

  /**
   * Sets the value of property label
   */
  inline void setLabel( QString newVal ) {
    label = newVal;
  };
  
  /**
   * Returns the value of property Number
   */
  inline const unsigned int getNumber() const {
    return deviceNumber;
  };

  /**
   * Sets the value of property Number
   */
  inline void setNumber( unsigned int newVal ) {
    deviceNumber = newVal;
  };

  inline DeviceType getType() {
    return devType;
  }

  inline QString getTypeAsString() {
    switch(devType) {
      case RE_OCL_CPU:
        return "CPU";
      case RE_OCL_GPU:
        return "GPU";
      case RE_OCL_ACCELERATOR:
        return "Accelerator";
      default:
        return "Unknown";
    };
  }

  QString toString() {
    return QString("%1 - type %2").arg(getName()).arg(getTypeAsString());
  }
};

typedef QSharedPointer<ReOCLDevice> ReOCLDevicePtr;
typedef QList<ReOCLDevicePtr> ReOCLDeviceList;
/**
 * Utility class to interface and query the OpenCL driver. This is a singleton.
 */
class REALITY_LIB_EXPORT ReOpenCL {
private:
  //! Constructor: ReOpenCL
  explicit ReOpenCL();

  //! Instance variable to implement the singleton pattern
  static ReOpenCL* instance;

  //! The reference to the system's OpenCL library
  QLibrary libOCL;
  bool hasOpenCL;

  //! List of OpenCL platform names
  typedef QHash<cl_platform_id, QString> ReOCLPlatformList;
  typedef QHashIterator<cl_platform_id, QString> ReOCLPlatformIterator;
  ReOCLPlatformList platformList;

  //! List of OpenCL device names
  ReOCLDeviceList deviceList;

  //! Find the number and names of the available OpenCL platforms
  void findPlatForms();

  //! Find the number and names of the available OpenCL devices
  void findDevices();

  void resolveSymbols();

public:

  ~ReOpenCL();
  
  static ReOpenCL* getInstance();

  //! Returns a boolean signifying if the machine has a working 
  //! OpenCL implementation
  bool isEnabled();

  ReOCLDeviceList getDeviceList();

  ReOCLPlatformList getPlatformList();

  //! Overloaded version expected to be called as getPlatformList<QStringList>()
  template<typename T> T getPlatformList() {
    ReOCLPlatformIterator i(platformList);
    T l;
    while(i.hasNext()) {
      i.next();
      l << i.value();
    }
    return l;
  }
};

} // namespace

#endif
