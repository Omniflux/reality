/*
 *  RegistrationDialog.h
 *  ReGeomExport
 *
 *  Created by Paolo H Ciccone on 11/10/10.
 *  Copyright 2010 Pret-A-3D. All rights reserved.
 *
 */

#include <QtGui>
#include <QRegExpValidator>
#include <QCryptographicHash>

#include "RegistrationDialog.h"
#include "ReDRM.h"
#include "blowfish.h"
#include "RealityBase.h"
#include "ReLogger.h"

// #define RE_REGISTRATION_URL "http://www.preta3d.com/%1/%2"
// #define RE_REGISTRATION_TASKS_DIR "tasks"
// #define RE_REGISTRATION_SCRIPT_NAME "Reality2Registration.php"

//! The position, in the serial number, that holds the character that 
//! identifies the seller of the license. 
//!  - P => "Pret-A-3"
//!  - R => "Renderosity"
//!  - D => "RuntimeDNA" 
//!  - Y => "YURDigital"
//!  - S => "Smith Micro"
//!  - H => "Hivewire"
//!  - Z => "Daz"

#define RE_SELLER_IDENTIFIER_POS 4

using namespace Reality;

// extern bool RealityIsRegistered;

// Length of a serial number
#define RE_SN_MAX_LENGTH 37

// Returns the product code for the running version of Reality
QString getProductCode() {
  QString prodCode;
  switch( RealityBase::getRealityBase()->getHostAppID() ) {
    case Poser:
      prodCode = decryptString(REALITY_PROD_CODE_PS);
      break;
    case DAZStudio:
      prodCode = decryptString(REALITY_PROD_CODE_DS);
      break;
    case RealityPro:
      prodCode = decryptString(REALITY_PROD_CODE_PRO);
      break;
    default:
      prodCode = "----";      
  }
  return prodCode;
}

class SNValidator: public QValidator {
private:
  QRegExp re;
  QString productCode;
public:
  SNValidator(QObject* parent) : 
    QValidator(parent),
    re(decryptString(SN_VALIDATOR))
  {
    productCode = getProductCode();
  }

  QValidator::State validate( QString& input, int& pos ) const {
    input = input.toUpper().simplified();
    if (pos == 1 && input[0] != 'R') {
      return Invalid;
    }
    if (pos == 2 && input[1] != '4') {
      return Invalid;
    }
    // Verify that the user is entering a valid serial for the 
    // DS edition
    if ( pos == 3 ) {
      if ( (productCode == decryptString(REALITY_PROD_CODE_DS)) &&
           (input[2] != 'D')  )  {
        return Invalid;
      }
      // Verify that the user is entering a valid serial for the 
      // Poser edition
      if ( (productCode == decryptString(REALITY_PROD_CODE_PS)) &&
           (input[2] != 'P')  )  {
        return Invalid;
      }
    }
    // if (pos == 3 && input[2] != '3') {
    //   return Invalid;
    // }
    if (re.exactMatch(input)) {
      return Acceptable;
    }
    return Intermediate;
  }
};

