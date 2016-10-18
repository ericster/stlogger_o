/**
 * @file dmesg.cc
 * @brief Dmesg Class file
 */

#include "dmesg.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define KLOG_READ_ALL   3
#define KLOG_LEN    (1 << 17)
#include <android/log.h>
#define LOG_TAG   "SOAK-jni"
#define  ALOG(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

#define BufferSize  256
#define KLOG_SIZE_BUFFER  10
#define KLOG_BUF_SHIFT    17      /* CONFIG_LOG_BUF_SHIFT from our kernel */
#define KLOG_BUF_LEN      (1 << KLOG_BUF_SHIFT)
#define SYS_BOOT_TIME "/proc/uptime"

namespace com {
namespace eolwral {
namespace osmonitor {
namespace core {

  dmesg::dmesg()
  {
    this->_bootTime = 0;
    this->_flatbuffer = NULL;
    this->_list.clear();
  }

  dmesg::~dmesg()
  {
    // clean up _curDemsgList
    if (this->_flatbuffer != NULL)
      delete this->_flatbuffer;
  }

  void dmesg::getBootTime()
  {
     // get uptime
     long uptime = 0;
     FILE *uptimeFile = fopen(SYS_BOOT_TIME, "r");

     if(uptimeFile)
     {
       if ( fscanf(uptimeFile, "%lu.%*u", &uptime) != 0x1 )
         uptime = 0;
       fclose(uptimeFile);
     }

     time_t currentTime = time(0);

     _bootTime = currentTime - uptime;
  }

  void dmesg::prepareBuffer()
  {

    if (this->_flatbuffer != NULL)
      delete this->_flatbuffer;

    this->_flatbuffer = new FlatBufferBuilder();
    this->_list.clear();
  }

  void dmesg::finishBuffer ()
  {
    // finish the buffer
    auto mloc = CreatedmesgInfoList(*this->_flatbuffer, this->_flatbuffer->CreateVector(this->_list));

    FinishdmesgInfoListBuffer(*this->_flatbuffer, mloc);
  }

  void dmesg::gatheringDmesg()
  {
    char* buffer = 0;
    int bufferSize = 0;

    char procLine[BufferSize];
    char* procLineEnd = 0;
    unsigned int procLineLen = 0;
    unsigned int offsetStart = 0;

    bufferSize = klogctl(KLOG_SIZE_BUFFER, 0, 0);
    if (bufferSize <= 0)
      bufferSize = KLOG_BUF_LEN;

    buffer = (char *) malloc(bufferSize + 1);
    if (buffer == 0)
      return;

    int readSize = klogctl(KLOG_READ_ALL, buffer, bufferSize);
    if(readSize < 0) {
      free(buffer);
      return;
    }

    // set C style end
    buffer[readSize] = 0;

    while((procLineEnd = strstr( buffer + offsetStart, "\n" )) != 0)
    {
      procLineLen = procLineEnd - (buffer + offsetStart);

      // every line must less than BufferSize
      if(procLineLen >= BufferSize)
        procLineLen = BufferSize-1;

      // copy message into line buffer
      strncpy(procLine, buffer + offsetStart, procLineLen);
      procLine[procLineLen] = '\0';

      // move to next block
      offsetStart = offsetStart + procLineLen + 1;

      char message[BufferSize];
      //char level = '\x0';
      char level[3] = "";
      unsigned long seconds = 0;
      int itemCounts= 0;

      memset(message, 0, BufferSize);
      memset(level,0, sizeof(level));
      // detect log message format and parse it

      if(procLine[3] == '[')
      {
        itemCounts = sscanf(procLine, "<%1c>[%lu.%*06u] %[^\n]", &level[0], &seconds, message);
        //itemCounts = sscanf(procLine, "<%c>[%lu.%*06u] %[^\n]", &level, &seconds, message);
        //seconds += _bootTime+seconds;
        ALOG("level = %c", (char)level[0]);
        ALOG("seconds %ul", seconds);
        ALOG("message %s", message);
      }
      else if (procLine[4] == '[')
      {
        itemCounts = sscanf(procLine, "<%c%c>[%lu.%*06u] %[^\n]", &level[0], &level[1], &seconds, message);
        //seconds += _bootTime+seconds;
        ALOG("level = %s", level);
        ALOG("seconds %ul", seconds);
        ALOG("message %s", message);
      }
      else
      {
        seconds = 0;
        itemCounts = sscanf(procLine, "<%c>%[^\n]", &level[0], message);
      }

      // push log into list
      if (itemCounts == 2 || itemCounts == 3 || itemCounts == 4)
      {
        // save message
        auto msg = this->_flatbuffer->CreateString(message);

        // prepare a Dmesginfo object
        dmesgInfoBuilder dmesgInfo(*this->_flatbuffer);

        dmesgInfo.add_seconds(seconds);
        switch(strtol(level, NULL, 10))
        {
        //case '0':
        case 0:
          dmesgInfo.add_level(dmesgLevel_EMERGENCY);
          break;
        //case '1':
        case 1:
          dmesgInfo.add_level(dmesgLevel_ALERT);
          break;
        //case '2':
        case 2:
          dmesgInfo.add_level(dmesgLevel_CRITICAL);
          break;
        //case '3':
        case 3:
          dmesgInfo.add_level(dmesgLevel_ERROR);
          break;
        //case '4':
        case 4:
          dmesgInfo.add_level(dmesgLevel_WARNING);
          break;
        //case '5':
        case 5:
          dmesgInfo.add_level(dmesgLevel_NOTICE);
          break;
        //case '6':
        case 6:
          dmesgInfo.add_level(dmesgLevel_INFORMATION);
          break;
        //case '7':
        case 7:
          dmesgInfo.add_level(dmesgLevel_DEBUG);
          break;
        default:
          dmesgInfo.add_level(dmesgLevel_INFORMATION);
          break;
        }

        dmesgInfo.add_message(msg);

        this->_list.push_back(dmesgInfo.Finish());
      }

      // EOF ?
      if(offsetStart >= readSize)
        break;
    }

    // release memory
    if (buffer != 0)
      free(buffer);
  }

  void dmesg::refresh()
  {
    // clean up
    this->prepareBuffer();

    // refresh boot time
    if (this->_bootTime == 0)
      this->getBootTime();

    // gathering dmesg messages
    this->gatheringDmesg();

    // finish the buffer
    this->finishBuffer ();

  }
  const uint8_t* dmesg::getData()
  {
    return this->_flatbuffer->GetBufferPointer();
  }

  const uoffset_t dmesg::getSize()
  {
    return this->_flatbuffer->GetSize();
  }

}
}
}
}

