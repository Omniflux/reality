/*
 *  RegistrationDialog.h
 *  ReGeomExport
 *
 *  Created by Paolo H Ciccone on 11/10/10.
 *  Copyright 2010-2012 Pret-A-3D. All rights reserved.
 *
 */

#ifndef REGISTRATION_DIALOG_H
#define REGISTRATION_DIALOG_H

#include <QDialog>
#include <QHttp>
#include "ui_reProdRegDialog.h"

// Fwd declaration
class ReLuxManager;

class RegistrationDialog: public QDialog, public Ui::reProdRegDialog {
  Q_OBJECT

  public:
    RegistrationDialog(QWidget* parent = 0);

    void setVerifier(ReLuxManager* _verifier) {
      verifier = _verifier;
    };
  
  private slots:
    void activateOffLine(bool onOff);  
    void sendRegistration();
    void registrationCompleted(int requestID, bool error );
    void registerOffLine();
    void enableButtons();
    
  private:
    QHttp http;
    int requestID;
    ReLuxManager* verifier;

    //! Previous Serial number in case of upgrade
    QString oldSN;
  
    bool processRegistrationData(QByteArray& response);
    bool formIsValid();
};

#endif
