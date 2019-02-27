#include "response.h"

#include <cstdlib>
#include <iostream>
#include <map>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <vector>

using namespace std;

void response::parse_response() {
  int i = 0;
  string temp;
  string curr = server_response;
  while (curr.size() > 2) {
    std::size_t pos = curr.find("\n");
    curr[pos - 1] = '\0';
    my_response.push_back(curr.substr(0, pos - 1));
    curr = curr.substr(pos + 1);
    // cout << my_response[i] << endl;
    i++;
  }
  // get_expire_time(0);
  // get_date();
}

string response::get_expire_time() {
  string line = server_response;
  if (line.find("Expires:") != string::npos) {
    expire_time = line.substr(line.find("Expires:") + 9);
    expire_time = expire_time.substr(0, expire_time.find("\r\n"));
  }
  return expire_time;
}

string response::get_cache_control() {
  string line = server_response;
  if (line.find("Cache-Control:") != string::npos) {
    cache_control = line.substr(line.find("Cache-Control:") + 15);
    cache_control = cache_control.substr(0, cache_control.find("\r\n"));
  }
  return cache_control;
}

string response::get_date() {
  string line = server_response;
  if (line.find("Date:") != string::npos) {
    date = line.substr(line.find("Date:") + 6);
    date = date.substr(0, date.find("\r\n"));
  }
  return date;
}

time_t response::convert_age() {
  struct tm a;
  strptime(date.c_str(), "%a, %d %b %Y %H:%M:%S GMT", &a);
  age = mktime(&a);
  return age;
}

time_t response::convert_maxAge() {
  struct tm m;
  strptime(expire_time.c_str(), "%a, %d %b %Y %H:%M:%S GMT", &m);
  maxAge = mktime(&m);
  return maxAge;
}

string response::get_response() { return server_response; }

bool response::get_status() {
  if (server_response.find("200 OK") != string::npos) {
    return true;
  } else {
    return false;
  }
}

bool response::check_expire() {
  // modify time with current time
  age = convert_age();
  // cout << "age: " << age << endl;
  maxAge = convert_maxAge();
  // cout << "maxage: " << maxAge << endl;
  modify_date(time(0));
  if (age > maxAge) {
    return true;
  } else {
    return false;
  }
}

void response::modify_date(time_t time) { age = time; }

// bool response::check_valid() { return valid; }
