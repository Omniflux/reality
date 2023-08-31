/*
 *  Created by Paolo H Ciccone on 6/25/11.
 *  Copyright 2011 Pret-A-3D. All rights reserved.
 */

#ifndef RE_SURFACE_INTEGRATOR
#define RE_SURFACE_INTEGRATOR

#include <QVariantMap>

#include "reality_lib_export.h"
#include "ReDefs.h"


namespace Reality {
 
/**
 * Base Class for all Surface Integrators
 */
class REALITY_LIB_EXPORT ReSurfaceIntegrator {

protected:
  QVariantMap values;  
  ReSurfaceIntegratorType type;

public:
  
  ReSurfaceIntegrator( const ReSurfaceIntegratorType type) :
    type(type)
  {
  
  };

  //! Destructor
  virtual ~ReSurfaceIntegrator() {

  }

  virtual QVariantMap& getValues() = 0;

  //! Sets all the values of the integrator with one single call
  //! by using a QVariantMap
  virtual void setValues( const QVariantMap& params ) = 0;

  virtual void setNamedValue( const QString& vName, const QVariant& newVal ) {
    if (vName == "type") {
      type = static_cast<ReSurfaceIntegratorType>(newVal.toInt());
    }
  }
  virtual QVariant getNamedValue( const QString& /* vName */ ) const {
    return type;
  }

  //! Returns the type of the integrator   
  inline ReSurfaceIntegratorType getType() {
    return type;
  }

  // friend QDataStream& operator <<( QDataStream& strm, ReSurfaceIntegrator& si );
  // friend QDataStream& operator >>( QDataStream& strm, ReSurfaceIntegrator* si );

};
//! Smart pointer type for the surface integrator
typedef QSharedPointer<ReSurfaceIntegrator> ReSurfaceIntegratorPtr;

#define BIDIR_EYE_DEPTH      "eyeDepth"
#define BIDIR_LIGHT_DEPTH    "lightDepth"
#define BIDIR_LIGHT_STRATEGY "lightStrategy"
#define BIDIR_EYE_RR         "eyeRR"
#define BIDIR_LIGHT_RR       "lightRR"
/**
 *  Surface Integrator Bidir
 */
class REALITY_LIB_EXPORT ReSIBidir : public ReSurfaceIntegrator {
  
protected:
  uint eyeDepth; 
  uint lightDepth; 
  uint eyeRR; 
  uint lightRR;
  QString lightStrategy;


public:
  
  ReSIBidir() : 
    ReSurfaceIntegrator(SI_BIDIR),
    eyeDepth(16), 
    lightDepth(16), 
    eyeRR(0), 
    lightRR(0),
    lightStrategy("auto")
  {

  };
  
  void setValues( const QVariantMap& params ) {
    if (params.contains(BIDIR_EYE_DEPTH)) {
      eyeDepth = params[BIDIR_EYE_DEPTH].toInt();
    }
    if (params.contains(BIDIR_LIGHT_DEPTH)) {
      lightDepth = params[BIDIR_LIGHT_DEPTH].toInt();
    }
    if (params.contains(BIDIR_LIGHT_STRATEGY)) {
      lightStrategy = params[BIDIR_LIGHT_STRATEGY].toString().toLower();
    }
  };

  void setNamedValue( const QString& vName, const QVariant& newVal ) {
    if (vName == "eyeDepth") {
      eyeDepth = newVal.toInt();
    }
    else if (vName == "lightDepth") {
      lightDepth = newVal.toInt();
    }
    else if (vName == "eyeRR") {
      eyeRR = newVal.toInt();
    }

    else if (vName == "lightRR") {
      lightRR = newVal.toInt();
    }
    else if (vName == "lightStrategy") {
      lightStrategy = newVal.toString();
    }
    else {
      ReSurfaceIntegrator::setNamedValue(vName, newVal);
    }
  };

