/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    
*/

#ifndef RE_MAINWINDOW_H
#define RE_MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "ui_reMainWindow.h"
#include "ReUiContainer.h"
#include "realitypanel.h"

/**
 The Reality main window. This is mainly a container for RealityPanel 
 with some startup/shutdown services added.
 */
class ReMainWindow : public QMainWindow, 
                     public Ui::MainWindow, 
                     public ReUiContainer
{
  
  Q_OBJECT
  
private:
  RealityPanel* rePanel;

  QLabel* sbNumMats;
  QLabel* sbMessages;

  QAction *undoAction;
  QAction *redoAction;

  void writeWindowConfig();
  void readWindowConfig();
  QNetworkAccessManager* networkManager;

  //! The size of the data thatwe will receive from the licensing server
  int licenseDataSize;

  // The server response data when we ask for updates/license verification
  QByteArray licenseResponse;

  //! We keep track which app called us
  HostAppID appID;

  /**
   * This function checks the validity of the serial number. In the 
   * future it will also provide update notifications for the user.
   */
  void checkForUpdates( const QString& hostVersion );

  //! This method configures all the actions associated with the main window.
  //! It connects them to the menus and stores them in the ReActionManager 
  //! collection so that they can be interfaces by other sections of the program
  void configureActions();

public:
  //! Constructor for MainWindow
  ReMainWindow(const HostAppID& appID = Poser, 
               const QString& ipaddress = "", 
               const QString& hostVersion = "");

  //! Destructor for MainWindow
 ~ReMainWindow() {
  };

  //! Returns the pointer to the MainWindow object
  static ReMainWindow* getRealityMainWindow();

  void closeEvent(QCloseEvent *event);

  QMenuBar* getMenuBar();

  //! Adds a predefined action designed to convert a material to a different
  //! type to the menu of the container. Implementation of the ReUiContainer
  //! interface
  void addConvertActionToMenu( QAction* action );  

  //! Show a message in the status bar. Implementation of the ReUiContainer
  //! interface
  void showMessage( const QString& msg );

  //! ReUiContainer interface 
  void setTitle( const QString& title ) {
    setWindowTitle(title);
  }

private slots:
  //! Open the Reality User's Guide 
  void openRUG();
  //! Open the web page for one of the options in the Help menu, like support, 
  //! video tutorial, etc.
  void helpMenuLinks();

  //! Show the About box
  void openAboutBox();

  //! Show the watermark page
  void showWatermark();

  void saveWatermark( const QUrl& link );

  //! Delete the serial number
  void unregisterReality();

  void processUpdateInfo(QNetworkReply* reply);
  void processConnectionErrors(QNetworkReply::NetworkError error);

  void setUILook();

  //! Call the ACSEL Manager
  void runAcselManager();

  //! Print statistic in the log file
  void printStats();

  //! Edit the application's CSS to set the look and feel
  void editCSS();

  /**
   Brings the window to the foreground. This is called by the host app 
   when the user calls Reality and the the program is already running.   
   */
  void bringToFront();
};

extern ReMainWindow* RealityMainWindow;

#endif