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
  map<string, pair<vector<char>, int>> way;
  // pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

public:
  cache() : cap(10), size(0){};
  cache(int capacity) : cap(capacity), size(0){};
  void saveCache(string req, vector<char> resp, int thread_id);
  bool getCache(string req, int client_fd, int thread_id);
};

#endif
