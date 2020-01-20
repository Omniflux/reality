##Changes in Reality 3.1

###New features
- Added IBL preview sphere
- Added support for smartIBL (sIBL) format. Now all the maps form hdrlabs.com can be used with one click
- Added Gamma value for the IBL map. This is automatically set from the sIBL file, if one is used.
- Added binary file format for export of scenes. This is now the default. Scenes saved with 3.0 are automatically converted at load time.
- Added support for Poser subdivision for props. Subdivision for figures is not available at this time.
- Enhanced GPU rendering. Speed-up can be more than 100%.
- Added field to set how many GPUs are installed in the computer.
- Selecting "Pure GPU" automatically changes the Scene type to "Mono-directional" and enables texture collection at 512x512.
- Added selection of the OpenCL device in the Acceleration options.
- Added selection of level of verbosity of the Lux Log tab. See the drop down menu in the Renderer section.
- Added "Render as statue" option in the Render tab.
- Added option in the Advanced tab to set the default location for new scenes
- Added the **Simple Plane** prop, a plane that comes UV-Mapped and oriented in the right way. Assign an image map to it and it will be mapped as expected. The plane is just one polygons so it requires the least amount of resources possible.
- Added the **Round Water Plane** to make it easier to add water to round/oval containers
- Improved quality of the highlights and fine patterns. This feature requires LuxRender 1.3.
- Selecting an image map for a texture starts from the last folder browsed.
- Changed the verbosity of the log tab of Lux to default to warning and error messages.
- Changed default value for the Firefly removal from 5 to 3.
- New high-resolution thumbnails for all Reality lights and props
- Expanded error message for when Reality cannot load. It now includes a hint about matching the bitness of Poser. This is for the Windows version only, Mac OS has a universal binary.
- Added a number of fixes that make Reality able to run with localized versions of Poser.
- Now if a figure is flagged as invisible it will not render.
- Completely revised Reality User's Guide. The light section has been completely re-written and other sections have been expanded greatly to provide more information. There is a new chapter on how to edit textures in Reality with **step-by-step tutorials**. There are many more illustration and the guide now includes a hyper-linked Index.
- Renamed "Turbo mode" to "Pure GPU"
- Changed IBL handling to use the infinitesample method. It provides faster rendering and more accurate definition of highlights

##Bug fixes
- Fixed bug that stopped Reality from restoring the lights and camera information from the Poser scene file
- Added check if the path for LuxRender is not set Reality will show a message about it instead of silently failing to render.
- Fixed fonts to be compatible with OS X Mavericks
- Fixed behavior of "Ignore Infinite light"
- Done some minor adjustments to the Volumes panel to make it more clear.
- Fixed bug that crashed Reality when clicking on the "Hidden" checkbox in the material name if the material was not previous selected. This was happening only on Windows.
- Fixed bug about reparenting of objects. This requires Poser 10/2014 with SR2 installed
- Fixed bug about renaming a prop "RealityLight" and crashing Reality if editing the light's properties
- Fixed bug that prevented restoring the image file name for a light.
- Fixed bug that caused an error message when using File | Close in Poser
- Fixed Application icon in Windows
- Fixed bug that caused avoiding saving the Face Mask gain.
- Fixed bug that cause textures to render white. This happened when using a ColorMath texture with no second texture and pure white as a mixed color
- Fixed Color Math editor to accept only textures of the right kind
- Fixed a bug in the Color Math editor that cause the Texture B to be created Numeric instead of class Color
- Changing the location where LuxRender is installed now defaults to the latest location used. This makes it faster and easier to point to a new installation
- Improved logging and stability
- Provided a webpage for the download of LuxRender from the Prêt-à-3D site. This makes it easier to find the right version. Updated the documentation to reflect the change.
