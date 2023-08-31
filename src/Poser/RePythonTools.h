/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef RE_PYTHON_TOOLS_H
#define RE_PYTHON_TOOLS_H

#include <boost/python.hpp>
#include <QVariant>


namespace python = boost::python;

QString getQString(boost::python::object u);

//! Converts either a Python List or Tuple, based on the
//! template parameter, to a QVariantList
template<typename T>
void convertPythonList( const T& tup, QVariantList& list );

//! Converts a Python dictionary to a QVariantMap
void convertPythonDict( const python::dict& dict, QVariantMap& map );

//! Converts a QVariantList to a Python list
void QVariantListToList( const QVariantList& qlist, python::list tup );

//! Converts a QVariantList to a Python dictionary
void QVariantMapToDict( const QVariantMap& qmap, python::dict& pyDict );

#endif
