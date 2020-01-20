###Lighting
- Added IBL Preview in Poser
- Added support for Smart IBL (sIBL) by [hdrlabs.com](http://hdrlabs.com)
- Added gamma value input for the IBL map. The value is automatically set from the sIBL file, if one is used
- Added support for the Ambient Value setting in Poser material. See the updated Modifiers tab

###Materials
- Improved conversion of materials to light to preserve more texture information
- Now converting a material to a light sets the light group to be equal to the material name

###User Interface
- Added binary file format export, which is now default. It's faster and produces smaller files.
- Faster export to LuxRender
- Added support for Poser subdivision. This feature requires Poser 10/Pro 2014 with SR3 installed
- Added selection of where to save a new scene instead of using the Documents folder by default
- Added control for the level of detail in the Log tab of LuxRender
- Added "Render as statue" option to render the whole scene as if it was made of white marble
- Added column in the material list that shows if a materials emits light
- The advanced option  "Ignore Infinite Light" now causes Reality to list the Infinite Lights but to automatically disable them
- Improved texture selector shown when selecting "Use Existing" from a texture gear menu. The selector now lists the name of the channel in a human-readable form
- Added automatic notification of Reality updates, if available.
- Selecting a new image map now remembers the last folder visited
- Now if a figure is flagged as invisible it will not render
- Renamed "Turbo mode" to "Pure GPU"

###Figure support
- Added support for Poser's Jessi
- Added support for Poser's James

###Documentation
- Vastly improved and expanded Reality User's Guide.  The Guide gained about 60 pages and it includes many more illustrations and video tutorials. The Guide also includes a fully interactive Index.

###Reality Add-ons
- Added the Simple Plane prop
- Added the Round Water Plane prop
- New high-resolution thumbnails for the Reality lights and props

###Rendering
- Added support for LuxRender 1.3
- Improved rendering of reflections and detailed patterns. (Blackman-Harris filter)
- Improved GPU rendering
- Added selection of the GPU devices to use

###Compatibility
- Fixed compatibility with translated versions of Poser
- Optimized Reality to run on Mac OS Mavericks
- Fixed issue with parenting/un-parenting objects (requires Poser 10/Pro 2014 with SR3)

###Bug fixes
- Fixed bug that stopped Reality from restoring the lights and camera information from the Poser scene file
- Fixed crash when clicking on the *Hidden* checkbox for material. This only affected the Windows version
- Fixed bug that prevented the gain for the hair mask from being restored
- Fixed bug in the handling of the Color Math texture
- Fixed bug that caused LuxRender to not start when rendering animations
- Fixed bug that prevented export of bump map for the Mirror material