  QVariant getNamedValue( const QString& vName ) const {
    if (vName == "eyeDepth") {
      return eyeDepth;
    }
    else if (vName == "lightDepth") {
      return lightDepth;
    }
    else if (vName == "eyeRR") {
      return eyeRR;      
    }
    else if (vName == "lightRR") {
      return lightRR;      
    }
    else if (vName == "lightStrategy") {
      return lightStrategy;     
    }
    else {
      return ReSurfaceIntegrator::getNamedValue(vName);
    }
  };

  
  virtual QVariantMap& getValues() {
    values.clear();
    values[BIDIR_EYE_DEPTH]      = eyeDepth;
    values[BIDIR_LIGHT_DEPTH]    = lightDepth;
    values[BIDIR_LIGHT_STRATEGY] = lightStrategy;
    values[BIDIR_EYE_RR]         = eyeRR;
    values[BIDIR_LIGHT_RR]       = lightRR;
    return(values);
  };

  inline uint getEyeDepth() {
    return eyeDepth;
  }

  inline uint getLightDepth() {
    return lightDepth;
  }

  inline QString getLightStrategy() {
    return lightStrategy;
  }

  inline void setLightStrategy( const QString newVal ) {
    lightStrategy = newVal;
  }

};

typedef QSharedPointer<ReSIBidir> ReSIBidirPtr;


#define PHOTON_MAP_DIRECT_LIGHTING   "directLighting"
#define PHOTON_MAP_DIRECT_PHOTONS    "directPhotons"
#define PHOTON_MAP_INDIRECT_PHOTONS  "indirectPhotons"
#define PHOTON_MAP_CAUSTIC_PHOTONS   "causticPhotons"
#define PHOTON_MAP_MAX_DEPTH         "maxDepth"
#define PHOTON_MAP_MAX_PHOTON_DEPTH  "maxPhotonDepth"

/**
 *  Surface Integrator ExPhotonMap
 */
class REALITY_LIB_EXPORT ReSIExPhotonMap : public ReSurfaceIntegrator {
  
protected:
  bool directLighting;
  uint directPhotons;
  uint indirectPhotons;
  uint causticPhotons;
  uint maxDepth;
  uint maxPhotonDepth;

public:
  
  ReSIExPhotonMap() : 
    ReSurfaceIntegrator(SI_EXPM)
  {
    directLighting  = true;
    directPhotons   = 1000000;
    indirectPhotons = 1000000;
    causticPhotons  = 1000000;
    maxDepth        = 5;
    maxPhotonDepth  = 10;
  };

  void setNamedValue( const QString& vName, const QVariant& newVal ) {
    if (vName == "directLighting") {
      directLighting = newVal.toBool();
    }
    else if (vName == "directPhotons") {
      directPhotons = newVal.toInt();
    }
    else if (vName == "indirectPhotons") {
      indirectPhotons = newVal.toInt();
    }
    else if (vName == "causticPhotons") {
      causticPhotons = newVal.toInt();
    }
    else if (vName == "maxDepth") {
      maxDepth = newVal.toInt();
    }
    else if (vName == "maxPhotonDepth") {
      maxPhotonDepth = newVal.toInt();
    }
    else {
      ReSurfaceIntegrator::setNamedValue(vName, newVal);
    }
  };

  QVariant getNamedValue( const QString& vName ) const {
    if (vName == "directLighting") {
      return directLighting;
    }
    else if (vName == "directPhotons") {
      return directPhotons;
    }
    else if (vName == "indirectPhotons") {
      return indirectPhotons;
    }
    else if (vName == "causticPhotons") {
      return causticPhotons;      
    }
    else if (vName == "maxDepth") {
      return maxDepth;
    }
    else if (vName == "maxPhotonDepth") {
      return maxPhotonDepth;     
    }
    else {
      return ReSurfaceIntegrator::getNamedValue(vName);
    }
  };
  