RegistrationDialog::RegistrationDialog(QWidget* parent) : QDialog(parent) {
  RealityIsRegistered = false;
  setupUi(this);
  // First and last name validator
  auto nameValidator = new QRegExpValidator(QRegExp("^.{2,}"));
  liFirstName->setValidator(nameValidator);
  liLastName->setValidator(nameValidator);
  // Email generic validation
  liEmail->setValidator(
    new QRegExpValidator(
          QRegExp("^(([A-Za-z0-9]+_+)|([A-Za-z0-9]+\\-+)|([A-Za-z0-9]+\\.+)|([A-Za-z0-9]+\\++))*[A-Za-z0-9]+@((\\w+\\-+)|(\\w+\\.))*\\w{1,63}\\.[a-zA-Z]{2,6}$"),
          liEmail
        )
  );
  liOrderNo->setValidator(
    // Match an order number that is one of:
    //   - Two digits follow by a dash followed by several digits 
    //     (Prêt-à-3D's order number)
    //   - A series of at least 4 digits (Renderosity, RuntimeDNA etc)
    //   - The Smith Micro humongous order number
    new QRegExpValidator(QRegExp("^\\d{2}-\\d{7}$|^\\d{4,}$|^SMI\\d{6}-\\d{4}-\\d{5}$"),liOrderNo)
  );
  liSerialNo->setMaxLength(RE_SN_MAX_LENGTH);
  liSerialNo->setValidator(
    new SNValidator(liSerialNo)
  );
  pages->setCurrentWidget(pageMain);
  btnRegOnLine->setEnabled(false);
  btnRegOffLine->setEnabled(false);
  connect(btnRegOffLine, SIGNAL(toggled(bool)), this, SLOT(activateOffLine(bool)));
  connect(btnRegOnLine, SIGNAL(clicked()), this, SLOT(sendRegistration()));
  connect(btnSaveKey, SIGNAL(clicked()), this, SLOT(registerOffLine()));
  connect(liSerialNo, SIGNAL(textChanged(const QString&)), this, SLOT(enableButtons()));
  setFocus(Qt::ActiveWindowFocusReason);
  
  setWindowState( (windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
  #ifdef __APPLE__
  raise();  
  #elif defined(WIN32)
  activateWindow(); 
  #endif
};

void RegistrationDialog::enableButtons() {
  if ( liSerialNo->hasAcceptableInput() ) {
    lblDisabledButtons->hide();
    btnRegOnLine->setEnabled(true);
    btnRegOffLine->setEnabled(true);
  }
  else {
    lblDisabledButtons->show();
    btnRegOnLine->setEnabled(false);
    btnRegOffLine->setEnabled(false);
  }
}

/**
 * Send the registration data to the registrration server
 */
void RegistrationDialog::sendRegistration() {
  if (!formIsValid()) {
    return;
  }
  btnRegOnLine->setEnabled(false);
  btnRegOffLine->setEnabled(false);
  
  RealityBase* realityBase = RealityBase::getRealityBase();

  QString prodCode = getProductCode();

  QString data = QString(decryptString(VALIDATION_SERVER_STRING))
                   .arg(liFirstName->text())
                   .arg(liLastName->text())
                   .arg(liOrderNo->text())
                   .arg(liSerialNo->text().replace("-",""))
                   .arg(liEmail->text())
                   .arg(prodCode)
                   .arg(decryptString(APP_CODE_NAME))
                   .arg(cbSendEmail->isChecked() ? 'y' : 'n')
                   .arg(realityBase->getOSType() == MAC_OS ? "M" : "W");

  if (!oldSN.isEmpty()) {
    data += "&" + QString(decryptString(PREVIOUS_SN_QUERY))
                   .arg(oldSN);
  }

  connect(&http,SIGNAL(requestFinished(int,bool)),this,SLOT(registrationCompleted(int,bool)));
  QUrl url(QString("%1?%2")
             .arg( decryptString(RE_REGISTRATION_URL)
                      .arg(decryptString(RE_REGISTRATION_TASKS_DIR))
                      .arg(decryptString(RE_REGISTRATION_SCRIPT_NAME))
                  )
             .arg(data));
  http.setHost(url.host(),url.port(80));
  requestID = http.get(QString("%1?%2")
                         .arg(url.path())
                         .arg(url.encodedQuery().data()));
  http.close();
};

void RegistrationDialog::registrationCompleted(int requestID, bool error ) {
  if (this->requestID != requestID) {
    return;
  }
  
  QByteArray response = http.readAll();
  if ( !processRegistrationData(response) ) {
    return;
  }
  // Signal the verifier to clear the anti-tampering flag
  verifier->clear();
};

void RegistrationDialog::registerOffLine() {
  QByteArray response(liOffLineKey->text().simplified().toAscii());
  response.prepend("OK  ");
  if (!formIsValid()) {
    pages->setCurrentWidget(pageMain);
    return;
  }
  if ( !processRegistrationData(response) ) {
    return;
  }
  // Signal the verifier to clear the anti-tampering flag
  verifier->clear();
};


bool RegistrationDialog::processRegistrationData( QByteArray& response ) {
  // The first 4 characters in the server data include the "OK" or error code
  if (!response.startsWith(decryptString(IS_OK).toAscii())) {
    pages->setCurrentWidget(pageError);
    lblErrorCode->setText(response.data());
    return(false);
  }
  
  response.remove(0, 4);
  QString userData = QString("%1,%2,%3,%4,%5")
                     .arg(liEmail->text())
                     .arg(liFirstName->text())
                     .arg(liLastName->text())
                     .arg(liOrderNo->text())
                     .arg(liSerialNo->text().replace("-",""));
  
  QByteArray hashString(userData.toUtf8());
  hashString.prepend( QString(decryptString(preUserInfo) + ",").toAscii() );
  hashString.append("," + decryptString(postUserInfo));
    
  QByteArray hashedData = QCryptographicHash::hash(
                            hashString, QCryptographicHash::Sha1
                          )
                          .toHex();
  if (response != hashedData) {
    lblErrorMsg->setText(tr(decryptString(ERR_WRONG_LICENSE).toAscii()));
    lblErrorCode->setText(tr(decryptString(N_A).toAscii()));
    pages->setCurrentWidget(pageError);
    return(false);
  }
  Blowfish cipher;
  QByteArray clearKey(decryptString(registrationKey).toAscii());
  QByteArray encData(userData.toUtf8());
  // encData.append(",activated");
  encData.append("," + decryptString(encDataSuffix));
  
  // Check if the buffer length is a multiple of 8
  // if not, extend it and zero the padding. This is
  // a requirement of BlowFish
  int eightMultiples = encData.length() % 8;
  int dataLength = encData.length();
  if (eightMultiples > 0) {    
    encData.resize(dataLength + 8 - eightMultiples);
    for (int i=dataLength; i<encData.length(); i++) {
      encData[i] = '_';
    }
  }
  
  cipher.Set_Passwd( clearKey.data() );    
  cipher.Encrypt(((unsigned char*) encData.data()), encData.size());
  
  ReConfigurationPtr config = RealityBase::getConfiguration();
  config->beginGroup(decryptString(RE_REGISTRATION_ROOT));
  switch (RealityBase::getRealityBase()->getHostAppID()) {
    case Poser:
      config->setValue(decryptString(RE_REGISTRATION_KEY_PS), encData.toBase64());
      break;
    case DAZStudio:
      config->setValue(decryptString(RE_REGISTRATION_KEY_DS), encData.toBase64());
      break;
  }
  config->endGroup();
  config->sync();

  RealityIsRegistered = true;
  pages->setCurrentWidget(pageCongrats);
  btnCancel->setText(decryptString(IS_OK));
  return(true);
}

void RegistrationDialog::activateOffLine(bool onOff) {
  if (onOff) {
    pages->setCurrentWidget(pageOffLine);
    btnRegOnLine->setEnabled(false);
  }
  else {
    pages->setCurrentWidget(pageMain);
    btnRegOnLine->setEnabled(true);
  }
};

bool RegistrationDialog::formIsValid() {
  liFirstName->setText(liFirstName->text().simplified());
  if (!liFirstName->hasAcceptableInput() || liFirstName->text() == "") {
    QMessageBox::warning(
      this,tr("Information"),tr("You must provide a valid first name")
    );
    return(false);
  }
  liLastName->setText(liLastName->text().simplified());
  if (!liLastName->hasAcceptableInput() || liLastName->text() == "") {
    QMessageBox::warning(
      this,tr("Information"),tr("You must provide a valid last name")
    );
    return(false);
  }
  liOrderNo->setText(liOrderNo->text().simplified());
  if ( !liOrderNo->hasAcceptableInput() || liOrderNo->text() == "") {
    QMessageBox::warning(
      this,tr("Information"),tr("You must provide your order number")
    );
    return(false);
  }
  liSerialNo->setText(liSerialNo->text().simplified());
  if (liSerialNo->text() == "") {
    QMessageBox::warning(this,
      tr("Information"),tr("You must provide the serial number for the product")
    );
    return(false);
  }
  liEmail->setText(liEmail->text().simplified());
  if ( !liEmail->hasAcceptableInput() || liEmail->text() == "") {
    QMessageBox::warning(
      this,tr("Information"),tr("You must provide a valid email address")
    );
    return(false);
  }
  auto sn = liSerialNo->text();
  if (sn.startsWith("R4PU") || sn.startsWith("R4DU")) {
    oldSN = QInputDialog::getText(
              this, 
              tr("Previous Serial number"), 
              tr("The serial number entered is for the upgrade version of "
                 "Reality.\n"
                 "Please enter the serial number of your previous version "
                 "of Reality")
            );
  }
  return(true);
}

