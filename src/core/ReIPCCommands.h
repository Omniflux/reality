/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef RE_IPC_COMMANDS_H
#define RE_IPC_COMMANDS_H

namespace Reality {

/**
 * Messages used for IPC between the Reality UI and the plugin running on the
 * host side.
 */
enum IPC_MESSAGES {
  // GUI
  GUI_CONNECTED,
  GUI_DISCONNECTED,
  // Sent by RealityRunner to cause the running GUI to
  // come to the foreground
  GUI_BRING_TO_FRONT,
  //! Instructs the GUI to stop updating the material preview
  //! This operation might be necessary when updating multiple 
  //! materials from a preset.
  GUI_PAUSE_MAT_PREVIEW,
  //! Resume the updating of the material preview
  GUI_RESUME_MAT_PREVIEW,

  // Objects
  GET_NUM_OBJECTS,
  GET_OBJECT,
  GET_OBJECT_LIGHTS,
  GET_OBJECTS,  
  GET_OBJECT_NAMES,
  HOST_OBJECT_ADDED,
  HOST_OBJECT_DELETED,
  HOST_OBJECT_RENAMED,
  HOST_OBJECT_ID_RENAMED,
  HOST_OBJECT_VISIBILITY_CHANGED,

  // Volumes
  GET_VOLUMES,
  VOLUME_ADDED,
  VOLUME_RENAMED,
  VOLUME_DELETED,
  VOLUME_UPDATED,   // 20

  // Materials
  GET_MATERIAL,
  HOST_MATERIAL_UPDATED,
  HOST_MATERIAL_SELECTED,
  //! Send by the host to the GUI when the requested material
  //! type conversion has been completed.
  HOST_MATERIAL_TYPE_CHANGED,

  //! When the user selects Copy/Paste the GUI applies the
  //! transformation and then sends the clipboard data to the
  //! host-app to perform the same operation
  MATERIAL_IMPORT_FROMCLIPBOARD_DATA,

  //! Message sent from the UI to the host-app to 
  //! indicate that the user has created a new texture
  //! for a material channel and that texture needs to be
  //! replicated in the host-side.
  MAKE_NEW_TEXTURE,

  //! Message sent from the UI to the host-app to 
  //! indicate that the user has created a new texture
  //! for a light and that texture needs to be
  //! replicated in the host-side.
  MAKE_NEW_SUB_TEXTURE,

  //! Message sent by the UI to the host-app to 
  //! indicated that an existing texture has been linked
  //! to a channel of the material
  LINK_TEXTURE,

  //! Message sent by the UI to the host-app to 
  //! indicated that an existing texture has been linked
  //! to a master texture. This is usually an event 
  //! triggered by the texture editor
  LINK_SUB_TEXTURE,

  //! Message sent by the UI to the host-app to 
  //! indicated that an existing texture has been unlinked
  //! from a channel of the material
  UNLINK_TEXTURE,

  //! Message sent by the UI to the host-app to 
  //! indicated that an existing sub-texture has been unlinked
  //! from a channel of the containing texture
  UNLINK_SUB_TEXTURE,

  //! This message cause a texture to be created from another texture
  //! and linked to a channel. This is used when synchronizing materials
  //! together. The texture is a duplicate of the texture from the same
  //! channel but from another material.
  SYNCHRONIZE_CHANNEL_TEXTURE,

  //! Message sent by the UI when a material has been edited
  UI_MATERIAL_EDITED,

  //! Sent when a material needs to be converted to a 
  //! different type. For example, from Matte to Glossy
  CHANGE_MATERIAL_TYPE,

  //! Sent when a texture type needs to be change to a new
  //! type, like when converting a Color constant to a Image Map
  CHANGE_TEXTURE_TYPE,

  //! Used to signal the host-side to apply a shader to a material.
  //! This can cause a material to change type.
  APPLY_ACSEL_SHADER,
  
  //! Used to signal the host-side to apply a universal shader to a material.
  //! This can cause a material to change type.
  APPLY_UNIVERSAL_SHADER,

  //! Revert the material to the original shader archived when it was created
  //! This is called by the user after a change or conversion to a different 
  //! type needs to be undone
  REVERT_SHADER_TO_ORIGINAL,

  // Lights
  GET_NUM_LIGHTS,
  GET_LIGHTS,
  HOST_LIGHT_ADDED,
  HOST_LIGHT_DELETED,
  HOST_LIGHTS_REMOVE_ALL,
  HOST_LIGHT_RENAMED,
  HOST_LIGHT_TYPE_CHANGED,  
  HOST_LIGHT_UPDATED,
  UI_LIGHT_EDITED,

  // Cameras
  GET_NUM_CAMERAS,
  GET_CAMERAS,
  HOST_CAMERA_ADDED,
  HOST_CAMERA_RENAMED,
  HOST_CAMERA_CHANGED,
  HOST_CAMERA_DELETED,
  //! The active camera in the host has changed and the UI needs to synchronize
  HOST_CAMERA_SELECTED,
  //! The UI selected a camera and the host needs to synchronize
  UI_CAMERA_SELECTED,
  UI_CAMERA_EDITED,
  REFRESH_CAMERA_LIST,

  // Commands
  RENDER_FRAME,
  RENDER_ANIMATION,
  SAVE_SCENE,
  SET_IBL_PREVIEW,
  
  //! Causes all the settings to be dropped and the scene to be reloaded
  REFRESH_SCENE,

  // 
  SET_NEW_SCENE,

  //! Used to sync the data in the Output tab from the
  //! GUI to the host-app side or the other way around. 
  //! The command is sent by the GUI side when the options
  //! are changed in the GUI and it's sent by the host-app
  //! side when a scene is loaded by the user.
  SYNC_OUTPUT_DATA,

  //! Used by the GUI to ask for the OUTPUT data of the scene.
  //! This message is sent normally when the GUI comes up and
  //! it needs to retrieve the Output data from the host-app
  //! side. The data is set from either the application's 
  //! defaults or the saved scene that has been loaded by the
  //! user.
  GET_OUTPUT_DATA,      

  HOST_FRAME_SIZE_UPDATED,

  //! Sent by the host-app when it's closing, used to close the Reality GUI
  //! if it's running in that moment.
  CLOSE_REALITY,

  //! Export progress messages. Sent by the exporter to the GUI
  EXP_EXPORT_STARTED,
  EXP_EXPORTING_OBJECT,
  EXP_EXPORT_FINISHED,

  //! Used to alert the user that the dimensions of the render are not set
  RENDER_DIMENSIONS_NOT_SET,

  //! Used by the GUI to retrieve the list of library paths obtained from the
  //! host app.
  GET_LIBRARY_PATHS,

  //! The server side sometimes needs to check if the GUI is around
  //! the PING message does that
  PING,

  //! The host-app has loaded a scene
  HOST_SCENE_LOADED,

  //! Select the material in the host app
  SELECT_MATERIAL_IN_HOST,

  //! Replace a texture with data in JSON format, usually from a copy/paste operation
  REPLACE_TEXTURE,

  UPDATE_ANIMATION_LIMITS

};

} // namespace

#endif