  void setValues(const QVariantMap& params ) {
    if (params.contains(PHOTON_MAP_DIRECT_LIGHTING)) {
      directLighting = params[PHOTON_MAP_DIRECT_LIGHTING].toBool();
    }
    if (params.contains(PHOTON_MAP_DIRECT_PHOTONS)) {
      directPhotons = params[PHOTON_MAP_DIRECT_PHOTONS].toInt();
    }
    if (params.contains(PHOTON_MAP_INDIRECT_PHOTONS)) {
      indirectPhotons = params[PHOTON_MAP_INDIRECT_PHOTONS].toInt();
    }
    if (params.contains(PHOTON_MAP_CAUSTIC_PHOTONS)) {
      causticPhotons = params[PHOTON_MAP_CAUSTIC_PHOTONS].toInt();
    }
    if (params.contains(PHOTON_MAP_MAX_DEPTH)) {
      maxDepth = params[PHOTON_MAP_MAX_DEPTH].toInt();
    }
    if (params.contains(PHOTON_MAP_MAX_PHOTON_DEPTH)) {
      maxPhotonDepth = params[PHOTON_MAP_MAX_PHOTON_DEPTH].toInt();
    }
  };
  
  virtual QVariantMap& getValues() {
    values.clear();
    values[PHOTON_MAP_DIRECT_LIGHTING]  = directLighting;
    values[PHOTON_MAP_DIRECT_PHOTONS]   = directPhotons;
    values[PHOTON_MAP_INDIRECT_PHOTONS] = indirectPhotons;
    values[PHOTON_MAP_CAUSTIC_PHOTONS]  = causticPhotons;
    values[PHOTON_MAP_MAX_DEPTH]        = maxDepth;
    values[PHOTON_MAP_MAX_PHOTON_DEPTH] = maxPhotonDepth;
    return(values);
  };
};


#define SPPM_MAX_EYE_DEPTH       "maxEyeDepth"
#define SPPM_MAX_PHOTON_DEPTH    "maxPhotonDepth"
#define SPPM_PHOTONS_PER_PASS    "photonsPerPass"
#define SPPM_START_RADIUS        "startRadius"
#define SPPM_ALPHA               "alpha"
#define SPPM_INCLUDE_ENVIRONMENT "includeEnvironment"
#define SPPM_LOOKUP_ACCELERATOR  "lookupAccel"
#define SPPM_PIXEL_SAMPLER       "pixelSampler"
#define SPPM_PHOTON_SAMPLER      "photonSampler"
#define SPPM_DIRECT_SAMPLING     "directSampling"

/**
 *  Surface Integrator SPPM
 */
class REALITY_LIB_EXPORT ReSISPPM : public ReSurfaceIntegrator {
  
protected:
  unsigned int maxEyeDepth;
  unsigned int maxPhotonDepth;
  unsigned int photonsPerPass;
  float startRadius;
  float alpha;
  bool includeEnvironment;
  bool directSampling;
  QString lookupAccelerator;
  QString pixelSampler;
  QString photonSampler;

public:
  
  ReSISPPM() :
    ReSurfaceIntegrator(SI_SPPM)
  {
    maxEyeDepth        = 32;
    maxPhotonDepth     = 32;
    photonsPerPass     = 2000000;
    startRadius        = 1.2f;
    alpha              = 0.7f;
    includeEnvironment = true;
    directSampling     = true;
    lookupAccelerator  = "hybridhashgrid";
    pixelSampler       = "vegas";
    photonSampler      = "halton";
  };

  void setNamedValue( const QString& vName, const QVariant& newVal ) {
    if (vName == "maxEyeDepth") {
      maxEyeDepth = newVal.toInt();
    }
    else if (vName == "maxPhotonDepth") {
      maxPhotonDepth = newVal.toInt();
    }
    else if (vName == "photonsPerPass") {
      photonsPerPass = newVal.toInt();
    }
    else if (vName == "startRadius") {
      startRadius = newVal.toFloat();
    }
    else if (vName == "alpha") {
      alpha = newVal.toFloat();
    }
    else if (vName == "includeEnvironment") {
      includeEnvironment = newVal.toBool();
    }
    else if (vName == "directSampling") {
      directSampling = newVal.toBool();
    }
    else if (vName == "lookupAccelerator") {
      lookupAccelerator = newVal.toString();
    }
    else if (vName == "pixelSampler") {
      pixelSampler = newVal.toString();
    }
    else if (vName == "photonSampler") {
      photonSampler = newVal.toString();
    }
    else {
      ReSurfaceIntegrator::setNamedValue(vName, newVal);
    }
  };

