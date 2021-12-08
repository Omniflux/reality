/**
 * \file
 *  Reality plug-in
 *  Copyright (c) Pret-a-3D/Paolo Ciccone 2014. All rights reserved.    
 */

#ifndef SHARED_MEM_IPC_H
#define SHARED_MEM_IPC_H

#include <QSharedMemory>
#include "RealityBase.h"
#include "ReLogger.h"
#include "ReIPCCommands.h"

#define BOOST_DATE_TIME_NO_LIB 1
#ifndef Q_MOC_RUN
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#endif

//! The name of the shared memory segment used to share the render options.
#define SHM_SCENE_RENDER_OPTIONS_NAME  "ReSceneRenderOptions"

//! The name of the shared memory segment used to transfer data for which we 
//! cannot predict the size in advance
#define SHM_VARIABLE_SIZE_CHANNEL_NAME "ReElasticData"

//! The name of the shared memory segment that is used to pass commands from
//! the host to the GUI
#define SHM_COMMANDS_HOST_TO_GUI_CHANNEL_NAME  "ReHostToGuiCommands"

//! The name of the shared memory segment that is used to pass commands from
//! the GUI to the host
#define SHM_COMMANDS_GUI_TO_HOST_CHANNEL_NAME  "ReGuiToHostCommands"

//! Size of the shared memory segment used to pass the data about the 
//! render options. One megabyte should suffice for event the largest
//! object.
#define SHM_SCENE_RENDER_OPTIONS_SIZE 1024*1024

//! Size of the channels used to send the command. We use enums for that.
#define SHM_COMMANDS_CHANNEL_SIZE  sizeof(IPC_MESSAGES)

/**
 * Class ReSharedMemIPC implements IPC using shared emmeory for transfering
 * objects using array buffers and QDataStream.
 */

// using namespace bipc;
using namespace Reality;
namespace bipc = boost::interprocess;

class ReSharedMemIPC {
private:
  bipc::shared_memory_object* shmBlock;
  bipc::mapped_region* readRegion;

public:
  enum ErrorCode {
    NotAttached,
    CouldNotLockData,
    SegmentTooSmall,
    OK
  };

  //! Communication can be performed via one out of three channels available.
  //! This enum defines the channel selector.
  enum ChannelSelector {
    FixedSizeChannel,
    VariableSizeChannel,
    CommandChannel
  };

  //! Constructor: ReSharedMemIPC
  ReSharedMemIPC() {
    shmBlock = NULL;
    readRegion = NULL;
    // Configure the shared memory segment for the render options
    QString shmKey = QString("%1_%2")
                       .arg(SHM_SCENE_RENDER_OPTIONS_NAME)
                       .arg(RealityBase::getRealityBase()->getHostAppIDAsString());
    RE_LOG_INFO() << "Shared memory segment key: " << shmKey;
    try {
      shmBlock = new bipc::shared_memory_object(bipc::open_or_create,
                                                shmKey.toAscii(),
                                                bipc::read_write);
      bipc::offset_t dataSize;
      if (shmBlock->get_size(dataSize)) {
        if (dataSize == 0) {
          shmBlock->truncate(SHM_SCENE_RENDER_OPTIONS_SIZE);
        }
      }
      shmBlock->get_size(dataSize);
      RE_LOG_INFO() << "Shared memory block is: " << dataSize << " bytes";
    }
    catch( const bipc::interprocess_exception& ipce ) {
      RE_LOG_INFO() << "Error in ReSharedMemIPC ctor: " << ipce.what();
    }
  };

  //! Destructor: ReSharedMemIPC
  ~ReSharedMemIPC() {
    if (shmBlock != NULL) {
      delete shmBlock;
    }
    if (readRegion != NULL) {
      delete readRegion;
    }
  };

protected:
  inline ErrorCode writeToFixedSizeChannel( const QByteArray& outbound ) {
    bipc::mapped_region region(*shmBlock, bipc::read_write);
    quint32 dataSize = outbound.count();
    char* target = static_cast<char*>(region.get_address());
    memcpy(target, &dataSize, sizeof(dataSize));
    memcpy(target+sizeof(dataSize), outbound.data(), dataSize);
    return OK;
  }

  inline ErrorCode readFromFixedSizeChannel( QByteArray& inbound ) {
    if (readRegion == NULL) {
      readRegion = new bipc::mapped_region(*shmBlock, bipc::read_only);
    }
    quint32 dataSize = 0;
    char* source = static_cast<char*>(readRegion->get_address());
    memcpy(&dataSize, source, sizeof(dataSize));
    // RE_LOG_INFO() << "Expected buffer from shared memory: " << dataSize << " long";
    inbound = QByteArray::fromRawData(source+sizeof(dataSize), dataSize);
    return OK;
  }  

public:

  inline ErrorCode read( const ChannelSelector which, QByteArray& inbound ) {
    switch(which) {
      case FixedSizeChannel:
        return readFromFixedSizeChannel(inbound);
        break;
      case VariableSizeChannel:
        return OK;
        break;
      case CommandChannel:
        return OK;
        break;
    }
  }

  inline ErrorCode write( const ChannelSelector which, const QByteArray& outbound ) {
    switch(which) {
      case FixedSizeChannel:
        return writeToFixedSizeChannel(outbound);
        break;
      case VariableSizeChannel:
        return OK;
        break;
      case CommandChannel:
        return OK;
        break;
    }
  }

  inline bipc::offset_t getSize() {
    bipc::offset_t dataSize;
    shmBlock->get_size(dataSize);
    return dataSize;
  }
};


#endif
