#ifndef __RESPONSE_H_
#define __RESPONSE_H_
// #include "../ECE568/proxy/proxy.h"
#include <cstdlib>
#include <iostream>
#include <map>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <vector>

using namespace std;

class response {
private:
  string server_response;
  string expire_time;
  vector<string> my_response;
  string cache_control;
  string date;
  bool valid;
  time_t maxAge;
  time_t age;
  bool expire;

public:
  response(string res) : server_response(res){};
  void parse_response();
  string get_expire_time();
  string get_cache_control();
  // bool validation();
  string get_date();
  time_t convert_age();
  time_t convert_maxAge();
  bool get_status();
  string get_response();
  bool check_expire();
  // every time we visit cache, change it with system time
  // we can also modify time by hand
  void modify_date(time_t time);
  // bool check_valid();
};
#endif
