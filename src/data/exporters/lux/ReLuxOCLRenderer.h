/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef RE_LUX_OCL_RENDERER_H
#define RE_LUX_OCL_RENDERER_H

#include "ReOpenCL.h" 
#include "ReSceneData.h"
#include "ReSceneDataGlobal.h"
#include "exporters/ReLuxRendererConfig.h"


using namespace Reality;

/**
 * Class that defines the default parameters that we use when
 * rendering with classic of accelerated (Embree) CPU renderer.
 */
class ReLuxOCLRenderer : public ReLuxRendererConfig {

private:
  //! This byte keeps the list of which OCL devices have been enabled
  //! to participate int he rendering
  unsigned char oclDevicesFlag;
  unsigned short int numDevices;

public:
  //! Constructor: ReLuxOCLRenderer
  ReLuxOCLRenderer() : oclDevicesFlag(0), numDevices(0)
  {
  };

  inline void setEnabledDevices( unsigned char newFlag, unsigned short int numDevs ) {
    oclDevicesFlag = newFlag;
    numDevices = numDevs;
  }

  inline bool isDeviceActive( const unsigned short int devNo ) {
    return ( (oclDevicesFlag >> devNo) & 1U );
  }

  inline QString getEnabledDevicesString() {
    if (numDevices == 0) {
      return "";
    }
    QString deviceFlagStr;
    for (int i = 0; i < numDevices; i++) {
      if ( (oclDevicesFlag >> i) & 1U ) {
        deviceFlagStr += "1";
      }
      else {
        deviceFlagStr += "0";
      }
    }
    return QString("\"opencl.devices.select = %1\"").arg(deviceFlagStr);
  }

  inline QString getWorkGroupSizeString() {
    ReOpenCL* ocl = ReOpenCL::getInstance();

    auto dl = ocl->getDeviceList();
    bool hasGPU = false;
    bool hasCPU = false;
    for (int i = 0; i < dl.count(); i++) {
      if (!isDeviceActive(i)) {
        continue;
      }
      switch(dl[i]->getType()) {
        case ReOCLDevice::RE_OCL_GPU:
          hasGPU = true;
          break;
        case ReOCLDevice::RE_OCL_CPU:
          hasCPU = true;
          break;
        default: break;
      } 
    }
    QString str;
    if (int wg = RealitySceneData->getOCLGroupSize()) {    
      if (hasGPU) {
        str += QString("\"opencl.gpu.workgroup.size = %1\" ").arg(wg);
      }
      if (hasCPU) {
        str += QString("\"opencl.cpu.workgroup.size = %1\" ").arg(wg);
      }
    }
    return str;
  }
};


#endif
