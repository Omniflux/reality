/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#include "ReAppVersion.h"
#include "ReMainWindow.h"
#include "RealityBase.h"
#include "ReMaterials.h"
#include "ReTools.h"

#include "ui_reAbout.h"
#include "ui_reWatermark.h"
#include "ReDRM.h"
#include "qjson/src/parser.h"
#include "ReUpdateNotification.h"
#include "ReAcselMgr.h"
#include "ReActionMgr.h"

#include <stdlib.h>

// DRM variables that are defined in other modules. The checks are done
// here to avoid having a single point of failure and to make the 
// hacking more difficult.

// extern bool RealityIsRegistered;

using namespace Reality;

// Globally available handle on the main window
ReMainWindow* RealityMainWindow;

void loadStyleSheet( QString& styleStr ) {
  QFile styleFile(":/textResources/Reality.css");
  if ( styleFile.open(QIODevice::ReadOnly) ) {
    styleStr = styleFile.readAll();
    styleFile.close();
  }
};

void ReMainWindow::setUILook() {
  QString styleStr;
  loadStyleSheet(styleStr);
  setStyleSheet(styleStr);
}

void ReMainWindow::configureActions() {
  // Set the menu bar
  auto undoStack = RealityBase::getUndoStack();
  auto actionMgr = ReActionManager::getInstance();

  undoAction = undoStack->createUndoAction(this, tr("Undo"));
  undoAction->setShortcuts(QKeySequence::Undo);
  actionMgr->storeAction("undo", undoAction);

  actionSave_Scene->setShortcuts(QKeySequence::Save);
  actionMgr->storeAction("save", actionSave_Scene);

  actionCopy->setShortcuts(QKeySequence::Copy);
  actionMgr->storeAction("copy", actionCopy);

  actionPaste->setShortcuts(QKeySequence::Paste);
  actionMgr->storeAction("paste", actionPaste);

  actionPasteWithTextures->setShortcut(Qt::ALT | Qt::CTRL | Qt::Key_V);
  actionMgr->storeAction("paste_with_textures", actionPasteWithTextures);

  // redoAction = undoStack->createRedoAction(this, tr("Redo"));
  // redoAction->setShortcuts(QKeySequence::Redo);  

  // menuEdit->insertAction(actionCopy, redoAction);
  menuEdit->insertAction(actionCopy, undoAction);

  // Help/About menu
  connect(actionRUG,              SIGNAL(triggered()), SLOT(openRUG()));
  connect(actionVideos,           SIGNAL(triggered()), SLOT(helpMenuLinks()));
  connect(actionLicense,          SIGNAL(triggered()), SLOT(helpMenuLinks()));
  connect(actionAutomatic_Presets, SIGNAL(triggered()), SLOT(helpMenuLinks()));
  connect(actionReality_forums,   SIGNAL(triggered()), SLOT(helpMenuLinks()));
  connect(actionContact_Support,  SIGNAL(triggered()), SLOT(helpMenuLinks()));
  connect(actionReality_Freebies, SIGNAL(triggered()), SLOT(helpMenuLinks()));
  connect(actionAbout,            SIGNAL(triggered()), SLOT(openAboutBox()));
  connect(actionWatermark,        SIGNAL(triggered()), SLOT(showWatermark()));
  connect(actionUnregister,       SIGNAL(triggered()), SLOT(unregisterReality()));

  actionMgr->storeAction("rug",              actionRUG);
  actionMgr->storeAction("videos",           actionVideos);
  actionMgr->storeAction("license",          actionLicense);
  actionMgr->storeAction("about",            actionAbout);
  actionMgr->storeAction("watermarks",       actionWatermark);
  actionMgr->storeAction("unregister",       actionUnregister);
  actionMgr->storeAction("automaticPresets", actionAutomatic_Presets);
  actionMgr->storeAction("forums",           actionReality_forums);
  actionMgr->storeAction("support",          actionContact_Support);
  actionMgr->storeAction("freebies",         actionReality_Freebies);

  // ACSEL Menu
  connect(actionACSEL_manager, SIGNAL(triggered()), 
          this, SLOT(runAcselManager()));
  actionMgr->storeAction("acsel_manager", actionACSEL_manager);

  actionMgr->storeAction("apply_compatible_shader_set", actionACSEL_applyCompatibleShaderSet);

  // Undocumented stat function
  auto statFuncAct = new QAction(this);
  statFuncAct->setShortcut(Qt::ALT | Qt::CTRL | Qt::SHIFT | Qt::Key_X);
  statFuncAct->setShortcutContext(Qt::ApplicationShortcut);
  statFuncAct->setText("Stats");
  connect(statFuncAct, SIGNAL(triggered()), this, SLOT(printStats()));
  this->addAction(statFuncAct);
  // menuEdit->addAction(statFuncAct);

  actionMgr->storeAction("stats", statFuncAct);

  // Undocumented editing of CSS file
  auto editCSSAct = new QAction(this);
  editCSSAct->setShortcut(Qt::ALT | Qt::CTRL | Qt::SHIFT | Qt::Key_C);
  editCSSAct->setShortcutContext(Qt::ApplicationShortcut);
  editCSSAct->setText("Edit CSS");
  connect(editCSSAct, SIGNAL(triggered()), this, SLOT(editCSS()));
  this->addAction(editCSSAct);

}

