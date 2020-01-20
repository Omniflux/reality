# This is the primary base class for Poser addons
# Your addon should inherit from this class.

class Addon:
    # The constructor. Fill in the addonInfo structure, but do not do
    # much beyond that. This may be called at a point in time where
    # Poser is not fully initialized yet and calls to the Poser Python API
    # may fail or crash Poser.
    def __init__(self):
        pass

    # Poser is fully initialized and your plugin is being loaded.
    # Fill in your data structures, build a GUI, do what your plugin does.
    def load(self):
        pass
        
    # Your plugin is unloaded from Poser, clean up your data, close open files,
    # close your custom GUI. Please leave no trace.
    def unload(self):
        pass
       
    # addons must fill this dictionary
    addonInfo = { 'id' : 'com.smithmicro.addon', # set this to a unique ID
    'author' : 'Smith Micro, Inc', # author info
    'copyright' : '(c) 2012 Smith Micro, Inc',
    'name' : 'AddOn', # this is the name of your plugin that will be visible in the GUI
    'version' : '1.0',
    'observer' : 0, # set to 1 if your addon implements the methods from SceneObserver
    'scenedata' : 0, # set to 1 if your addon implements the methods from SceneDataSaver
    'prefsdata' : 0, # set to 1 if your addon implements the methods from PrefsSaver
     }

# These methods will not get called immediately when things happen
# They get called when the program flow is finished with its work and
# returns to regular event processing
class SceneObserver:
    def objectsAdded(self, objects):
        pass
    def objectsDeleted(self, objects):
        pass    
    def objectsRenamed(self, objectsOldNames):
        pass
    def objectSelected(self, selection):
        pass
    def sceneDeleting(self, scene):
        pass
    def sceneCreated(self, scene):
        pass
    def frameChanged(self):
        pass
        
class SceneDataSaver:
    # This method will be called when a Poser scene is saved.
    # Your addon should return a dictionary that can be serialized.
    # Do not make any assumptions about how and where your dictionary is stored.
    # It may be in the PZ3, it may be outside of the PZ3,
    # it may be as ASCII text or as binary. Under no circumstances
    # try to read, write or modify the addon data stored in a PZ3 directly.
    def saveData(self):
        return None
        
    # This method will be called when a Poser scene is loaded
    # that contains data saved by your addon. You will receive
    # the dictionary that your addon passed into saveData() when
    # the scene was saved.
    def loadData(self, data):
        pass

class PrefsSaver:
    # This method is called when Poser asks your addon to save its preferences.
    # Typically this happens when Poser exits or when an addon explicitly asked
    # for the preferences to be saved.
    # Return a dictionary that can be serialized.
    def savePrefs(self):
        return None
    
    # This method will be called after Poser has launched and the addons are loaded.
    # You will receive the dictionary that your addon passed to savePrefs() when
    # your addon was asked to save the prefs.
    def loadData(self, data):
        pass
