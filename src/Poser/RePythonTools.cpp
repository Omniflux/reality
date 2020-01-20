/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#include "RePythonTools.h"
#include "ReLogger.h"

using namespace python;

QString getQString(boost::python::object u) {
  const char* value = extract<const char*>(str(u).encode("utf-8"));
  return QString::fromUtf8(value);
}

//! Converts either a Python List or Tuple, based on the
//! template parameter, to a QVariantList
template<typename T>
void convertPythonList( const T& tup, QVariantList& list ) {

  int numItems = len(tup);
  for (int i = 0; i < numItems; i++) {
    QVariant val;
    python::object oValue = tup[i];
    PyObject* valuePtr = oValue.ptr();

    if (PyString_Check(valuePtr) || PyUnicode_Check(valuePtr)) {
      // val = QVariant( extract<QString>(str(oValue)) );
      val = QVariant( getQString(oValue) );
    }
    else if (PyFloat_Check(valuePtr)) {
      val = QVariant( extract<float>(oValue) );
    }
    else if (PyInt_Check(valuePtr)) {
      val = QVariant( extract<int>(oValue) );
    }
    else if (PyLong_Check(valuePtr)) {
      qlonglong v = extract<long>(oValue);
      val = QVariant(v);
    }
    else if (PyBool_Check(valuePtr)) {
      val = QVariant( extract<bool>(oValue) );
    }
    else if (PyTuple_Check(valuePtr)) {
      QVariantList subList;
      python::tuple subTup = extract<python::tuple>(oValue);
      convertPythonList<python::tuple>(subTup, subList);
      val = QVariant(subList);
    }
    else if (PyList_Check(valuePtr)) {
      QVariantList subList;
      python::list subPyList = extract<python::list>(oValue);
      convertPythonList<python::list>(subPyList, subList);
      val = QVariant(subList);
    }
    else if (PyDict_Check(valuePtr)) {
      QVariantMap subMap;
      python::dict subDict = extract<python::dict>(oValue);
      convertPythonDict(subDict, subMap);
      val = QVariant( subMap );
    }
    else {
      // val = QVariant( extract<QString>(str(oValue.attr("__class__"))) );
      val = QVariant( getQString(oValue.attr("__class__")) );
    }
    list.append(val);
  }    
}

void convertPythonDict( const python::dict& dict, QVariantMap& map ) {
  try {
    python::object ikeys = dict.iterkeys();
    python::object ivalues = dict.itervalues();
    int numItems = len(dict);
    for(int i=0; i < numItems; i++ ) {
      python::object oKey = ikeys.attr("next")();
      python::object oValue = ivalues.attr("next")();

      QString key = getQString(oKey);
      QVariant value;
      PyObject* valuePtr = oValue.ptr();
      if (PyString_Check(valuePtr) || PyUnicode_Check(valuePtr)) {
        // value = QVariant( extract<QString>(str(oValue)) );
        value = QVariant( getQString(oValue) );
      }
      else if (PyFloat_Check(valuePtr)) {
        value = QVariant( extract<float>(oValue) );
      }
      else if (PyInt_Check(valuePtr)) {
        value = QVariant( extract<int>(oValue) );
      }
      else if (PyLong_Check(valuePtr)) {
        value = QVariant( static_cast<qlonglong>(extract<long>(oValue)) );
      }
      else if (PyBool_Check(valuePtr)) {
        value = QVariant( extract<bool>(oValue) );
      }
      else if (PyDict_Check(valuePtr)) {
        QVariantMap subMap;
        python::dict subDict = extract<python::dict>(oValue);
        convertPythonDict(subDict, subMap);
        value = QVariant( subMap );
      }
      else if (PyTuple_Check(valuePtr)) {
        QVariantList list;
        python::tuple subTup = extract<python::tuple>(oValue);
        convertPythonList(subTup, list);
        value = QVariant(list);
      }
      else if (PyList_Check(valuePtr)) {
        QVariantList list;
        python::list pyList = extract<python::list>(oValue);
        convertPythonList(pyList, list);
        value = QVariant(list);
      }
      else {
        value = QVariant( getQString(oValue.attr("__class__")) );
      }
      map[key] = value;
    }
  }
  catch( const std::exception& e) {
    RE_LOG_INFO() << "Exception in RealityX::convertPythonDict()";
  }
}

void QVariantMapToDict( const QVariantMap& qmap, python::dict& pyDict ) {
  QMapIterator<QString, QVariant> i(qmap);

  while( i.hasNext() ) {
    i.next();
    QString key = i.key();
    QVariant val = i.value();

    switch( val.type() ) {
      case QVariant::String:
        pyDict[key.toStdString()] = python::str(val.toString().toStdString());
        break;
      case QVariant::Bool:
        pyDict[key.toStdString()] = val.toBool();
        break;
      case QMetaType::Int:
      case QMetaType::Long:
      case QMetaType::LongLong:
        pyDict[key.toStdString()] = val.toLongLong();
        break;
      case QMetaType::UInt:
      case QMetaType::ULong:
      case QMetaType::ULongLong:
        pyDict[key.toStdString()] = val.toULongLong();
        break;
      case QMetaType::Float:
      case QMetaType::Double: {
        pyDict[key.toStdString()] = val.toDouble();
        break;
      }
      case QVariant::Map: {
        python::dict newDict;
        QVariantMapToDict(val.toMap(), newDict);
        pyDict[key.toStdString()] = newDict;
        break;
      }
      case QVariant::List: {
        python::list pyList;
        QVariantListToList(val.toList(), pyList);
        pyDict[key.toStdString()] = pyList;
        break;
      }
    }
  }
}

void QVariantListToList( const QVariantList& qlist, python::list lst ) {
  int numElements = qlist.count();
  for (int i = 0; i < numElements; i++) {
    QVariant val = qlist[i];
    switch( val.type() ) {
      case QVariant::String:
        lst.append(val.toString().toStdString());
        break;
      case QVariant::Bool:
        lst.append(val.toBool());
        break;
      case QMetaType::Int:
      case QMetaType::Long:
      case QMetaType::LongLong:
        lst.append(val.toLongLong());
        break;
      case QMetaType::UInt:
      case QMetaType::ULong:
      case QMetaType::ULongLong:
        lst.append(val.toULongLong());
        break;
      case QMetaType::Float:
      case QMetaType::Double: {
        lst.append(val.toDouble());
        break;
      }
      case QMetaType::QVariantList:{
        python::list newList;
        QVariantListToList(val.toList(), newList);
        lst.append(newList);
        break;
      }
      case QMetaType::QVariantMap: {
        python::dict newDict;
        QVariantMapToDict(val.toMap(), newDict);
        lst.append(newDict);
        break;
      }
    }
  }
}

