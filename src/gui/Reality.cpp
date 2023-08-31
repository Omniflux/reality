/*
  Reality plug-in
  Copyright (c) Pret-a-3D/Paolo Ciccone 2012. All rights reserved.    

  This is the Reality executable launcher, aka the main() function
*/

#include <boost/program_options.hpp>
#include <QDialog>
#include <QFileInfo>
#include <QProxyStyle>
#include <QSplashScreen>
#include <QVariant>

#include "ReAcsel.h"
#include "ReLogger.h"
#include "ReMainWindow.h"
#include "ReTools.h"
#include "ReVersion.h"
#include "ui_reCommandLineOptions.h"


namespace bpo = boost::program_options;

/**
 * Proxy Style used to disable the rounded top and bottom margins of 
 * the combo-box popup menu. On Mac OS those top and bottom portions
 * are always visiable and cannot be styled via CSS resulting in white
 * areas above the darker body of the menu set by the style sheet.
 */
class ReProxyStyle : public QProxyStyle
{
 public:
   int styleHint( StyleHint hint, 
                  const QStyleOption *option = 0,
                  const QWidget *widget = 0, 
                  QStyleHintReturn *returnData = 0) const
   {
     //disable combo-box popup top & bottom areas
     if ( SH_ComboBox_Popup == hint ) {
       return 0;
     }
     return QProxyStyle::styleHint( hint, option, widget, returnData ); 
   }
};

void showUsage( const QVariantMap& enabledOptions ) {
  QString version = QString(
                      "%1.%2.%3"
                    )
                    .arg(REALITY_MAIN_VERSION)
                    .arg(REALITY_SUB_VERSION)
                    .arg(REALITY_PATCH_VERSION);
  Ui::reCommandLineOptions ui;
  QDialog cmdOpt(0);
  ui.setupUi(&cmdOpt);
  ui.RealityVersion->setText(version);
  ui.HostAppID->setText(enabledOptions["appID"].toString());
  ui.Configuration->setText(enabledOptions["configurationFile"].toString());
  ui.CommandLine->setText(enabledOptions["usageString"].toString());
  if (enabledOptions["showCredits"].toBool()) {
    ui.Credits->setText("Reality is written by Paolo Ciccone and it's "
                        "copyright (c) 2010 by Pret-a-3D and Paolo Ciccone");
  }
  cmdOpt.exec();
}


void getStartupOptions(int argc, char **argv, QVariantMap& enabledOptions) {

  std::string appIDOpt;
  std::string bundleName;
  std::string aliasFilename;
  std::string ipAddressOpt;
  std::string hostVersion = "";
  std::string luxRenderNodes = "";
  std::string acselBundle = "";

  enabledOptions["showUsage"]    = false;
  enabledOptions["ipAddress"]    = "localhost";
  enabledOptions["appID"]        = "Poser";
  enabledOptions["showcredits"]  = false;

  // These are the publicly-available options
  bpo::options_description stdOptions("Available options");  
  stdOptions.add_options()
    ("help,h", "Shows this help message")
    ("appID", bpo::value<std::string>(&appIDOpt),
     "The name of the application interfacing with Reality. Allowed values: "
     "Poser, DS")
    ("importBundle,b", bpo::value<std::string>(&bundleName),
      "Import the ACSEL bundle named in arg")
    ("importAliases,a", bpo::value<std::string>(&aliasFilename),
      "Import the ACSEL object aliases in arg")
    ("bundleOverwrite,o", 
     "When installing an ACSEL bundle, shaders that already exist in the "
     "database are not installed. If this switch is used then the shaders "
     "in the bundle will overwrite any previous definition of the same "
     "shaders."
    )
    ("exportAcselDB", bpo::value<std::string>(&acselBundle), "Export the ACSEl database to the file specified")
    ("version,v", "Prints the version of Reality and exits") ;

  // Private, hidden options
  bpo::options_description hiddenOptions("Extra options");
  hiddenOptions.add_options() 
    ("ipAddress", bpo::value<std::string>(&ipAddressOpt), "IP Address of the host")
    ("hostVersion", bpo::value<std::string>(&hostVersion), "The version of the host app")
    ("addLuxNodes", bpo::value<std::string>(&luxRenderNodes), "List of Lux render nodes to add to the configuration")
    ("clearLuxNodes", "Clears the list of Lux render nodes")
    ("credits", "Print the credits of the program");

  bpo::options_description allOptions;
  allOptions.add(stdOptions).add(hiddenOptions);

  try {
    bpo::variables_map optionValues;
    // Avoid exceptions for options given on the command line but that are not 
    // under control of boost
    bpo::store(bpo::command_line_parser(argc, argv).options(allOptions).allow_unregistered().run(), optionValues);
    bpo::notify(optionValues);

    // Read the configuration file, if present    
    QByteArray applicationPath = getApplicationPath();
    QFileInfo programLocationInfo(applicationPath.data());
    QString configFilePath;
    configFilePath = QString("%1/RealityConfig.txt").arg(programLocationInfo.absolutePath());

    if (programLocationInfo.exists()) {      
      enabledOptions["configurationFile"] = configFilePath;

      std::ifstream ifs(configFilePath.toUtf8().data());
      if (ifs) {
        bpo::store(bpo::parse_config_file(ifs, allOptions), optionValues);
        bpo::notify(optionValues);
        ifs.close();
      }
    }
    if (optionValues.count("help")) {
      enabledOptions["showUsage"] = true;
      std::stringstream helpDesc;
      helpDesc << stdOptions;      
      enabledOptions["usageString"] = helpDesc.str().c_str();
    }
    if (optionValues.count("appID")) {
      if (appIDOpt == "Poser" || appIDOpt == "DS" || appIDOpt == "Pro") {
        enabledOptions["appID"] = appIDOpt.c_str();        
      }
    }
    if (optionValues.count("importBundle")) {
      enabledOptions["importBundle"] = QString::fromUtf8(bundleName.c_str());
    }
    if (optionValues.count("bundleOverwrite")) {
      enabledOptions["bundleOverwrite"] = true;
    }
    if (optionValues.count("importAliases")) {
      enabledOptions["importAliases"] = QString::fromUtf8(aliasFilename.c_str());
    }
    if (optionValues.count("version")) {
      enabledOptions["version"] = true;
    }
    if (optionValues.count("ipAddress")) {
      enabledOptions["ipAddress"] = ipAddressOpt.c_str();
    }
    if (optionValues.count("hostVersion")) {
      enabledOptions["hostVersion"] = hostVersion.c_str();
    }
    if (optionValues.count("credits")) {
      enabledOptions["showCredits"] = true;
    }
    if (optionValues.count("addLuxNodes")) {
      std::cout << "Adding " << luxRenderNodes 
                << " to the list of LuxRender nodes\n";
      auto config = RealityBase::getConfiguration();
      config->setValue("LuxRenderNodes", luxRenderNodes.c_str());
      config->sync();
    }
    if (optionValues.count("clearLuxNodes")) {
      std::cout << "Clearing the list of LuxRender nodes\n";
      auto config = RealityBase::getConfiguration();
      config->remove("LuxRenderNodes");
      config->sync();
    }
    if (optionValues.count("exportAcselDB")) {
      enabledOptions["exportAcselDB"] = QString::fromUtf8(acselBundle.c_str());
    }
  }
  catch( const std::exception e) {
    std::cerr << "Exception in reading the configuration parameters.\n";
  }
}

