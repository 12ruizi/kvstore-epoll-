#pragma once
#include <map>
#include <string>
class REquest {
public:
  virtual ~REquest(){};
  char *_data;
  std::string _method;
  std::string _version;
  virtual std::string get_key_URL() = 0;
};

class HttpRequest : public REquest {
public:
  std::string _url;
  std::string _version;
  std::map<std::string, std::string> _headers;
  std::string get_key_URL() override { return _method + _url; }
};
