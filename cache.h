#ifndef __CACHE_H_
#define __CACHE_H_
#include "response.h"
#include <cstdlib>
#include <iostream>
#include <map>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unordered_map>
#include <vector>

using namespace std;

class cache {
private:
  int cap;
  int size;
  // request is key, response is value
  map<string, vector<char>> way;

public:
  cache() : cap(100), size(0){};
  cache(int capacity) : cap(capacity), size(0){};
  void saveCache(string req, vector<char> resp);
  bool getCache(string req, int client_fd);
};

#endif
