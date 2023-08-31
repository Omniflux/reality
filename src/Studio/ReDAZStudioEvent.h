/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef RE_DS_EVENT_H
#define RE_DS_EVENT_H

#include <boost/any.hpp>
#include <QEvent>
#include <QHash>


namespace Reality {

/**
 * Studio's signals happen at the beginning of the event and not at the end.
 * For example, the signal that an object has been added to the scene happens
 * when the object is added to the scene, before it's populated and initialized
 * with all the data, including materials and subnodes. 
 * 
 * For Reality we need to process objects and other events after Studio is done,
 * not before. To solve this issue we create a special event class and push it at
 * the end of the event queue. The custom event() method in the Reality plugin
 * then process this event object and executes the expected methods based on 
 * the information stored in the event object.
 *
 */

class ReStudioEvent : public QEvent {
public:
  typedef QHash<QString, boost::any> EventProperties;

private:
  //! Flag used to communicate if the processing of the event should 
  //! close the queue as well. This is interpreted by the event handler
  //! to set the isSceneLoading flag to false
  bool closeEventQueue;

  //! Description of what kind of Reality event this is. The description
  //! is used by Reality to decide what method to call to handle the event
  QString eventDesc;

  EventProperties properties;

public:
  //! User-defined identifier of the event in the hierarchy of Qt events
  static int reEventType;

  //! Constructor: ReStudioEvent
  ReStudioEvent( const QString& eventDesc, 
                 const bool closeEventQueue = false) : 
    QEvent( QEvent::Type( reEventType ) ),
    closeEventQueue(closeEventQueue),
    eventDesc(eventDesc)
  {
    // nothing
  };
  
  inline bool shouldCloseEventQueue() {
    return closeEventQueue;
  }

  inline QString& getDescription() {
    return eventDesc;
  }

  inline void addProperty( const QString& propName, boost::any& propVal ) {
    properties[propName] = propVal;
  }

  inline boost::any& getProperty( const QString& propName ) {
    return properties[propName];
  }

};

} // namespace

#endif