  QVariant getNamedValue( const QString& vName ) const {
    if (vName == "maxEyeDepth") {
      return maxEyeDepth;
    }
    else if (vName == "maxPhotonDepth") {
      return maxPhotonDepth;
    }
    else if (vName == "photonsPerPass") {
      return photonsPerPass;
    }
    else if (vName == "startRadius") {
      return startRadius;
    }
    else if (vName == "alpha") {
      return alpha;
    }
    else if (vName == "includeEnvironment") {
      return includeEnvironment;
    }
    else if (vName == "directSampling") {
      return directSampling;
    }
    else if (vName == "lookupAccelerator") {
      return lookupAccelerator;
    }
    else if (vName == "pixelSampler") {
      return pixelSampler;
    }
    else if (vName == "photonSampler") {
      return photonSampler;
    }
    else {
      return ReSurfaceIntegrator::getNamedValue(vName);
    }
  };

  void setValues(const QVariantMap& params ) {
    if (params.contains(SPPM_MAX_EYE_DEPTH)) {
      maxEyeDepth = params[SPPM_MAX_EYE_DEPTH].toInt();
    };
    if (params.contains(SPPM_MAX_PHOTON_DEPTH)) {
      maxPhotonDepth = params[SPPM_MAX_PHOTON_DEPTH].toInt();
    };
    if (params.contains(SPPM_PHOTONS_PER_PASS)) {
      photonsPerPass = params[SPPM_PHOTONS_PER_PASS].toInt();
    };
    if (params.contains(SPPM_START_RADIUS)) {
      startRadius = params[SPPM_START_RADIUS].toDouble();
    };
    if (params.contains(SPPM_ALPHA)) {
      alpha = params[SPPM_ALPHA].toDouble();
    };
    if (params.contains(SPPM_INCLUDE_ENVIRONMENT)) {
      includeEnvironment = params[SPPM_INCLUDE_ENVIRONMENT].toBool();
    };
    if (params.contains(SPPM_LOOKUP_ACCELERATOR)) {
      lookupAccelerator = params[SPPM_LOOKUP_ACCELERATOR].toString();
    };
    if (params.contains(SPPM_PIXEL_SAMPLER)) {
      pixelSampler = params[SPPM_PIXEL_SAMPLER].toString();
    };
    if (params.contains(SPPM_PHOTON_SAMPLER)) {
      photonSampler = params[SPPM_PHOTON_SAMPLER].toString();
    };
    if (params.contains(SPPM_DIRECT_SAMPLING)) {
      directSampling = params[SPPM_DIRECT_SAMPLING].toBool();
    };
  };
  
  virtual QVariantMap& getValues() {
    values.clear();
    values[SPPM_MAX_EYE_DEPTH]       = maxEyeDepth;
    values[SPPM_MAX_PHOTON_DEPTH]    = maxPhotonDepth;
    values[SPPM_PHOTONS_PER_PASS]    = photonsPerPass;
    values[SPPM_START_RADIUS]        = startRadius;
    values[SPPM_ALPHA]               = alpha;
    values[SPPM_INCLUDE_ENVIRONMENT] = includeEnvironment;
    values[SPPM_LOOKUP_ACCELERATOR]  = lookupAccelerator;
    values[SPPM_PIXEL_SAMPLER]       = pixelSampler;
    values[SPPM_PHOTON_SAMPLER]      = photonSampler;
    values[SPPM_DIRECT_SAMPLING]     = directSampling;
    return(values);
  }
};


#define PATH_MAX_DEPTH      "maxDepth"
#define PATH_LIGHT_STRATEGY "lightStrategy"

/**
 *  Surface Integrator Path
 */
class REALITY_LIB_EXPORT ReSIPath : public ReSurfaceIntegrator {
  
protected:
  uint maxDepth;
  QString lightStrategy;  

public:  
  ReSIPath() :
    ReSurfaceIntegrator(SI_PATH)
  {
    maxDepth = 16;
    lightStrategy = "one";
  };

