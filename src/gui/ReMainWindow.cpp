/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#include "ReMainWindow.h"

#if defined(_WIN32)
#include <windows.h>
#endif

#include <QCloseEvent>
#include <QDebug>
#include <QJson/Parser>

#include "ReActionMgr.h"
#include "ReSceneDataGlobal.h"
#include "ReVersion.h"
#include "RealityPanel/RealityDataRelay.h"
#include "RealityUI/ReAcselMgr.h"
#include "RealityUI/ReCssEditor.h"
#include "RealityUI/ReUpdateNotification.h"
#include "ui_reAbout.h"
#include "ui_reWatermark.h"


using namespace Reality;

// Globally available handle on the main window
ReMainWindow* RealityMainWindow;

/**
 * Convert a version number in the format v.sub.patch.build to a 32-bit unsigned integer
 */
quint32 packVersionNumber(const QString vnum) {
    QStringList numbers = vnum.split(".");
    if (numbers.count() != 4) {
        return(0);
    }
    qint32 v;
    v = numbers[0].toInt() << 24;
    v += numbers[1].toInt() << 16;
    v += numbers[2].toInt() << 8;
    v += numbers[3].toInt();
    return(v);
};

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
  connect(actionAbout,            SIGNAL(triggered()), SLOT(openAboutBox()));
  connect(actionWatermark,        SIGNAL(triggered()), SLOT(showWatermark()));

  actionMgr->storeAction("rug",              actionRUG);
  actionMgr->storeAction("videos",           actionVideos);
  actionMgr->storeAction("about",            actionAbout);
  actionMgr->storeAction("watermarks",       actionWatermark);

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

  #if defined(__APPLE__)
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

 if ( source == "actionVideos") {
    QDesktopServices::openUrl(QUrl("http://www.youtube.com/user/PretA3D"));
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
  #if defined(_WIN32)
    // Hack to bring Reality in the foreground on Window
    ::SetWindowPos(effectiveWinId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
    ::SetWindowPos(effectiveWinId(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
  #endif
  show();
  activateWindow();
  raise();
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
  QUrl homepageURL(QString::fromUtf8(HOMEPAGE_URL));
  if (!homepageURL.host().compare("github.com", Qt::CaseInsensitive) &&
      QRegExp("/[\\w\\.-]+/[\\w\\.-]+/?").exactMatch(homepageURL.path()))
  {
    QUrl updateURL(QString("https://api.github.com/repos/%1/releases/latest").arg(homepageURL.path().remove(QRegExp("(^/|/$)"))));

    networkManager = new QNetworkAccessManager(this);
    QNetworkRequest request(updateURL);

    request.setRawHeader("Accept", "application/vnd.github+json");
    request.setRawHeader("X-GitHub-Api-Version", "2022-11-28");

    QNetworkReply* reply = networkManager->get(request);

    connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(processUpdateInfo(QNetworkReply*)));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(processConnectionErrors(QNetworkReply::NetworkError)));
  }
}

void ReMainWindow::processConnectionErrors(QNetworkReply::NetworkError error) {
  qDebug() << "Update server connection error: " << error;
}


void ReMainWindow::processUpdateInfo(QNetworkReply* reply) {
  if (reply->error() == QNetworkReply::NoError) {
    QJson::Parser parser;
    bool ok;

    QByteArray data = reply->readAll();

    QVariantMap response = parser.parse(data, &ok).toMap();
    if (!ok) {
      return;
    }

    QRegExp versionCheck = QRegExp("^v?(\\d+)\\.(\\d+)\\.(\\d+)");
    int pos = versionCheck.indexIn(response["name"].toString());
    if (pos > -1) {
      QString curVersion = QString("%1.%2.%3.%4")
        .arg(REALITY_MAIN_VERSION)
        .arg(REALITY_SUB_VERSION)
        .arg(REALITY_PATCH_VERSION)
        .arg(REALITY_BUILD_NUMBER);

      QString newVersion = QString("%1.%2.%3.0")
          .arg(versionCheck.cap(1).toUInt())
          .arg(versionCheck.cap(2).toUInt())
          .arg(versionCheck.cap(3).toUInt());

      qint32 curVersionNumber = packVersionNumber(curVersion);
      qint32 newVersionNumber = packVersionNumber(newVersion);

      if (newVersionNumber > curVersionNumber) {
        if (updateTimeLapsed()) {
          ReUpdateNotification updateDialog;
          updateDialog.setBuildInfo(curVersion, newVersion);
          updateDialog.setDescription(response["body"].toString());
          updateDialog.setUrl(response["html_url"].toString());
          updateDialog.exec();
          saveUpdateCheckDate();
        }
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

static ReCssEditor* cssEditor = NULL;
void ReMainWindow::editCSS() {
  cssEditor = new ReCssEditor(this);
  cssEditor->setAttribute(Qt::WA_DeleteOnClose, true);
  cssEditor->show();
}
