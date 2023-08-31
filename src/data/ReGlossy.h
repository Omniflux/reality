/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef REGLOSSY_H
#define REGLOSSY_H

#include "reality_lib_export.h"
#include "ReModifiedMaterial.h"


#define RE_GLOSSY_KD_CH "Kd"
#define RE_GLOSSY_KS_CH "Ks"
#define RE_GLOSSY_KA_CH "Ka"
#define RE_GLOSSY_KT_CH "Kt"
#define RE_GLOSSY_KG_CH "Kg"
#define RE_GLOSSY_COAT_DEFAULT_THICKNESS 0.2f

namespace Reality {

/**
 The Glossy and Glossy Translucent material.
 */
class REALITY_LIB_EXPORT ReGlossy: public ReModifiedMaterial {
protected:
  // Used as a placeholder and for the rare case when 
  // a texture might become null
  ReTexturePtr dummy;

  qint32 uGlossiness, vGlossiness;

  bool surfaceFuzz;

  bool topCoat;
  bool translucent;

  float coatThickness;

public:

   explicit ReGlossy( const QString name, const ReGeometryObject* parent);

  void fromMaterial( const ReMaterial* baseMat );

  ~ReGlossy();

  void setKd(const ReTexturePtr _Kd);

  void setKs(const ReTexturePtr newKs);

  void setKa(const ReTexturePtr newKa);
  void setKt(const ReTexturePtr newKt);

  inline ReTexturePtr getKd() const {
    return channels["Kd"];
  }
  inline ReTexturePtr getKs() const {
    return channels["Ks"];
  }
  inline ReTexturePtr getKa() const {
    return channels["Ka"];
  }
  inline ReTexturePtr getKt() const {
    return channels["Kt"];
  }

  inline qint32 getUGlossiness() const{
    return uGlossiness;
  }
  inline void setUGlossiness( const qint32 newVal){
    uGlossiness = newVal;
  }

  inline qint32 getVGlossiness() const{
    return vGlossiness;
  }

  inline void setVGlossiness( const qint32 newVal){
    vGlossiness = newVal;
  }

  /*
   Method: getKg
   */
  const ReTexturePtr getKg() const;

  /*
   Method: setKg
   */
  void setKg( ReTexturePtr newVal );

  /*
   Method: getSurfaceFuzz
   */
  inline bool getSurfaceFuzz() const{
    return surfaceFuzz;
  };

  /*
   Method: setSurfaceFuzz
   */
  inline void setSurfaceFuzz( bool newVal ){
    surfaceFuzz = newVal;
  };

  /*
   Method: getCoatThickness
   */
  inline float getCoatThickness() const{
    return coatThickness;
  };

  /*
   Method: setCoatThickness
   */
  void setCoatThickness( float newVal ){
    coatThickness = newVal;
  };

  /**
   Method: isTranslucent
   */
  inline bool isTranslucent() const {
    return translucent;
  };

  /**
   Method: setTranslucent
   */
  inline void setTranslucent( bool newVal ) {
    translucent = newVal;
  };

  /**
    Returns the value of property topCoat
    */
  inline bool hasTopCoat() const {
    return topCoat;
  };

  /**
   Sets the value of property topCoat
   */
  inline void setTopCoat( bool newVal ) {
    topCoat = newVal;
  };


  void setNamedValue( const QString& name, const QVariant& value );

  const QVariant getNamedValue( const QString& name ) const;

  /*
   Method: serialize

   This is used to transmit the material between the host-side and GUI-side
   parts of Reality.
  */
  void serialize( QDataStream& dataStream ) const;

  /*
   Method: deserialize

   Rebuilds the material from a data stream. Used to transmit the material between the 
   host-side and GUI-side parts of Reality.
   */
  void deserialize( QDataStream& dataStream );

  QString toString();
};

typedef QSharedPointer<ReGlossy> ReGlossyPtr;


} // namespace

#endif