ReMainWindow::ReMainWindow(const HostAppID& appID, 
                           const QString& ipaddress, 
                           const QString& hostVersion) : 
  appID(appID)
{
  RealityMainWindow = this;  
  licenseDataSize = 0;
  checkForUpdates(hostVersion);

  setupUi(this);
 
  ReConfigurationPtr reConfig = RealityBase::getConfiguration(); 

  // Set the Style sheet
  setUILook();

  setWindowTitle(QString(tr("Reality %1 Edition -- unconnected"))
                   .arg(RealityBase::getRealityBase()->getPrintableHostAppID()));
  configureActions();

  show();

  rePanel = new RealityPanel(this, ipaddress);
  setCentralWidget(rePanel);
  connect(realityDataRelay, SIGNAL(bringUIToFront()), this, SLOT(bringToFront()));

  // // Necessary hack because vertical splitters don't respond to 
  // // style sheets in Qt 4.8.6
  // rePanel->materialPageSplitter->setStyleSheet(
  //   "QSplitter::handle:vertical {"
  //   "  background-color: #505050;"
  //   "  height: 4px;"
  //   "};"
  // );

  // Connect the Close button...
  QObject::connect(rePanel->btnClose, SIGNAL(clicked()), this, SLOT(close()));
  QObject::connect(rePanel, SIGNAL(closed()), this, SLOT(close()));

  // Set the status bar
  sbMessages = new QLabel("");
  sbMessages->setIndent(12);
  statusBar()->addWidget(sbMessages,1);
  
  readWindowConfig();
}

ReMainWindow* ReMainWindow::getRealityMainWindow() {
  return RealityMainWindow;
}

QMenuBar* ReMainWindow::getMenuBar() {
  return menubar;
}

void ReMainWindow::addConvertActionToMenu( QAction* action ) {
  menuConvert_to->addAction(action);
}

void ReMainWindow::showMessage( const QString& msg ) {
  sbMessages->setText(msg);
}

void ReMainWindow::openRUG() {
  QFileInfo progPath(QCoreApplication::applicationFilePath());

  #ifdef __APPLE__
    QFileInfo rugPath(QString("%1/../../../Reality_Users_Guide.pdf").arg(progPath.absolutePath()));
  #elif defined(_WIN32)
    QFileInfo rugPath(QString("%1/Reality_Users_Guide.pdf").arg(progPath.absolutePath()));
  #endif

  if (!rugPath.exists()) {
    QMessageBox::information(this, 
                             tr("Information"), 
                             QString(
                               tr("Unfortunately the Reality User's Guide cannot be found. "
                                  "File %1 is missing.")
                             )
                             .arg(rugPath.absoluteFilePath()));
    return;                             
  }
  QString rugURL = QString("file://%1").arg(rugPath.absoluteFilePath());
  QDesktopServices::openUrl(QUrl::fromLocalFile(rugURL));
}

