/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */
#ifndef RE_CSS_EDITOR_H
#define RE_CSS_EDITOR_H

#include <QDialog>
#include <QFile>
#include "ui_reCssEditor.h"
#include "ReMainWindow.h"

namespace Reality {

/**
 * Dialog box to do live CSS editing
 */
class ReCssEditor : public QDialog, public Ui::reCssEditor 
{
  Q_OBJECT
private:

public:
  //! Constructor
  ReCssEditor( QWidget* parent = 0 ) : QDialog(parent) {
    setupUi(this);
    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(14);    
    teCssEditor->setFont(font);

    connect(btnSave, SIGNAL(clicked()), this, SLOT(saveCssCode()));
    connect(searchText, SIGNAL(textChanged(const QString&)), this, SLOT(doSearchText(const QString&)));
  }
protected:

  void showEvent( QShowEvent* event ) {
    teCssEditor->setPlainText(ReMainWindow::getRealityMainWindow()->styleSheet());
  }

private slots:
  void saveCssCode() {
    ReMainWindow::getRealityMainWindow()->setStyleSheet(teCssEditor->toPlainText());
  };

  void doSearchText(const QString& x) {
    QString s = searchText->text();
    if (!s.isEmpty()) {
      QTextCursor cursor;
      cursor.setPosition(0);
      teCssEditor->setTextCursor(cursor);
      teCssEditor->find(s);
    }
  }
};

} // namespace

#endif