int importAcselBundle( const QString& bundleName, const bool bundleOverwrite ) {
  RE_LOG_INFO() << "Installing ACSEL bundle " 
                << bundleName.toStdString() 
                << std::endl;

  ReAcsel* acsel = ReAcsel::getInstance();
  ReAcsel::ExistingShaderSets tmp;
  if (bundleOverwrite) {
    RE_LOG_INFO() << "  overwriting previous shaders, if already present.\n";
  }
  auto retval = acsel->importBundle( bundleName, bundleOverwrite, tmp );
  switch(retval) {
    case ReAcsel::ShaderSetAlreadyExists: {
      RE_LOG_INFO() << "Bundle already exists";
      break;
    }
    case ReAcsel::WrongACSELVersion: {
      RE_LOG_INFO() << "This ACSEL Bundle is not compatible with this"
                       " version of Reality";
      break;
    }
    case ReAcsel::SyntaxError: {
      RE_LOG_INFO() << "This ACSEL Bundle is corrupted";
      break;
    }
    case ReAcsel::NotAnACSELBundle: {
      RE_LOG_INFO() << "The file is not an ACSEL Bundle";
      break;
    }
    case ReAcsel::CannotAddShaderSet: {
      RE_LOG_INFO() << "It was not possible to save a shader set from the bundle";
      break;
    }
    case ReAcsel::GeneralError: {
      RE_LOG_INFO() << "Unknown ACSEL import error";
      break;
    }
    case ReAcsel::Success: {
      RE_LOG_INFO() << "Bundle imported successfully";
      break;
    }
  }  
  return retval;
}

int main(int argc, char **argv) {
  QString ipAddress;
  HostAppID appID = Poser;
  QString hostVersion = "--";

  // Get the command line parameters and settings from the configuration file
  QVariantMap enabledOptions;
  getStartupOptions(argc, argv, enabledOptions);

  ipAddress = enabledOptions["ipAddress"].toString();
  QString tmp = enabledOptions["appID"].toString();
  if (tmp == "Poser") {
    appID = Poser;
  }
  else if (tmp == "DS") {
    appID = DAZStudio;
  }
  if (enabledOptions.contains("version")) {
    std::cout << QString( "Reality %1.%2.%3.%4\n" )
                   .arg(REALITY_MAIN_VERSION)
                   .arg(REALITY_SUB_VERSION)
                   .arg(REALITY_PATCH_VERSION)
                   .arg(REALITY_BUILD_NUMBER)
                   .toStdString();
    return 0;
  }
  if (enabledOptions.contains("hostVersion")) {
    hostVersion = enabledOptions.value("hostVersion").toString();
  }
  if (enabledOptions.contains("importBundle")) {
    return importAcselBundle(
             enabledOptions.value("importBundle").toString(),
             enabledOptions.value("bundleOverwrite").toBool()
            );
  }
  if (enabledOptions.contains("exportAcselDB")) {
    ReAcsel::getInstance()->exportDbToBundle(
      enabledOptions.value("exportAcselDB").toString()
    );
    return 0;
  }
  if (enabledOptions.contains("importAliases")) {
    ReAcsel::getInstance()->importObjectAliases(
      enabledOptions.value("importAliases").toString()
    );
    return 0;
  }

  QApplication app(argc, argv);
  app.setStyle( new ReProxyStyle() );

  QSplashScreen splash(QPixmap(":/images/RealityLogo.png"), Qt::WindowStaysOnTopHint);
  splash.setEnabled(false); //Prevent user from closing the splash
  splash.show();
  app.processEvents();  

  if (enabledOptions["showUsage"].toBool()) {
    splash.hide();
    showUsage(enabledOptions);
  }
  RealityBase::getRealityBase()->setHostAppID(appID);
  
  RealityMainWindow = new ReMainWindow(appID, ipAddress, hostVersion);
  splash.hide();
  RealityMainWindow->show();
  app.processEvents();

#if defined(__APPLE__)
    RealityMainWindow->raise();
    RealityMainWindow->activateWindow();
#endif  

  return app.exec();
}
