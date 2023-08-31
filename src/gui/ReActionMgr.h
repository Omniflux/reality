/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef RE_ACTION_MGR_H
#define RE_ACTION_MGR_H

#include <QAction>


/**
 * The ReActionManager class defines a collection of QActions that are stored
 * and can be retrieved and attached to different UI elements in Reality.
 */
class ReActionManager {
protected:
  //! We define the action dictionary as a QHash that holds a pointer to an
  //! action, which is identified by a string key.
  typedef QHash<QString, QAction*> ReActionDictionary;

  //! The list of actions stored
  ReActionDictionary actions;

  //! Automatic cleanup system. This struct deleted the instance of the ReActionmanager
  //! A static instance of this struct is declared in the getInstance() method.
  //! When the instance goes out of scope it deletes the ReActionManager.
  struct cleanup {    
    ~cleanup() {
      auto self = ReActionManager::getInstance(); 
      if (self) {
        delete self;
        self = NULL;
      }; 
    }
  };

private:
  //! Constructor: ReActionManager
  ReActionManager() {
  };

public:
  //! Destructor: ReActionManager
  virtual ~ReActionManager() {
  };

  inline void storeAction( const QString& actionID, QAction *const action ) {
    actions[actionID] = action;
  }
  
  inline QAction* newAction( const QString& id, const QString& label, QObject* parent ) {
    auto action = new QAction( label, parent );
    storeAction(id, action);
    return action;
  }

  inline QAction* getAction( const QString& actionID ) const {
    if (actions.contains(actionID)) {
      return actions.value(actionID);
    }
    return NULL;
  }

  static ReActionManager* getInstance() {
    // The singleton instance. 
    static ReActionManager* instance = NULL;
    
    // The object responsible for deleting the instance when the program ends.
    static cleanup destroyer;

    if (instance == NULL) {
      instance = new ReActionManager();
    }
    return instance;
  }
};

#endif
