Reality update of 4/25/13 - Build 3.0.1.193

To check if you need this update please look in the About box of Reality, available from the program's menu. If the version shown is 3.0.1.193 then you don't need to update. If the version number shown is different then you need to update your installation of Reality.

This update fixes several issues that were found in release 3.0.0. Specifically:

- Updated the exporter to handle some object that caused the Reality exporter to abort with a message in the "Main Wacros" window of Poser. 
- Fixed export of parented objects
- Fixed crash when converting Ramp nodes (Band texture in Lux)
- Added support for Genesis.
- When editing materials or other information in Reality the Poser scene is now flagged as "in need to be saved". 
- When deleting some objects in the Poser scene Reality did not remove the corresponding entry in its database. This is working now.
- Fixed exception when entering the Setup room with objects in the scene.
- Changed saving of of image map file to be more robust. 
- Improved handling of situations where the user has no write rights for the scene file.
- Changed export to set light strategy of "one" when using the GPU.
- Fixed errors when applying some materials (mt5). 
- Fixed alpha channel use for mesh lights. Previously mesh lights always had an alpha channel regardless of what was selected in the light editor.
- Fixed range for the IBL Y-axis rotate parameter.
- Increased maximum limit for 2D texture tiling.
- Stability fixes.

To update your copy of Reality do as follows:

- Re-download Reality from your account page
- Make sure that Poser and Reality are closed
- Delete the Reality folder inside the Runtime/Python/addons folder
- Re-install Reality as described in the Reality User's Guide using the newly dowloaded archive.
