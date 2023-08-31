/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#include "ReOpenCL.h"

#include "ReLogger.h"


//! Definitions of the OpenCL functions that we are going to need. These
//! will be used to dynamically resolve the function pointers once the 
//! OpenCL library is loaded


typedef cl_int (*clGetPlatformIDs_t)(cl_uint         /* num_entries */, 
                                     cl_platform_id* /* platforms */, 
                                     cl_uint*        /* num_platforms */);

typedef cl_int (*clGetDeviceIDs_t)(cl_platform_id   /* platform */,
                                   cl_device_type   /* device_type */, 
                                   cl_uint          /* num_entries */, 
                                   cl_device_id *   /* devices */, 
                                   cl_uint *        /* num_devices */);

typedef cl_int (*clGetPlatformInfo_t)(cl_platform_id   platform,
                                      cl_platform_info param_name,
                                      size_t           param_value_size,
                                      void*            param_value,
                                      size_t*          param_value_size_ret);

typedef cl_int (*clGetDeviceInfo_t)(cl_device_id    device,
                                    cl_device_info  param_name, 
                                    size_t          param_value_size, 
                                    void *          param_value,
                                    size_t *        param_value_size_ret);

//! OCL API function pointers that will be resolved at runtime
clGetPlatformIDs_t  oclGetPlatformIDs;
clGetDeviceIDs_t    oclGetDeviceIDs;
clGetPlatformInfo_t oclGetPlatformInfo;
clGetDeviceInfo_t   oclGetDeviceInfo;

using namespace Reality;

ReOpenCL* ReOpenCL::instance = NULL;

ReOpenCL::ReOpenCL() {
  hasOpenCL = false;

  // Try to dynamically load the library 
#if defined(__APPLE__)
  QString libpath = "/System/Library/Frameworks/OpenCL.framework/OpenCL";
#elif defined(_WIN32)
  // QString libpath = "C:/Windows/System32/OpenCL";
  QString libpath = "OpenCL";
#endif
  libOCL.setFileName(libpath);
  hasOpenCL = libOCL.load();
  if (hasOpenCL) {
    RE_LOG_INFO() << "Loaded OpenCL library " << libOCL.fileName();
    oclGetPlatformIDs  = (clGetPlatformIDs_t)  libOCL.resolve("clGetPlatformIDs");
    oclGetDeviceIDs    = (clGetDeviceIDs_t)    libOCL.resolve("clGetDeviceIDs");
    oclGetPlatformInfo = (clGetPlatformInfo_t) libOCL.resolve("clGetPlatformInfo");
    oclGetDeviceInfo   = (clGetDeviceInfo_t)   libOCL.resolve("clGetDeviceInfo");
    findPlatForms();
    findDevices();
  }
  else {
    RE_LOG_INFO() << "OpenCL library could not be loaded";
  }
}

ReOpenCL::~ReOpenCL() {
}

void ReOpenCL::resolveSymbols() {
  oclGetPlatformIDs  = (clGetPlatformIDs_t)  libOCL.resolve("clGetPlatformIDs");
  if (!oclGetPlatformIDs) {
    hasOpenCL = false;
    return;
  }
  oclGetDeviceIDs    = (clGetDeviceIDs_t)    libOCL.resolve("clGetDeviceIDs");
  if (!oclGetDeviceIDs) {
    hasOpenCL = false;
    return;
  }
  oclGetPlatformInfo = (clGetPlatformInfo_t) libOCL.resolve("clGetPlatformInfo");
  if (!oclGetPlatformInfo) {
    hasOpenCL = false;
    return;
  }
  oclGetDeviceInfo   = (clGetDeviceInfo_t)   libOCL.resolve("clGetDeviceInfo");
  if (!oclGetPlatformInfo) {
    hasOpenCL = false;
    return;
  }
}

QString formatOCLVersionString( QString rawOCLString ) {
  // Remove the "OpenCL" string at the beginning of the version number
  rawOCLString.replace(QRegExp("^OpenCL "), "");
  // Change " (Date)"" to " - Date"
  rawOCLString.replace(" (", " - ");
  rawOCLString.replace(')', "");

  // Removes "CUDA x.y.z"
  return rawOCLString.replace(QRegExp("CUDA\\s\\d+\\.\\d+\\.\\d+"), "");
}

QString formatOCLPlatformString( QString rawOCLString ) {
  rawOCLString.replace("NVIDIA CUDA", "nVidia", Qt::CaseSensitive);
  return rawOCLString.replace("Intel(R) OpenCL", "Intel", Qt::CaseSensitive);
}


QString formatOCLDeviceString( QString devStr ) {
  devStr.replace("(R)", "");
  return devStr.simplified();
}


