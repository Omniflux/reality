#include "ReTimedEditor.h"

using namespace Reality;

ReTimedEditor::TimerTable ReTimedEditor::timerTable;

void ReTimedEditor::initTimer( const int interval,
                               const QObject* target,
                               const char* member ) 
{
  timerTable.value(timerID)->setInterval(interval);
  timerTable.value(timerID)->setSingleShot(true);

  // Must use reinterpret_cast to work around the limit that this
  // class cannot be derived from QObject
  QObject::connect( 
    timerTable.value(timerID),
    SIGNAL(timeout()), 
    target, 
    member
  );
}
