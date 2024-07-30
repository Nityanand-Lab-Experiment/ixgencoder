
#include "jsonParser.h"
#include "includes/gst.h"

Json::Value ixgParser::parsering(string fpath) {
  ifstream json(fpath);
  if (!json.is_open()) {
    cout << "File doesn't exits" << endl;
    exit(0);
  }

  // check if there is any error is getting data from the json file
  if (!reader.parse(json, root)) {
    cout << "failed:" << reader.getFormattedErrorMessages();
    // exit(1);
    cout << "Try Again" << endl;
  }
  json.close();
  return root;
}

bool ixgParser::fileExist(string filePath) {
  return (access(filePath.c_str(), F_OK) != -1);
}

void getJsonValue(void *elemt, Json::Value root, int ch) {
  ixg::ixgElement **__ixgElementInstance =
      static_cast<ixg::ixgElement **>(elemt);

  _ixgInt cpipe = -1;
  // input parameter
  if ("SDI" == root["input"]["type"].asString()) {
    __ixgElementInstance[ch]->__ixgElemt.__ixgInput._ixgtype = sdi;
    __ixgElementInstance[ch]->__ixgElemt.__ixgInput.active =
        root["input"]["active"].asBool();
    __ixgElementInstance[ch]->__ixgElemt.__ixgInput._ixgSdi.device_number =
        root["input"]["config"]["source"].asInt();
    __ixgElementInstance[ch]->__ixgElemt.__ixgInput._ixgSdi.format =
        root["input"]["config"]["format"].asString();
  }

  // Encoding parameter
  __ixgElementInstance[ch]->__ixgElemt.__ixgEncoder.bitrate =
      root["encoding"]["bitrate"].asString();
  __ixgElementInstance[ch]->__ixgElemt.__ixgEncoder.max_bitrate =
      root["encoding"]["max_bitrate"].asString();
  __ixgElementInstance[ch]->__ixgElemt.__ixgEncoder.gop =
      root["encoding"]["gop_size"].asString();
  __ixgElementInstance[ch]->__ixgElemt.__ixgEncoder.bitrate_type =
      root["encoding"]["bitrate_mode"].asString();
  __ixgElementInstance[ch]->__ixgElemt.__ixgEncoder.preset =
      root["encoding"]["preset"].asString();

  __ixgElementInstance[ch]->__ixgElemt.__ixgEncoder.use_hw =
      root["encoding"]["use_hardware"].asBool();

  // output parameter
  // std::string outputType = root["output"]["type"].asString();
  bool outputActive = root["output"]["active"].asBool();
  if (outputActive) {
    __ixgElementInstance[ch]->__ixgElemt.__ixgOutput._ixgtype = ip; // todo
    __ixgElementInstance[ch]->__ixgElemt.__ixgOutput._ixgIp._ixgMulticast.ip =
        root["output"]["config"]["IP"].asString();
    __ixgElementInstance[ch]->__ixgElemt.__ixgOutput._ixgIp._ixgMulticast.port =
        root["output"]["config"]["Port"].asInt();
  }

  // Access teeOutputs section
  for (const auto &teeOutput : root["teeOutputs"]) {
    std::string type = teeOutput["type"].asString();
    bool active = teeOutput["active"].asBool();
    cpipe++;
    __ixgElementInstance[ch]->__regOutputList[cpipe].name = type;

    if (active) {
      if (type == "Multicast") {
        __ixgElementInstance[ch]
            ->__ixgElemt.__ixgteeOutput[cpipe]
            ._ixgIp._ixgMulticast.ip = teeOutput["config"]["IP"].asString();
        __ixgElementInstance[ch]
            ->__ixgElemt.__ixgteeOutput[cpipe]
            ._ixgIp._ixgMulticast.port = teeOutput["config"]["Port"].asInt();
      } else if (type == "SRT") {
        __ixgElementInstance[ch]
            ->__ixgElemt.__ixgteeOutput[cpipe]
            ._ixgIp._ixgSrt.ip = teeOutput["config"]["IP"].asString();
        __ixgElementInstance[ch]
            ->__ixgElemt.__ixgteeOutput[cpipe]
            ._ixgIp._ixgSrt.port = teeOutput["config"]["port"].asInt();
        __ixgElementInstance[ch]
            ->__ixgElemt.__ixgteeOutput[cpipe]
            ._ixgIp._ixgSrt.uri = teeOutput["config"]["uri"].asString();
      } else if (type == "Record") {
        __ixgElementInstance[ch]
            ->__ixgElemt.__ixgteeOutput[cpipe]
            ._ixgRec.rec_location = teeOutput["config"]["location"].asString();
      } else if (type == "Delay") {
        __ixgElementInstance[ch]
            ->__ixgElemt.__ixgteeOutput[cpipe]
            ._ixgDelay.time = teeOutput["delay"].asInt();
        std::string source =
            teeOutput["config"]["source"]
                .asString(); // edit for source location for reading file
        __ixgElementInstance[ch]
            ->__ixgElemt.__ixgteeOutput[cpipe]
            ._ixgDelay.rec_location =
            teeOutput["config"]["location"].asString();
        __ixgElementInstance[ch]
            ->__ixgElemt.__ixgteeOutput[cpipe]
            ._ixgDelay.dest_url = teeOutput["config"]["url"].asString();
      }
    }
  }
}