void ReMainWindow::helpMenuLinks() {
  auto source = QObject::sender()->objectName();

  if (source == "actionAutomatic_Presets") {
    QDesktopServices::openUrl(QUrl("http://preta3d.com/reality-automatic-presets/"));
  }
  else if ( source == "actionVideos") {
    QDesktopServices::openUrl(QUrl("http://www.youtube.com/user/PretA3D"));
  }
  else if ( source == "actionReality_forums") {
    QDesktopServices::openUrl(QUrl("http://preta3d.com/forums"));
  }
  else if ( source == "actionReality_Freebies") {
    QDesktopServices::openUrl(QUrl("http://preta3d.com/reality-automatic-presets/"));
  }
  else if ( source == "actionLicense") {
    QDesktopServices::openUrl(QUrl("http://preta3d.com/retrieve-reality-license/"));
  }
  else if ( source == "actionContact_Support") {
    QDesktopServices::openUrl(QUrl("http://preta3d.com/support/"));
  }
}

void ReMainWindow::openAboutBox() {
  Ui::reAbout about;
  QDialog aboutBox(this);
  about.setupUi(&aboutBox);

  // Set up the About box text
  QString realityVersion = QString("%1.%2.%3.%4")
                           .arg(REALITY_MAIN_VERSION)
                           .arg(REALITY_SUB_VERSION)
                           .arg(REALITY_PATCH_VERSION)
                           .arg(REALITY_BUILD_NUMBER);
  QFile res(":/textResources/about.html");
  res.open(QIODevice::ReadOnly);
  QTextStream aboutPageStream(&res);
  QString aboutText = aboutPageStream.readAll();
  aboutText.replace("$QT_BUILD_VERSION", QT_VERSION_STR);
  aboutText.replace("$QT_LINK_VERSION", qVersion());
  about.textBrowser->setHtml(aboutText.arg(realityVersion));
  res.close();
  aboutBox.exec();
}

void ReMainWindow::showWatermark() {
  Ui::reWatermark watermark;
  QDialog watermarkBox(this);
  watermark.setupUi(&watermarkBox);
  //! Save the watermark when the user clicks on the link 
  connect(watermark.textBrowser, SIGNAL(anchorClicked(const QUrl&)),
          this, SLOT(saveWatermark(const QUrl&)));

  watermark.textBrowser->setSource(QUrl(":/textResources/watermark.html"));
  watermarkBox.exec();
}

void ReMainWindow::saveWatermark( const QUrl& link ) {
  QString dirName = QFileDialog::getExistingDirectory(
                      this, 
                      tr("Directory where to save the watermark file"),
                      "",
                      QFileDialog::ShowDirsOnly
                    );

  if (dirName.isEmpty()) {
    return;
  }
  QFile img(":" + link.path());
  QByteArray imgData;
  if ( img.open(QIODevice::ReadOnly) ) {
    imgData = img.readAll();
    img.close();
  }
  else {
    QMessageBox::information(this, 
                             tr("Watermark"), 
                             QString(tr("Could not read the image file")));
    return;
  }

  QFileInfo fi(link.path());
  QString wmFileName = QString("%1/%2.%3").arg(dirName).arg(fi.baseName()).arg(fi.suffix());
  QFile wmFile(wmFileName);
  if (wmFile.open(QIODevice::WriteOnly)) { 
    wmFile.write(imgData);
    wmFile.close();
    QMessageBox::information(this, 
                             tr("Watermark"), 
                             QString(tr("Watermark file saved as %1").arg(wmFileName)));
  }
  else {
    QMessageBox::information(this, 
                             tr("Watermark"), 
                             QString(tr("Could not save file %1").arg(wmFileName)));

  }
  
}

