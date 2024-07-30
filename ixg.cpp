
#include "includes/gst.h"
#include "includes/utils.h"
#include "jsonParser.h"
#include <iostream>
#include <thread>

volatile sig_atomic_t ctrlc_pressed = 0;

ixg::ixgElement *__ixgElementInstance[4] = {NULL};
ixgStream::gstBasePipe *__gstpipeInstance[4] = {NULL};
ixgStream::gstOutputTeePipe *__gstOutputInstance[4][5] = {NULL};

void ixgStartChannel(ixgParser _parser, string file, int ch) {
  if (__ixgElementInstance[ch] == NULL) {
    __ixgElementInstance[ch] = new ixg::ixgElement();
    getJsonValue(__ixgElementInstance, _parser.parsering(file), ch);
  } else {
    std::cout << "free element and create again" << std::endl;
    delete __ixgElementInstance[ch];
    __ixgElementInstance[ch] = NULL;
    __ixgElementInstance[ch] = new ixg::ixgElement();
    getJsonValue(__ixgElementInstance, _parser.parsering(file), ch);
  }

  if (__ixgElementInstance[ch]->__ixgElemt.__ixgInput.active) {
    if (__gstpipeInstance[ch] == NULL) {
      std::cout << "calling gstreamer" << std::endl;
      __gstpipeInstance[ch] = new ixgStream::gstBasePipe();
      __gstpipeInstance[ch]->gstBasePipeline(__ixgElementInstance, ch);
      _ixgbool temp = true;
      __gstpipeInstance[ch]->start(&temp, true);
    } else
      std::cout << "pipline already active" << std::endl;
  } else {
    _ixgbool temp = true;
    __gstpipeInstance[ch]->stop(&temp, true);
    delete __gstpipeInstance[ch];
    __gstpipeInstance[ch] = NULL;
  }
  /*********************************************************/
  // Parsing outputs
  /*const Json::Value &outputs = root["outputs"];
  for (const auto &output : outputs) {
    std::string outputType = output["type"].asString();
    bool outputActive = output["active"].asBool();
    bool isNeedEncoding = output["encoding"]["isencode"].asBool();
    bool startFlag = false;
    bool startNoActionFlag = false;
    bool stopFlag = false;
    bool stopNoActionFlag = false;
    cpipe++;
    gPPipe = cpipe;
    goutputType = outputType;
    // cout << "outputtype=" << outputType << endl;
    if (outputType == "RTMP") {
      // std::string rtmpURI = output["config"]["uri"].asString();
      if (outputActive) {
        if (outputInstance[ppipe][cpipe] == NULL) {
          outputInstance[ppipe][cpipe] = new gstOutputPipe();
          if (isNeedEncoding)
            outputInstance[ppipe][cpipe]->gstOutputPipeStart(
                instance[ppipe]->getpipe(), output, true);
          else
            outputInstance[ppipe][cpipe]->gstOutputPipeStart(
                instance[ppipe]->getpipe(), output);

          outputInstance[ppipe][cpipe]->setActive(true);
          startFlag = true;
        } else
          startNoActionFlag = true;
      } else {
        if ((outputInstance[ppipe][cpipe] != NULL) &&
            (outputInstance[ppipe][cpipe]->isActive())) {
          if (isNeedEncoding)
            outputInstance[ppipe][cpipe]->gstOutputPipeStop(
                instance[ppipe]->getpipe());
          else
            outputInstance[ppipe][cpipe]->gstOutputPipeStop(
                instance[ppipe]->getpipe(), 2);

          delete outputInstance[ppipe][cpipe];
          outputInstance[ppipe][cpipe] = NULL;
          stopFlag = true;
        } else
          stopNoActionFlag = true;
      }
    }
  }*/
  /******************************************************* */
  for (int i = 0; i < 5; i++) {

    if (__ixgElementInstance[ch]->__regOutputList[i].name == "Multicast") {
      if (__ixgElementInstance[ch]->__regOutputList[i].active) {
        if (__gstOutputInstance[ch][i] == NULL) {
          __gstOutputInstance[ch][i] = new ixgStream::gstOutputTeePipe();
          __gstOutputInstance[ch][i]->gstOutputPipeStart(
              __gstpipeInstance[ch]->getpipe(), __ixgElementInstance, ch, i);
        } else
          cout << "already active" << endl;
      } else {
        if (__gstOutputInstance[ch][i] != NULL) {
          __gstOutputInstance[ch][i]->gstOutputPipeStop(
              __gstpipeInstance[ch]->getpipe(), 1);
          delete __gstOutputInstance[ch][i];
          __gstOutputInstance[ch][i] = NULL;
        } else
          cout << "already inactive" << endl;
      }
    }
    if (__ixgElementInstance[ch]->__regOutputList[i].name == "SRT") {
      if (__ixgElementInstance[ch]->__regOutputList[i].active) {
        if (__gstOutputInstance[ch][i] == NULL) {
          __gstOutputInstance[ch][i] = new ixgStream::gstOutputTeePipe();
          __gstOutputInstance[ch][i]->gstOutputPipeStart(
              __gstpipeInstance[ch]->getpipe(), __ixgElementInstance, ch, i);
        } else
          cout << "already active" << endl;
      } else {
        if (__gstOutputInstance[ch][i] != NULL) {
          __gstOutputInstance[ch][i]->gstOutputPipeStop(
              __gstpipeInstance[ch]->getpipe(), 1);
          delete __gstOutputInstance[ch][i];
          __gstOutputInstance[ch][i] = NULL;
        } else
          cout << "already inactive" << endl;
      }
    }
    if (__ixgElementInstance[ch]->__regOutputList[i].name == "Record") {
      if (__ixgElementInstance[ch]->__regOutputList[i].active) {
        if (__gstOutputInstance[ch][i] == NULL) {
          __gstOutputInstance[ch][i] = new ixgStream::gstOutputTeePipe();
          __gstOutputInstance[ch][i]->gstOutputPipeStart(
              __gstpipeInstance[ch]->getpipe(), __ixgElementInstance, ch, i);
        } else
          cout << "already active" << endl;
      } else {
        if (__gstOutputInstance[ch][i] != NULL) {
          __gstOutputInstance[ch][i]->gstOutputPipeStop(
              __gstpipeInstance[ch]->getpipe(), 1);
          delete __gstOutputInstance[ch][i];
          __gstOutputInstance[ch][i] = NULL;
        } else
          cout << "already inactive" << endl;
      }
    }
    if (__ixgElementInstance[ch]->__regOutputList[i].name == "Delay") {
    }
    if (__ixgElementInstance[ch]->__regOutputList[i].name == "RTMP") {
      if (__ixgElementInstance[ch]->__regOutputList[i].active) {
        if (__gstOutputInstance[ch][i] == NULL) {
          __gstOutputInstance[ch][i] = new ixgStream::gstOutputTeePipe();
          __gstOutputInstance[ch][i]->gstOutputPipeStart(
              __gstpipeInstance[ch]->getpipe(), __ixgElementInstance, ch, i);
        } else
          cout << "already active" << endl;
      } else {
        if (__gstOutputInstance[ch][i] != NULL) {
          __gstOutputInstance[ch][i]->gstOutputPipeStop(
              __gstpipeInstance[ch]->getpipe(), 2);
          delete __gstOutputInstance[ch][i];
          __gstOutputInstance[ch][i] = NULL;
        } else
          cout << "already inactive" << endl;
      }
    }

    cout << "type: " << __ixgElementInstance[ch]->__regOutputList[i].name
         << "\tActive: " << std::boolalpha
         << __ixgElementInstance[ch]->__regOutputList[i].active << endl;
  }
}