void ReOpenCL::findPlatForms() {
  if (!hasOpenCL) {
    return;
  }
  const int platformNameSize = 128;
  const int platformVerSize = 40;
  char platformName[platformNameSize];
  char platformVersion[platformVerSize];
  size_t retval;

  cl_platform_id* platforms = NULL;
  cl_uint platformCount;

  oclGetPlatformIDs(0, NULL, &platformCount);
  platforms = new cl_platform_id[platformCount];
  oclGetPlatformIDs(platformCount, platforms, NULL);

  for (int i = 0; i < platformCount; i++) {
    oclGetPlatformInfo(platforms[i], 
                       CL_PLATFORM_NAME, 
                       platformNameSize,
                       platformName,
                       &retval );
    oclGetPlatformInfo(platforms[i], 
                       CL_PLATFORM_VERSION,
                       platformVerSize,
                       platformVersion,
                       &retval );
    QString platform = QString("%1 - v.%2")
                      .arg(formatOCLPlatformString(platformName))
                      .arg(formatOCLVersionString(platformVersion));
    platformList[platforms[i]] = platform;
    RE_LOG_INFO() << "OCL platform: " << platforms[i] << " " << platform;
  }
  if (platforms) {
    delete[] platforms;
  }
}

void ReOpenCL::findDevices() {
  if (!hasOpenCL) {
    return;
  }

  // cl_platform_id* platforms;
  // cl_uint platformCount;
  const int deviceNameSize = 128;
  char deviceName[deviceNameSize];

  // oclGetPlatformIDs(0, NULL, &platformCount);
  // platforms = new cl_platform_id[platformCount];
  // oclGetPlatformIDs(platformCount, platforms, NULL);
  // RE_LOG_INFO() << "Num platforms: " << platformCount;
  int deviceID = 0;
  ReOCLPlatformIterator i(platformList);
  // for (int i = 0; i < platformCount; i++) {
  while( i.hasNext() ) {
    cl_uint deviceCount;
    cl_device_id* devices;
    i.next();
    // get all devices for this platform
    oclGetDeviceIDs(i.key(), CL_DEVICE_TYPE_ALL, 0, NULL, &deviceCount);
    RE_LOG_INFO() << "Found " << deviceCount << " devices for platform " << i.value();
    // devices = (cl_device_id*) malloc(sizeof(cl_device_id) * deviceCount);
    devices = new cl_device_id[deviceCount];
    oclGetDeviceIDs(i.key(), CL_DEVICE_TYPE_ALL, deviceCount, devices, NULL);

    // for each device print critical attributes
    for (int numDevice = 0; numDevice < deviceCount; numDevice++) {
      oclGetDeviceInfo(devices[numDevice], 
                       CL_DEVICE_NAME, 
                       deviceNameSize, 
                       deviceName, 
                       NULL);
      // Get the type of the device
      cl_device_type devType;
      oclGetDeviceInfo(devices[numDevice], 
                       CL_DEVICE_TYPE, 
                       sizeof(devType), 
                       &devType,
                       NULL);
      ReOCLDevice::DeviceType reDevType;
      switch(devType) {
        case CL_DEVICE_TYPE_CPU:
          reDevType =  ReOCLDevice::DeviceType::RE_OCL_CPU;
          break;
        case CL_DEVICE_TYPE_GPU:
          reDevType =  ReOCLDevice::DeviceType::RE_OCL_GPU;
          break;
        case CL_DEVICE_TYPE_ACCELERATOR:
          reDevType =  ReOCLDevice::DeviceType::RE_OCL_ACCELERATOR;
          break;
        default:
          // Just to avoid keeping a variable un-initialized
          reDevType = ReOCLDevice::DeviceType::RE_OCL_ACCELERATOR;
          RE_LOG_INFO() << "OCL Device " << deviceName 
                        << " is of unsupported type " << devType;
      };
      deviceList.append( 
        ReOCLDevicePtr( new ReOCLDevice( 
                              deviceName, 
                              formatOCLDeviceString(deviceName),
                              deviceID++,
                              reDevType
                            )
                      )
      );
      RE_LOG_INFO() << "Found OCL device: " << deviceList.last()->toString();
    }
    delete[] devices;
  }

  // delete[] platforms;
}

ReOpenCL* ReOpenCL::getInstance() {
  if (!instance) {
    instance = new ReOpenCL();
  }
  return instance;
}

//! Returns a boolean signifying if the machine has a working 
//! OpenCL implementation
bool ReOpenCL::isEnabled() {
  return hasOpenCL;
};

ReOpenCL::ReOCLPlatformList ReOpenCL::getPlatformList() {
  return platformList;
}

ReOCLDeviceList ReOpenCL::getDeviceList() {
  return deviceList;
};
