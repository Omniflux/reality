/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef LUX_MATH_TEXTURE_EXPORTER_H
#define LUX_MATH_TEXTURE_EXPORTER_H

#include <QStringBuilder>

#include "reality_lib_export.h"
#include "ReLuxTextureExporter.h"
#include "ReTexture.h"
#include "textures/ReMath.h"


namespace Reality {

/*
  Class: ReLuxMathExporter
 */

class REALITY_LIB_EXPORT ReLuxMathExporter : public ReLuxTextureExporter {
private:
  enum OperantType {
    OP_TEXTURE,
    OP_FLOAT
  };

  /**
   * Method: exportOperand
   * Analyzes an operand for the texture and intelligently generates the 
   * most appropriate output. If the operand has no texture then the simple
   * amount value is returned. 
   * If the texture is present then it is multiplied with the amount unless
   * the amount is 1.
   * If the texture is of type color it's then converted to numeric and the 
   * name of the numeric texture is used in the mix. 
   * 
   * For example, creating as Math texture like this:
   * \code
   *   auto bumpScale = ReMathPtr( new ReMath("mathDemo") );
   *   bumpScale->setTexture1(clampTex);
   *   bumpScale->setAmount2(0.42);
   *   bumpScale->setFunction(ReMath::multiply);
   * \endcode
   *
   * Will result in an exported texture of:
   *
   * \code
   *   Texture "mathDemo" "float" "scale" 
   *     "texture tex1" [<clampTex reference>] 
   *     "float tex2" [0.42] 
   * \endcode
   * 
   * Parameters:
   *   - operandNo: 1 or 2, to indicate the first or second operand
   *   - tex: The input texture
   *   - amount: The input amount
   *   - texName: The name of the texture to use in the mix. Output parameter.
   *   - textures: Additional textures to export. This is were the texture converted
   *               to numeric type are stored.
   * Return:
   *   A value that indicates if the operand is a simple float value or a texture
   */
  OperantType exportOperand( const int openrandNo,
                             const ReTexturePtr tex, 
                             const float amount, 
                             QString& texName,
                             QString& textures ) 
  {
    if (tex.isNull()) {
      return OP_FLOAT;
    }

    ReTexturePtr t1;
    if ( tex->getType() == TexImageMap && 
         tex->getDataType() != ReTexture::numeric) 
    {
      t1 = ReLuxTextureExporter::writeFloatVersion( tex, textures );
    }
    else {
      t1 = tex;
    }

    texName = t1->getUniqueName();
    // Multiply texture1 by amount if it makes sense
    if ( amount != 1.0 ) {
      QString mixName = QString("%1_amount%2").arg(texName).arg(openrandNo);
      auto opTex = ReMathPtr( new ReMath(mixName) );
      opTex->setTexture1(t1);
      opTex->setTexture2( ReTexturePtr() );
      opTex->setAmount2(amount);

      if (!ReLuxTextureExporter::isTextureInCache(opTex)) {
        // Yes, a mutually-recursive call
        textures += exportTexture(opTex);
        ReLuxTextureExporter::addTextureToCache( opTex );
        texName = mixName;
      }
      else {
        texName = ReLuxTextureExporter::getTextureFromCache(opTex)->getUniqueName();
      }
    }
    return OP_TEXTURE;
  }

public:
  const QString exportTexture( ReTexturePtr texbase, 
                               const QString& assignedName = "", 
                               bool isForPreview = false ) 
  {
    ReMathPtr tex = texbase.staticCast<ReMath>();
    QString str;
    QString function;

    switch( tex->getFunction() ) {
      case ReMath::add:
        function = "add";
        break;
      case ReMath::subtract:
        function = "subtract";
        break;
      case ReMath::multiply:
        function = "scale";      
        break;
      default:
        function = "add";
        break;
    }
    QString tex1Name, tex2Name;
    float amount1 = tex->getAmount1();
    float amount2 = tex->getAmount2();

    ReTexturePtr tex1 = ReLuxTextureExporter::getTextureFromCache(
                          tex->getTexture1()
                        );
    ReTexturePtr tex2 = ReLuxTextureExporter::getTextureFromCache(
                          tex->getTexture2()
                        );

    QString textures;
    QString operand1, operand2;
    QString texName;

    if ( exportOperand(1, tex1, amount1, texName, textures) == OP_FLOAT ) {
      operand1 = QString("\"float tex1\" [%1]").arg(amount1);
    }
    else {
      operand1 = QString("\"texture tex1\" [\"%1\"]").arg(texName);
    }
    if ( exportOperand(2, tex2, amount2, texName, textures) == OP_FLOAT ) {
      operand2 = QString("\"float tex2\" [%1]").arg(amount2);
    }
    else {
      operand2 = QString("\"texture tex2\" [\"%1\"]").arg(texName);
    }

    str += QString(
             "Texture \"%1\" \"float\" \"%2\" %3 %4\n" 
           )
           .arg(assignedName == "" ? tex->getUniqueName() : assignedName)  // #1
           .arg(function)
           .arg(operand1)
           .arg(operand2);

    return textures % str;  
  };
};

}
#endif
