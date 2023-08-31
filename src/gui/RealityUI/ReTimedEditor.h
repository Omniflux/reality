/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_TIMED_EDITOR_H
#define RE_TIMED_EDITOR_H

#include <QHash>
#include <QTimer>


namespace Reality {

/**
  Most editors in Reality use a material preview. To avoid overloading
  the previewer we want to start a new preview only if the user is
  done with the edits. To do this we start a new timer at the beginning
  of each edit and, whent the timer expires, we kick off a new preview.

  This class is really more like an dinterface. It behaves like a 
  QObject-derived class with the exception that it cannot be a QObject
  subclass because this class is used in a multiple-inheritance context
  and only one QOBject subclass is allowed by Qt. For this reasons it's
  crucial to make sure that this interface is not used by classes that are
  not derived from QObject.

 */
class ReTimedEditor {

protected:
  //! This type defines a table that is keyed by the name of the class 
  //! and that contains the timer for thatclass
  typedef QHash<QString, QTimer*> TimerTable;

  //! Table of all the timers in use
  static TimerTable timerTable;

  //! ID of the timer used by the subclass
  QString timerID;

public:
  //! Constructor
  //! Each class that subclasses this interface must provide an identifier
  //! for its timer. The timer can be shared. All materials editors use
  //! the same timer, for example. All texture editors use a shared timer
  //! which is different from the material editors timer.
  ReTimedEditor( const QString& timerID ) : timerID(timerID) {
    if (!timerTable.contains(timerID)) {
      timerTable[timerID] = new QTimer();
    }
  };

  void initTimer( const int interval, const QObject* target, const char* member );

  inline void resetTimer() {
    timerTable.value(timerID)->stop();
    timerTable.value(timerID)->start();
  }

  inline void stopTimer() {
    timerTable.value(timerID)->stop();
  }

  inline void removeTimer() {
    timerTable.value(timerID)->disconnect();
  }

};

} // namespace


#endif