// Signal handler for Ctrl+C (SIGINT)
void signalHandler(int signum) {
  if (signum == SIGINT) {
    std::cout << "get terminate signal. Exiting..." << std::endl;
    ctrlc_pressed = 1;
  }
}

int main(int argc, char *argv[]) {

  signal(SIGINT, signalHandler);
  gst_init(&argc, &argv);

  /*gst_debug_set_active(true);
  gst_debug_set_default_threshold(
      GST_LEVEL_INFO); // GST_LEVEL_LOG, GST_LEVEL_ERROR, GST_LEVEL_WARNING,
                       // GST_LEVEL_INFO, GST_LEVEL_DEBUG*/

  string cfgfilepath1 = "../cmd/channel1.json";
  string cfgfilepath2 = "../cmd/channel2.json";

  ixgParser _parser;
  while (!ctrlc_pressed) {
    if (_parser.fileExist(cfgfilepath1)) {
      cout << "found a file: " << cfgfilepath1 << endl;
      ixgStartChannel(_parser, cfgfilepath1, 0);
      remove(cfgfilepath1.c_str());
    } else if (_parser.fileExist(cfgfilepath2)) {
      cout << "found a file: " << cfgfilepath2 << endl;
      ixgStartChannel(_parser, cfgfilepath2, 1);
      remove(cfgfilepath2.c_str());
    }
  }
  std::cout << "freeing memory" << endl;
  for (int i = 0; i < 4; i++) {
    if (__ixgElementInstance[i] != NULL)
      delete __ixgElementInstance[i];
    if (__gstpipeInstance[i] != NULL)
      delete __gstpipeInstance[i];
    for (int j; j < 5; j++)
      if (__gstOutputInstance[i][j] != NULL)
        delete __gstOutputInstance[i][j];
  }
  std::cout << "freeing memory done" << endl;
  return 0;
}
