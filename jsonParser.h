#ifndef JSONPARSER_INCLUDE
#define JSONPARSER_INCLUDE

#include "includes/headers.h"
using namespace std;

class ixgParser {
public:
  Json::Reader reader;             // for reading the data
  Json::Value root;                // for modifying and storing new values
  Json::StyledStreamWriter writer; // for writing in json files

public:
  bool fileExist(string filePath);
  Json::Value parsering(string fpath);
};

void getJsonValue(void *elemt, Json::Value root, int ch);

#endif