void ReMainWindow::bringToFront() {
  #ifdef WIN32
    // Hack to bring Reality in the foreground on Window
    ::SetWindowPos(effectiveWinId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
    ::SetWindowPos(effectiveWinId(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
  #endif
  show();
  activateWindow();
  raise();
}

void ReMainWindow::unregisterReality() {
  QMessageBox::StandardButton result = QMessageBox::question(
    this, 
    tr("Confirmation"), 
    tr("Do you really want to remove the registration for Reality?"),
    QMessageBox::Yes | QMessageBox::No,
    QMessageBox::No
  );
  if (result == QMessageBox::Yes) {
    removeRegistration();
    QMessageBox::information(
      this, 
      tr("Information"), 
      tr("This copy of Reality has been un-registered. Reality will now close.")
    );
    close();
  }
}

void ReMainWindow::writeWindowConfig() {
  ReConfigurationPtr settings = RealityBase::getConfiguration();
  settings->beginGroup("MainWindow");
  settings->setValue("size", size());
  settings->setValue("pos", pos());
  settings->endGroup();
  settings->sync();
}

void ReMainWindow::readWindowConfig() {
  ReConfigurationPtr settings = RealityBase::getConfiguration();
  settings->beginGroup("MainWindow");
  resize(settings->value("size", QSize(400, 400)).toSize());
  move(settings->value("pos", QPoint(200, 200)).toPoint());
  settings->endGroup();
}

void ReMainWindow::closeEvent(QCloseEvent *event) {
  writeWindowConfig();
  rePanel->disconnect();
  event->accept();
}

//! Verifies if Reality needs to report an available update to the 
//! user.
bool updateTimeLapsed() {
  QDate today = QDate::currentDate();
  ReConfigurationPtr config = RealityBase::getConfiguration();
  if (!config->contains(RE_CFG_LAST_UPDATE_CHECK)) {
    config->setValue(RE_CFG_LAST_UPDATE_CHECK, today.toString(RE_CFG_DATE_FORMAT));
    return true;
  }
  QDate lastCheck = QDate::fromString(
    config->value(RE_CFG_LAST_UPDATE_CHECK).toString(),
    RE_CFG_DATE_FORMAT
  );

  return lastCheck.daysTo(today) > RE_CFG_DAYS_BETWEEN_UPDATE_CHECKS;
}

//! Stores the date of the last time we checked if there was an update
void saveUpdateCheckDate() {
  ReConfigurationPtr config = RealityBase::getConfiguration();
  QDate today = QDate::currentDate();
  config->setValue(RE_CFG_LAST_UPDATE_CHECK, today.toString(RE_CFG_DATE_FORMAT));
}


void ReMainWindow::checkForUpdates( const QString& hostVersion ) {
  networkManager = new QNetworkAccessManager(this);
  // This is actually the class that handles the license data
  ReStringMap userData = ReUserData::getUserData();

  QNetworkRequest request;
  QString version = QString(
                      "%1.%2.%3.%4"
                    )
                    .arg(REALITY_MAIN_VERSION)
                    .arg(REALITY_SUB_VERSION)
                    .arg(REALITY_PATCH_VERSION)
                    .arg(REALITY_BUILD_NUMBER);

  QUrl url(decryptString(UPDATER_URL));
  QString keyOrderNo, 
          keySerialNo;

  keyOrderNo = decryptString(RE_USER_DATA_ORDER_NO);
  keySerialNo = decryptString(RE_USER_DATA_SERIAL_NO);

  // The product code is "RE", for "Reality"
  url.addQueryItem(decryptString(FLD_PROD_CODE), "RE");
  url.addQueryItem(decryptString(FLD_BUILD_NO), version);
  url.addQueryItem(decryptString(FLD_OS), 
    (RealityBase::getRealityBase()->getOSType() == MAC_OS ? "M" : "W"));
  url.addQueryItem(keyOrderNo, userData[keyOrderNo]);
  url.addQueryItem(keySerialNo, userData[keySerialNo]);
  // The host version is empty for now because we cannot grab that
  // at this point. This was used to distinguish between DS3 and DS4
  url.addQueryItem(decryptString(FLD_HOST_VERSION), hostVersion);
  QString appCode;
  switch(appID) {
    case Poser:
      appCode = "PS";
      break;
    case DAZStudio:
      appCode = "DS";
      break;
    default:
      appCode = "--";      
  }
  url.addQueryItem(decryptString(FLD_HOST_APP), appCode);

  request.setUrl(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, 
                    decryptString(HTTP_POST_HEADER));

  QNetworkReply *reply = networkManager->post(request, url.encodedQuery());
  connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(processUpdateInfo(QNetworkReply*)));
  connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
          this, SLOT(processConnectionErrors(QNetworkReply::NetworkError)));
}

void ReMainWindow::processConnectionErrors(QNetworkReply::NetworkError error) {
  qDebug() << "Update server connection error: " << error;
}


void ReMainWindow::processUpdateInfo(QNetworkReply* reply) {
  // If therer was an error it's likely that the machine is not 
  // connected. We don't want to remove the license because the
  // computer is off-line.
  if (reply->error() != QNetworkReply::NoError) {
    return;
  }
  QJson::Parser parser;
  bool ok;

  QByteArray rawData = reply->readAll();

  // We might receive the data in several chunks. So, we first retrieve 
  // the length of the stream that we will receive 
  QTextStream dStream(rawData);

  // If this is the first chunk then we don't know yet the size of the data to be read
  // The size is in clear at the beginning of the data and it's separated from the data
  // itself by a space.
  if (licenseDataSize == 0) {
    dStream >> licenseDataSize;
    char ch;
    // remove the space after the number
    dStream >> ch;
  }
  // Now read the data received and append it to the buffer. If the read is complete then 
  // we will read the response as a series of JSON fields.
  QByteArray tmp;
  dStream >> tmp;
  licenseResponse.append(tmp);

  // If we didn't receive all the data then exit and wait for the next chunk
  if (licenseResponse.length() < licenseDataSize) {
    return;
  }

  QByteArray clearData = QByteArray::fromBase64(licenseResponse);

  QVariantMap response = parser.parse(clearData, &ok).toMap();
  if (!ok) {
    return;
  }

  // The status field from the server data contains a SHA1 hash obtained 
  // from the customer's data
  // We recreate the hash here and check if the two are the same. If they 
  // don't match then the license is invalid.

  // Create the sha1 hash based on the following string:
  //   orderNo,lastName,firstname,serialNo,orderNo
  ReStringMap userData = ReUserData::getUserData();

  // Obfuscation, see note below
  QString orderNo = userData[decryptString(RE_USER_DATA_ORDER_NO)];
  QString hashBase = QString("%1%2%3")
                     .arg(orderNo)
                     // .arg(userData[decryptString(RE_USER_DATA_LAST_NAME)])
                     // .arg(userData[decryptString(RE_USER_DATA_FIRST_NAME)])
                     .arg(userData[decryptString(RE_USER_DATA_SERIAL_NO)])
                     // Note we use the order # from the user data to obfuscate the fact that we are using the same 
                     // value twice. In this way we don't show the same variable in the disassembled code
                     .arg(userData[decryptString(RE_USER_DATA_ORDER_NO)]);

  QString hash = sha1(hashBase);

  if (response[decryptString(FLD_STATUS)] != hash) {
    removeRegistration();
    QMessageBox::information(
      this, 
      tr("Information"), 
      tr("The license for this installation of Reality is invalid.\n"
         "Please contact Pret-a-3D to obtain a valid license.\n"
         "The contact our support team please go to http://preta3d.com and select the Support menu.")
      );
    // Andromeda code
    exit(601);
    return;
  }
  QString description = response["description"].toString();

  if ( !description.isNull() ) {
    // All good up to here, let's check if this build is up-to-date
    QString currVersion = QString(
                        "%1.%2.%3.%4"
                      )
                      .arg(REALITY_MAIN_VERSION)
                      .arg(REALITY_SUB_VERSION)
                      .arg(REALITY_PATCH_VERSION)
                      .arg(REALITY_BUILD_NUMBER);
    QString newVersion = response["buildNo"].toString();
    qint32 newVersionNumber = packVersionNumber(newVersion);
    qint32 thisVersionNumber = packVersionNumber(currVersion);

    if (newVersionNumber > thisVersionNumber) {
      if ( updateTimeLapsed()) {
        ReUpdateNotification updateDialog;
        updateDialog.setBuildInfo(currVersion, newVersion);
        updateDialog.setDescription(description);
        updateDialog.exec();
        saveUpdateCheckDate();
      }
    }
  }
  reply->deleteLater();
}

void ReMainWindow::runAcselManager() {
  auto dialog = new ReAcselManager(this);
  dialog->exec();
}

void ReMainWindow::printStats() {
  RE_LOG_INFO() << "Number of objects: " << RealitySceneData->getNumObjects() << "\n"
                << "Number of cameras: " << RealitySceneData->getNumCameras() << "\n"
                << "Number of lights: " << RealitySceneData->getNumLights();
}

#include "ReCssEditor.h"
static ReCssEditor* cssEditor = NULL;
void ReMainWindow::editCSS() {
  cssEditor = new ReCssEditor(this);
  cssEditor->setAttribute(Qt::WA_DeleteOnClose, true);
  cssEditor->show();
}
