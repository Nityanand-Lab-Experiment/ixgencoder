#ifndef PARAMS_INCLUDE
#define PARAMS_INCLUDE

#include "gst.h"
#include "headers.h"

using namespace std;

typedef enum { sdi, ip, file, device, testsrc } _type;

typedef enum { multicast, srt, rtmp } _iptype;

typedef std::string _ixgStr;
typedef int _ixgInt;
typedef bool _ixgbool;

typedef struct sdi {
  _ixgInt device_number;
  _ixgStr format;
} _sdi;

typedef struct multicast {
  _ixgStr ip;
  _ixgInt port;
} _multicast;

typedef struct srt {
  _ixgStr ip;
  _ixgInt port;
  _ixgStr mode;
  _ixgInt latency;
  _ixgStr uri;
} _srt;

typedef struct rtmp {
  _ixgStr url;
} _rtmp;

typedef struct _ip {
  _iptype _ixgIptype;
  _multicast _ixgMulticast;
  _srt _ixgSrt;
  _rtmp _ixgRtmp;
} _ips;

typedef struct file {
  _ixgStr uri;
} _file;

typedef struct record {
  _ixgStr rec_location;
} _rec;

typedef struct delay {
  _ixgStr rec_location;
  _ixgInt time; // in second
  _ixgStr dest_url;
} _delay;

typedef struct _input {
  _ixgbool active;
  _type _ixgtype;
  _sdi _ixgSdi;
  _ip _ixgIp;
  _file _ixgFile;
} _ixgInput;

typedef struct encoding {
  _ixgbool use_hw;
  _ixgStr bitrate;
  _ixgStr max_bitrate;
  _ixgStr codec;
  _ixgStr framerate;
  _ixgStr bitrate_type;
  _ixgStr gop;
  _ixgStr preset;
} _ixgEnc;

typedef struct _output {
  _ixgbool active;
  _type _ixgtype;
  _sdi _ixgSdi;
  _ips _ixgIp;
  _file _ixgFile;
} _ixgOutput;

typedef struct _teeoutput {
  _ixgbool active;
  _type _ixgtype;
  _sdi _ixgSdi;
  _ips _ixgIp;
  _file _ixgFile;
  _rec _ixgRec;
  _delay _ixgDelay;
} _ixgteeOutput;

typedef struct _ixgParameters {
  _ixgInput __ixgInput;
  _ixgEnc __ixgEncoder;
  _ixgOutput __ixgOutput;
  _teeoutput __ixgteeOutput[5];
  _ixgbool __isencode[5];
  _ixgEnc __isEncode[5];

} _ixgElement;

typedef struct registerTeeOutputList {
  _ixgStr name;
} _regOutputList;

namespace ixg {
class ixgElement {
public:
  _ixgElement __ixgElemt;
  _regOutputList __regOutputList[5];
};
} // namespace ixg

#endif