/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

/**
 * This progran computes the CRC for a file, passed in the first parameter,
 * and generates the C source necessary to store the CRC value in the Reality 
 * library
 */
#include "crc.h"
#include <QFile>
#include <QByteArray>

#include <iostream>

using namespace std;
int main( int argc, char* argv[] ) {
  cout << "Computing CRC for " << argv[1] 
       << " and storing it in " << argv[2] << endl;
  QFile prog(argv[1]);
  int computedCRC = 0;
  if ( prog.open(QIODevice::ReadOnly) ) {
    QByteArray content = prog.readAll();
    prog.close();
    computedCRC = crcFast(content.data(), content.count());
  }
  else {
    cout << "Error: cannot open file " << argv[1] << endl;
    return 1;
  }
  QFile library(argv[2]);

  if ( library.open(QIODevice::ReadWrite) ) {
    QByteArray buffer = library.readAll();
    const char* key = "Reality![";
    int pos = buffer.indexOf(key);
    if (pos != -1) {
      cout << "CRC String found at " << pos << endl;
      library.seek(pos+strlen(key));
      library.write((char*)&computedCRC, sizeof(computedCRC));
    }
    library.close();
  }
  else {
    cout << "Error: could not open file " << argv[2] << endl;
    return 1;
  }
  return 0;
}