  void setNamedValue( const QString& vName, const QVariant& newVal ) {
    if (vName == "maxDepth") {
      maxDepth = newVal.toInt();
    }
    else if (vName == "lightStrategy") {
      lightStrategy = newVal.toString();
    }
    else {
      ReSurfaceIntegrator::setNamedValue(vName, newVal);
    }
  };

  QVariant getNamedValue( const QString& vName ) const {
    if (vName == "maxDepth") {
      return maxDepth;
    }
    else if (vName == "lightStrategy") {
      return lightStrategy;
    }
    else {
      return ReSurfaceIntegrator::getNamedValue(vName);
    }
  };

  virtual QVariantMap& getValues() {
    values.clear();
    values[PATH_MAX_DEPTH] = maxDepth;
    values[PATH_LIGHT_STRATEGY] = lightStrategy;
    return(values);
  };

  void setValues(const QVariantMap& params ) {
    if (params.contains(PATH_MAX_DEPTH)) {
      maxDepth = params[PATH_MAX_DEPTH].toInt();
    }
    if (params.contains(PATH_LIGHT_STRATEGY)) {
      lightStrategy = params[PATH_LIGHT_STRATEGY].toString();
    }
  };

};

/**
 * Path Surface Integrator for the GPU accelerated renderer
 */
class REALITY_LIB_EXPORT ReSIPathGPU : public ReSIPath {

protected:
  QString rrStrategy;

public:
  
  ReSIPathGPU() {
    maxDepth = 10; 
    lightStrategy = "one";
    rrStrategy    = "efficiency";
  }

  virtual QVariantMap& getValues() {
    ReSIPath::getValues();
    values[PATH_MAX_DEPTH] = maxDepth;
    return(values);
  };

  void setValues(const QVariantMap& params ) {
    if (params.contains(PATH_MAX_DEPTH)) {
      maxDepth = params[PATH_MAX_DEPTH].toInt();
    }
  };
};

#define DIRECT_MAX_DEPTH "maxDepth"
/**
 * Used for the Preview mode
 */
class REALITY_LIB_EXPORT ReSIDirect : public ReSurfaceIntegrator {
protected:
  int maxDepth;

public:
  
  ReSIDirect( const uint maxDepth = 5 ) :
    ReSurfaceIntegrator(SI_DIRECT),
    maxDepth(maxDepth)
  {
    
  }

  QVariantMap& getValues() {
    values.clear();
    return(values);
  };
  void setValues(const QVariantMap& /* params */) {
    // Nothing yet
  };
  
  inline int getMaxDepth() {
    return maxDepth;
  }
};

inline QDataStream& operator <<( QDataStream& strm, ReSurfaceIntegratorPtr si ) {
  auto vals = si->getValues();
  strm << (quint16) si->getType() << vals;
  return strm;
}

inline QDataStream& operator >>( QDataStream& strm, ReSurfaceIntegratorPtr& si ) {
  quint16 siTypeInt;
  QVariantMap vals;
  strm >> siTypeInt >> vals;

  ReSurfaceIntegratorType siType = static_cast<ReSurfaceIntegratorType>(siTypeInt);
  switch(siType) {
    case SI_BIDIR:
      si = ReSurfaceIntegratorPtr(new ReSIBidir());
      break;
    case SI_PATH:
      si = ReSurfaceIntegratorPtr(new ReSIPath());
      break;
    case SI_SPPM:
      si = ReSurfaceIntegratorPtr(new ReSISPPM());
      break;
    case SI_EXPM:
      si = ReSurfaceIntegratorPtr(new ReSIExPhotonMap());
      break;
    case SI_DIRECT:
      si = ReSurfaceIntegratorPtr(new ReSIDirect());
      break;
    default:
      si = ReSurfaceIntegratorPtr(new ReSIBidir());
  }
  si->setValues(vals);
  return strm;
}

} // namespace

#endif
