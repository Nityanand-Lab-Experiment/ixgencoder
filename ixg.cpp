
#include "includes/gst.h"
#include "includes/utils.h"
#include "jsonParser.h"
#include <iostream>
#include <thread>

volatile sig_atomic_t ctrlc_pressed = 0;

ixg::ixgElement *__ixgElementInstance[4] = {NULL};
ixgStream::gstBasePipe *__gstpipeInstance[4] = {NULL};

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
}

// Signal handler for Ctrl+C (SIGINT)
void signalHandler(int signum) {
  if (signum == SIGINT) {
    std::cout << "Ctrl+C pressed. Exiting..." << std::endl;
    ctrlc_pressed = 1;
  }
}

int main(int argc, char *argv[]) {

  signal(SIGINT, signalHandler);
  gst_init(&argc, &argv);

  gst_debug_set_active(true);
  gst_debug_set_default_threshold(
      GST_LEVEL_INFO); // GST_LEVEL_LOG, GST_LEVEL_ERROR, GST_LEVEL_WARNING,
                       // GST_LEVEL_INFO, GST_LEVEL_DEBUG

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
  std::cout << "free memory" << endl;
  for (int i = 0; i < 4; i++) {
    if (__ixgElementInstance[i] != NULL)
      delete __ixgElementInstance[i];
    if (__gstpipeInstance[i] != NULL)
      delete __gstpipeInstance[i];
  }
  return 0;
}
