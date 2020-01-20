# File: ReMaterial.py
# Reality plug-in
# Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.
#
#
# Module: Material converter
# 
# This module converts the Poser materials to the Reality/Lux equivalents
#

import poser
import ReTools
import Reality

ReTextureType_Band           = 100
ReTextureType_Bricks         = 110
ReTextureType_Checkers       = 120
ReTextureType_Clouds         = 130
ReTextureType_Constant       = 140
ReTextureType_Dots           = 150
ReTextureType_FBM            = 160
ReTextureType_FresnelColor   = 165
ReTextureType_ImageMap       = 170
ReTextureType_Marble         = 180
ReTextureType_Mix            = 190
ReTextureType_MultiMix       = 200
ReTextureType_DistortedNoise = 225
ReTextureType_ColorMath      = 230
ReTextureType_Math           = 235
ReTextureType_Wrinkled       = 240
ReTextureType_Component      = 250
ReTextureType_Wood           = 111


class ReMaterial:

    """
    Variable: reMat

    The Reality version of the material
    """
    reMat = {}

    """
    Method: __init__

    Method to convert from a Poser material to a Reality one
    This method is meant to be fast. Checks are few and only the ones
    strictly necessary. We don't check that the passed parameter is
    of type Material, that has to be done by the caller. This method gets
    called many, many times.

    By default materials starts as undefined (N/A).

    Parameters:
        aMat - a reference to a material from the Actor/Figure

    """
    def __init__(self, aMat):
        self.reMat = { 
            "name"         : "",
            # FireFly
            "source"       : "ff",
            "type"         : "N/A",
            "preset"       : "",
            "nodes"        : {},
            "diffuse"      : {},
            "diffuse 2"    : {},
            "specular"     : {"map":"", "color": [0.0,0.0,0.0]},
            "specular 2"   : {"map":"", "color": [0.0,0.0,0.0]},
            "coat"         : {"map":"", "color": [1.0,1.0,1.0]},
            "bump"         : {},
            "displacement" : {"map":""},
            "translucence" : {},
            "alpha"        : {"map": "", "strength": 1.0},
            "uRoughness"   : 1.0,
            "vRoughness"   : 1.0,
            "roughness map": "",
            "isSkin"       : False,
            # The gain is based on the Ambient_value field
            "lightGain"    : 0.0,
            # Flag that indicates if the light should have a built-in
            # alpha channel that convers black to transparent
            "lightAlpha"   : False,
            "ambient"      : {"map" : "", "color" :""}
        }

        try:
            """
            Notes:

            Get the PoserSurface node
            
            Note: we don't need to do anything with the Reflection inputs because
            those produce biased output. Lux takes care of the reflection 
            automatically, if the environment around the mode is set up correctly.
            """
            self.reMat["name"] = aMat.Name()
            # If this is an object the first entry is the PoserSurface node
            # If this is a light then the first entry is the Light node
            if ReTools.POSER_MAJOR < 11:
                poserSurface = aMat.ShaderTree().Node(0)
            else:
                st = aMat.ShaderTree()
                poserSurface = st.RendererRootNode(poser.kRenderEngineCodeFIREFLY)
                if not poserSurface:
                    poserSurface = st.RendererRootNode(poser.kRenderEngineCodeSUPERFLY)

            if poserSurface:
                for channel in poserSurface.Inputs():
                    chName = channel.InternalName()
                    # Reality.writeToLog("Channel: %s/%s" % (channel.Name(),channel.InternalName()))

                    # If the channel is from a Light it is labelled "Color"
                    if chName in ("Diffuse_Color","Color"):
                        self.reMat["diffuse"]["color"] = channel.Value()
                        diffTree = channel.InNode(1)
                        if diffTree:
                            self.reMat["diffuse"]["map"] = self.convertNode(diffTree)
                    elif chName == "Diffuse_Value":
                        self.reMat["diffuse"]["strength"] = channel.Value()
                        strengthNode = channel.InNode(1)
                        if strengthNode:
                            self.reMat["diffuse"]["strength map"] = self.convertNode(strengthNode)
                    elif chName == "Ambient_Value":
                        # If a material has both an Ambient value that is
                        # higher and than zero and an Ambient color that is
                        # different from black then we flag this material as
                        # being a light
                        self.reMat["lightGain"] = channel.Value()
                    elif chName == "Ambient_Color":
                        ambientColor = channel.Value()
                        if ambientColor != (0.0, 0.0, 0.0):
                            # Convert the material to a mesh light with no
                            # alpha and with its own light group
                            self.reMat["lightAlpha"] = False
                            self.reMat["ambient"]["color"] = ambientColor
                            ambTree = channel.InNode(1)
                            if ambTree:
                                self.reMat["ambient"]["map"] = self.convertNode(ambTree)
                    elif chName == "AlternateDiffuse":
                        aDiffTree = channel.InNode(1)
                        self.reMat["diffuse 2"]["color"] = channel.Value()
                        if aDiffTree:
                            self.reMat["diffuse 2"]["map"] = self.convertNode(aDiffTree)

                    elif chName == "Highlight_Color":
                        specGain = poserSurface.InputByInternalName("Highlight_Value").Value()
                        # The specular value input might be at zero. In that case we
                        # don't read the specular channel.
                        if specGain != 0:
                            self.reMat["specular"]["color"] = channel.Value()
                            specTree = channel.InNode(1)
                            if specTree:
                                self.reMat["specular"]["map"] = self.convertNode(specTree)

                    # If a specular value is present with a map connected we 
                    # convert the value to a corresponding grayscale color and 
                    # use this channel for defining the specular instead
                    elif chName == "Highlight_Value":
                        specTree = channel.InNode(1)
                        # self.reMat["specular"]["strength"] = channel.Value()
                        if specTree:
                            self.reMat["specular"]["map"] = self.convertNode(specTree)
                            specStrength = channel.Value()
                            self.reMat["specular"]["color"] = (specStrength, specStrength, specStrength)
                        else:
                            specStrength = channel.Value()
                            self.reMat["uRoughness"] = 1.0 - specStrength
                            self.reMat["vRoughness"] = 1.0 - specStrength

                    elif chName == "AlternateSpecular":
                        # The Alternate Specular channel wil only have a valid
                        # color if a node is connected to it. Otherwise the 
                        # color is underfined.
                        specTree = channel.InNode(1)
                        if specTree:
                            self.reMat["specular 2"]["color"] = channel.Value()
                            self.reMat["specular 2"]["map"] = ""
                            specMap = self.convertNode(specTree)
                            # Important test because the Blinn node conversion
                            # set the specular map outside this block
                            if specMap != "":
                                self.reMat["specular 2"]["map"] 
                            
                    elif chName == "Transparency_Max":
                        # We mean alpha as amount of opacity, Poser means amount of transparency
                        self.reMat["alpha"]["strength"] = 1.0 - channel.Value()
                        alphaTree = channel.InNode(1)
                        if alphaTree:
                            self.reMat["alpha"]["map"] = self.convertNode(alphaTree)
                            # When there is an alpha map the Transparency value is actually
                            # inverted in Poser. A value of 1.0 means full use of the 
                            # alpha map.
                            self.reMat["alpha"]["strength"] = channel.Value()

                    elif chName == "Translucence_Color":
                        self.reMat["translucence"]["color"] = channel.Value()
                        transTree = channel.InNode(1)
                        if transTree:
                            self.reMat["translucence"]["map"] = self.convertNode(transTree)

                    elif chName == "Translucence_Value":
                        self.reMat["translucence"]["strength"] = channel.Value()

                    elif chName == "Bump":
                        self.reMat["bump"]["map"] = ""
                        # Poser bump units are returned in inches, so we set the 
                        # positive/negative values for bump with a conversion
                        # to the metric units. Lux uses meters for unit so we 
                        # need to dived by 1000 the conversion from Poser. That's
                        # why we multiply by 0.0254 instead of 25.4
                        bumpValue = channel.Value() * 0.0254
                        # The strenght value is purely empirical, based on test done on several
                        # skins
                        self.reMat["bump"]["strength"] = 0.5
                        # Distribute the bump evenly between positive and negative
                        self.reMat["bump"]["neg"] = - (bumpValue/2)
                        self.reMat["bump"]["pos"] = bumpValue/2

                        bumpTree = channel.InNode(1)
                        if bumpTree:
                            self.reMat["bump"]["map"] = self.convertNode(bumpTree)

                    elif chName == "Displacement":
                        dispTree = channel.InNode(1)
                        if dispTree:
                            self.reMat["displacement"]["map"] = self.convertNode(dispTree)
                            self.reMat["displacement"]["strength"] = 1.0
                            # Poser disp units are returned in inches, so we set the 
                            # positive/negative values for disp with a conversion
                            # to the metric units. Lux uses meters for unit so we 
                            # need to dived by 1000 the conversion from Poser. That's
                            # why we multiply by 0.0254 instead of 25.4
                            dispValue = channel.Value() * 0.0254
                            # Poser's displacement maps use black for no displacement and
                            # pure white for full displacement
                            self.reMat["displacement"]["neg"] = 0
                            self.reMat["displacement"]["pos"] = dispValue
                    # Light intensity
                    elif chName == "Intensity":
                        # ReTools.debug("intens.: %f" % channel.Value())
                        self.reMat["intensity"] = channel.Value()
                    # Spotlight cone angle. Lux doesn't have a "start angle"
                    elif chName == "AngleEnd":
                        self.reMat["angle"] = channel.Value()

        except Exception, e:
            print "ReMaterial Exception. Exception type: %s, message: %s" % (type(e),e.message)

    ##
    # Returns true/false depending if the given channel (input) has a node
    # network attached to it
    def hasInputNode(self,channel):
        return channel.InNode(1) != None

    def convertNode(self,node):
        nodeType = node.Type()
        nodeID   = node.InternalName()
        if nodeID in self.reMat["nodes"]:
            return nodeID
        try:
            if nodeType == poser.kNodeTypeCodeIMAGEMAP:
                props = {"type" : ReTextureType_ImageMap}                
                imFileName = node.InputByInternalName("Image_Source").Value()
                if imFileName == "":
                    return "";
                props["fileName"] = imFileName
                uScale = node.InputByInternalName("U_Scale").Value()
                vScale = node.InputByInternalName("V_Scale").Value()
                if uScale == 0:
                    uScale = 1
                if vScale == 0:
                    vScale = 1
                props["u tile"]   = 1.0/uScale
                props["v tile"]   = 1.0/vScale
                props["u offset"] = node.InputByInternalName("U_Offset").Value()
                props["v offset"] = node.InputByInternalName("V_Offset").Value()
                props["gain"]     = 1.0 #node.InputByInternalName("Texture_Strength").Value()
                self.reMat["nodes"][nodeID] = props

            elif nodeType == poser.kNodeTypeCodeBLENDER:
                props = {"type" : ReTextureType_Mix}
                # Grab the value for the first color or texture
                texValues = self.getInputData(node,"Input_1")
                props["tex1 color"] = texValues["value"]
                props["tex1 map"]   = texValues["map"]

                # Grab the value for the second color or texture
                texValues = self.getInputData(node,"Input_2")
                props["tex2 color"] = texValues["value"]
                props["tex2 map"]   = texValues["map"]

                # Grab the value for the mix. This could be driven by a texture
                texValues = self.getInputData(node,"Blending")
                props["mix"] = texValues["value"]
                props["mix map"] = texValues["map"]

                # If the result of the parsing is to simply blend two colors
                # it's very likely that the nodes in input were not translatable 
                # and so the whole node should be ignored
                if props["tex1 map"] == "" and props["tex2 map"] == "":
                    return ""

                self.reMat["nodes"][nodeID] = props

            # The Scatter and Diffuse nodes are "Stop nodes". When we find them we don't just bypass it, we
            # need to stop processing whatever is upstream of it
            elif nodeType == poser.kNodeTypeCodeSCATTER or \
                 nodeType == poser.kNodeTypeCodeDIFFUSE :
                return ""

            # EdgeBlend nodes are ignored, Lux specular rendering makes them unnecessary
            # On the other hand they can link to other nodes of interest so we need to
            # scan the subtree
            elif nodeType == poser.kNodeTypeCodeEDGEBLEND:
                return self.scanUnsupportedNode(node, ["Attenuation"])

            elif nodeType == poser.kNodeTypeCodeCOLORMATH:
                # Query what kind of math function this is...
                function = node.InputByInternalName("Math_Argument").Value()
                # 1==Add, 2==Subtract, 3==Multiply
                if function >= 1.0 and function <= 3.0:
                    props = {"type" : ReTextureType_ColorMath}
                    props["value 1"] = {}
                    props["value 2"] = {}

                    # We need to test if the two values have shader trees
                    # attached to them originally. This let us know that the
                    # function is used to actually combine the results of the
                    # shader trees. The result of the conversion can be empty
                    # for each tree. In that case, if we had a input tree originally,
                    # the we need to discard the value for the input
                    v1Input = node.InputByInternalName("Value_1")
                    v1HasTree = v1Input.InNode(1) != None

                    v2Input = node.InputByInternalName("Value_2")
                    v2HasTree = v2Input.InNode(1) != None

                    v1 = self.getInputData(node,"Value_1")
                    v2 = self.getInputData(node,"Value_2")
                    props["value 1"]["color"] = v1["value"]
                    props["value 2"]["color"] = v2["value"]

                    props["value 1"]["map"] = v1["map"]
                    props["value 2"]["map"] = v2["map"]
                    if function == 1.0:
                        props["function"] = "a"
                    elif function == 2.0:
                        props["function"] = "s"
                    else:
                        props["function"] = "m"

                    # Sanity check
                    if v1HasTree and v1["map"] == "":
                        return ""
                    if v2HasTree and v2["map"] == "":
                        props["function"] = "m"                        
                        props["value 2"] = {}

                    self.reMat["nodes"][nodeID] = props
                else:
                    return ""


            elif nodeType == poser.kNodeTypeCodeMATH:
                # Query what kind of math function this is...
                function = node.InputByInternalName("Math_Argument").Value()
                # Multiply
                if function >= 1.0 and function <= 3.0:
                    props = {"type" : ReTextureType_Math}
                    props["value 1"] = {}
                    props["value 2"] = {}
                    v1 = self.getInputData(node,"Value_1")
                    v2 = self.getInputData(node,"Value_2")
                    props["value 1"]["value"] = v1["value"]
                    props["value 2"]["value"] = v2["value"]

                    props["value 1"]["map"] = v1["map"]
                    props["value 2"]["map"] = v2["map"]
                    if function == 1.0:
                        props["function"] = "a"
                    elif function == 2.0:
                        props["function"] = "s"
                    else:
                        props["function"] = "m"

                    self.reMat["nodes"][nodeID] = props
                else:
                    return ""
                    
            elif nodeType == poser.kNodeTypeCodeCOLORRAMP:
                props = {"type" : ReTextureType_Band}
                # How Poser interprets the slots in the ColorRamp node
                # The offsets are equally spaced
                props["offsets"] = (0,0.33,0.66,1.0)
                props["tex1"]    = node.InputByInternalName("Color1").Value()
                props["tex2"]    = node.InputByInternalName("Color2").Value()
                props["tex3"]    = node.InputByInternalName("Color3").Value()
                props["tex4"]    = node.InputByInternalName("Color4").Value()
                
                amountChannel = node.InputByInternalName("Input")
                props["amount"]  = amountChannel.Value()
                # Convert a map if it is available
                amountNode = amountChannel.InNode(1)
                if amountNode:
                    props["amount map"] = self.convertNode(amountNode)

                self.reMat["nodes"][nodeID] = props

            elif nodeType == poser.kNodeTypeCodeHAIR:
                # Hair nodes have two inputs: Root Color and Tip Color. 
                # We use those two colors as the first and last input for 
                # a Band texture
                props = {"type" : ReTextureType_Band}
                # The offsets are equally spaced
                props["offsets"] = (0,0.33,0.66,1.0)
                props["tex1"]    = node.InputByInternalName("Root_Color").Value()
                tipColor = node.InputByInternalName("Tip_Color").Value()
                props["tex2"]    = tipColor
                props["tex3"]    = tipColor
                props["tex4"]    = tipColor
                
                props["amount"]  = 1.0

                # Make sure that nothing is in the alternate specular channel
                # because the hair node has its own specular channel
                self.reMat["specular 2"]["map"] = ""
                self.reMat["specular 2"]["color"] = [0.0,0.0,0.0]
                
                specChannel = node.InputByInternalName("Specular_Color")
                self.reMat["specular"]["color"] = specChannel.Value()
                self.reMat["specular"]["map"] = ""
                specMap = specChannel.InNode(1)
                if specMap:
                    self.reMat["specular"]["map"] = self.convertNode(specMap)

                self.reMat["nodes"][nodeID] = props

            # elif nodeType == poser.kNodeTypeCodeDIFFUSE:
            #     props = {"type" : ReTextureType_Constant}
            #     props["value"] = node.InputByInternalName("Diffuse_Color").Value()
            #     props["gain"]  = node.InputByInternalName("Diffuse_Value").Value()
            #     self.reMat["nodes"][nodeID] = props

            elif nodeType == poser.kNodeTypeCodeSPOTS:
                props = {"type" : ReTextureType_DistortedNoise}
                bcValues = self.getInputData(node,"Base_Color")
                props["base color"] = bcValues["value"]
                props["base map"]   = bcValues["map"]
                    
                spotsValues = self.getInputData(node,"Spot_Color")
                props["spot map"] = spotsValues["map"]

                # The value 1.0 corresponds to "Original", otherwise it's "Improved"
                if node.InputByInternalName("Noise_Type").Value() == 1.0:
                    props["noise basis"] = "original perlin"
                else:
                    props["noise basis"] = "improved perlin"
                props["noise distortion"] = "improved perlin";
                # Empirical observation showed that dividing by 10 gets us closer to the
                # size of noise generated by Poser
                props["size"]       = node.InputByInternalName("Spot_Size").Value()/10
                props["contrast"]   = node.InputByInternalName("Softness").Value()
                props["brightness"] = node.InputByInternalName("Threshold").Value()
                # Constant obtained from empirical tests
                props["amount"] = 1.820  
                self.reMat["nodes"][nodeID]  = props

            elif nodeType == poser.kNodeTypeCodeNOISE:
                # The noise node in Poser has little resemblance with the
                # FBM texture in Lux and changes to the Poser's parameters
                # don't lead to much variantion more than some distortion 
                # along the axes. So, we simply plug some default values 
                # that will provide a similar noise pattern.
                props = {"type" : ReTextureType_FBM}
                # Texture is scaled to 3% because Poser's noise creates a
                # very fine noise grain
                props["scale"]     = 0.03
                props["octaves"]   = 4
                props["roughness"] = 1.5
                self.reMat["nodes"][nodeID]  = props

            elif nodeType == poser.kNodeTypeCodeCLOUDS:
                props = {"type" : ReTextureType_Clouds}
                skyValues = self.getInputData(node,"Sky_Color")
                props["sky color"]   = skyValues["value"]
                props["sky map"] = skyValues["map"]
                cloudsValues = self.getInputData(node,"Cloud_Color")
                props["cloud map"] = cloudsValues["map"]

                # The value 1.0 corresponds to "Original", otherwise it's "Improved"
                if node.InputByInternalName("Noise_Type").Value() == 1.0:
                    props["noise basis"] = "original perlin"
                else:
                    props["noise basis"] = "improved perlin"                    
                props["size"]       = node.InputByInternalName("Scale").Value()
                # props["brightness"] = node.InputByInternalName("Gain").Value()
                props["contrast"] = node.InputByInternalName("Gain").Value()
                # Depth in Lux is 0-based, in Poser it's 1-based
                props["depth"]      = node.InputByInternalName("Complexity").Value()-1
                # props["contrast"]   = node.InputByInternalName("Bias").Value()
                props["brightness"] = node.InputByInternalName("Bias").Value()
                props["style"]      = "soft" if node.InputByInternalName("Bottom").Value()\
                                                < 0.5 else "hard"
                self.reMat["nodes"][nodeID]  = props

            elif nodeType == poser.kNodeTypeCodeTURBULENCE:
                props = {"type" : ReTextureType_Clouds}

                # The value 1.0 corresponds to "Original", otherwise it's "Improved"
                if node.InputByInternalName("Noise_Type").Value() == 1.0:
                    props["noise basis"] = "original perlin"
                else:
                    props["noise basis"] = "improved perlin"
                props["size"]       = node.InputByInternalName("x_Index").Value()/10*3
                props["contrast"]   = node.InputByInternalName("Gain").Value()*2
                # Depth in Lux is 0-based, in Poser it's 1-based
                props["depth"]      = node.InputByInternalName("Octaves").Value()/3
                props["brightness"] = node.InputByInternalName("Bias").Value()+0.2
                props["style"]      = "hard"
                self.reMat["nodes"][nodeID]  = props

            elif nodeType == poser.kNodeTypeCodeMARBLE:
                """
                The Poser Marble node is nothing like marble and it translate closer
                to the LuxRender's wood texture
                """
                props = {"type" : ReTextureType_Wood}

                # The value 1.0 corresponds to "Original", otherwise it's "Improved"
                if node.InputByInternalName("Noise_Type").Value() == 1.0:
                    props["noise basis"] = "original perlin"
                else:
                    props["noise basis"] = "improved perlin"
                props["size"]       = node.InputByInternalName("Scale").Value()*0.25
                props["turbulence"] = node.InputByInternalName("Turbulence").Value()*6
                props["contrast"]   = 2
                props["brightness"] = 1
                props["soft noise"] = False
                props["pattern"]    = "BAND_NOISE"
                props["vein wave"]  = "SIN"
                self.reMat["nodes"][nodeID]  = props

            elif nodeType == poser.kNodeTypeCodeGRANITE:
                # Most of the following values have been obtained by direct experimentation
                # with the procedural shader. They are not conclusive by a long shot but they
                # provide a starting point.
                props = {"type" : ReTextureType_Clouds}

                # The value 1.0 corresponds to "Original", otherwise it's "Improved"
                if node.InputByInternalName("Noise_Type").Value() == 1.0:
                    props["noise basis"] = "original perlin"
                else:
                    props["noise basis"] = "improved perlin"
                props["size"]       = node.InputByInternalName("Scale").Value()/1000
                props["contrast"]   = 1.0
                # Depth in Lux is 0-based, in Poser it's 1-based
                props["depth"]      = node.InputByInternalName("Shades").Value()-1
                props["brightness"] = 1.3
                props["style"]      = "soft"
                self.reMat["nodes"][nodeID]  = props
        
            elif nodeType == poser.kNodeTypeCodeANISOTROPIC:
                """
                The Anisotropic node is used to communicate the use of Anisotropic values
                for the Glossy, Skin, and Metal materials. It does not translate into a
                Lux/SLG texture.
                """
                # props = {"type" : ReTextureType_Anisotropic}

                self.reMat["uRoughness"] = node.InputByInternalName("u_Roughness").Value()
                self.reMat["vRoughness"] = node.InputByInternalName("v_Roughness").Value()

                # Check if there is a texture in the Specular Value input
                specularMap = node.InputByInternalName("Specular_Value").InNode(1)
                if specularMap:
                    return self.convertNode(specularMap)

                return ""

            elif nodeType == poser.kNodeTypeCodeGLOSSY or \
                 nodeType == poser.kNodeTypeCodeSPECULAR :
                """
                The presence of a Glossy node bypasses the other nodes attached to 
                it before and feeds directly to the specular values for this material.
                """
                inpRoughness = node.InputByInternalName("Roughness")
                if inpRoughness:                    
                    self.reMat["uRoughness"] = inpRoughness.Value()
                    self.reMat["vRoughness"] = self.reMat["uRoughness"]
                # The Ks value is the strength of the specular color.
                if nodeType == poser.kNodeTypeCodeGLOSSY:
                    colorStrength = node.InputByInternalName("Ks").Value()
                else:
                    colorStrength = node.InputByInternalName("Specular_Value").Value()
                specCol = node.InputByInternalName("Specular_Color").Value()
                # We directly convert the combination of specular color and strength
                # to the correspondeng RGB value.
                specCol = (specCol[0]*colorStrength, specCol[1]*colorStrength, specCol[2]*colorStrength)
                self.reMat["specular"]["color"] = specCol
                return ""

            elif nodeType == poser.kNodeTypeCodeBLINN:
                """
                The presence of a Blinn node bypasses the other nodes attached to it before
                and feeds directly to the specular values for this material.
                """
                # The reflectivity value is the "shininess" of this node. For practical
                # purposes we consider tha limits to be 0.0-5.0 and we reduce it to be
                # a roughness value between 0.0 - 1.0
                refl = node.InputByInternalName("Reflectivity")
                reflRemapped = refl.Value() / 5.0
                self.reMat["uRoughness"] = 1.0 - reflRemapped
                self.reMat["vRoughness"] = self.reMat["uRoughness"]
                # Check if there is a reflectivity map
                reflMap = refl.InNode(1)
                if reflMap:
                    self.reMat["roughness map"] = self.convertNode(reflMap)

                specularMap = self.scanUnsupportedNode(node,[]) 
                if specularMap != "":
                    self.reMat["specular"]["map"] = specularMap # self.convertNode(specularMap)

                return ""

            elif nodeType == poser.kNodeTypeCodePHONG:
                """
                The presence of a Phong node bypasses the other nodes attached to 
                it before and feeds directly to the specular values for this material.
                """
                self.reMat["uRoughness"] = 1.0 - node.InputByInternalName("Specular_Value").Value()
                self.reMat["vRoughness"] = self.reMat["uRoughness"]
                specCol = node.InputByInternalName("Specular_Color").Value()
                # We directly convert the combination of specular color and strength
                # to the correspondeng RGB value.
                specCol = (specCol[0], specCol[1], specCol[2])
                self.reMat["specular"]["color"] = specCol
                return ""

            elif nodeType == poser.kNodeTypeCodeSKIN:
                """
                The presence of the skin node is used for signaling tha this material is
                of type skin. The node itself is not converted.
                """
                self.reMat["isSkin"] = True
                return self.scanUnsupportedNode(node,[])

            elif nodeType == poser.kNodeTypeCodeCOMP:
                # Query what channel we are extracting...
                props = {"type" : ReTextureType_Component   }
                props["channel"] = node.InputByInternalName("Component").Value()

                colorChannel = node.InputByInternalName("Color")
                props["color"]  = colorChannel.Value()
                # Convert a map if it is available
                colorNode = colorChannel.InNode(1)
                if colorNode:
                    props["map"] = self.convertNode(colorNode)
                self.reMat["nodes"][nodeID] = props
                
            else:
                """
                Unsupported nodes are skipped but we still look inside their inputs
                to see if there are nodes that we do support
                """
                # ReTools.debug("# Scanning unsupported node: " + node.Name())
                # for aInput in node.Inputs():
                #     subNode = aInput.InNode(1)
                #     if subNode:
                #         ReTools.debug("## Scanning input " + subNode.Name())
                #         aMap = self.convertNode(subNode)
                #         # As long as we found a supported node then we can stop
                #         # We can't work with multiple supported nodes because we
                #         # don't know how they should be mixed. One node, usually
                #         # an Image Map, is enough data recovery from a network that starts
                #         # with an unsupported node.
                #         if aMap != "":
                #             ReTools.debug("### Found node in unsupported tree: " + aMap)
                #             return aMap
                # return ""
                return self.scanUnsupportedNode(node, [])

        except Exception, e:
            print "Exception in convertNode(%s) for material %s: %s" % (repr(nodeType), self.reMat["name"], e.message)

        return nodeID

    def scanUnsupportedNode(self, node, ignoreInputs):
        """
        Scans a node that is not supported by Reality and continues the
        scanning for the inputs of this node. This allows Reality to find all the
        relevant data even when an unsupported node is in the middle of the chain.

        The ingoreInputs list cotains a list of inputs that don't need to be scanned.
        For example the "Attenuation" input of the EdgeBlend node is not something
        that is useful for the overall conversion.
        """
        for aInput in node.Inputs():
            # Skip the input that we don't want to consider
            if aInput.Name() in ignoreInputs:
                continue
            subNode = aInput.InNode(1)
            if subNode:
                aMap = self.convertNode(subNode)
                # As long as we found a supported node then we can stop
                # We can't work with multiple supported nodes because we
                # don't know how they should be mixed. One node, usually
                # an Image Map, is enough data recovery from a network that starts
                # with an unsupported node.
                if aMap != "":
                    return aMap
        return ""

    """
    Method: getInputData

    Returns the value stored into a node's input. If there is an input
    node attached to the input channel then the node is traversed and
    stored in the "map" portion of the returned dictionary
    """
    def getInputData(self,node,inputName):
        retVal = {"value" : None, "map" : ""}
        nodeInput = node.InputByInternalName(inputName)
        if nodeInput:
            retVal["value"] = nodeInput.Value()
            network = nodeInput.InNode(1)
            if network:
                retVal["map"] = self.convertNode(network)
        else:
            print "Error: %s is not a valid input for node %s" % (inputName,node.Name())
        return retVal

    """
    Method: getData

    Return the converted material in the form of a dictionary
    """
    def getData(self):
        return self.reMat
    

