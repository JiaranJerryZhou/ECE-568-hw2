#include <cstdlib>
#include <fstream>
#include <iostream>
#include <mutex>
#include <netdb.h>
#include <netinet/in.h>
#include <ostream>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <time.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

#include "cache.h"

#include "response.h"
using namespace std;

void cache::saveCache(string req, vector<char> r, int thread_id) {

  pair<vector<char>, int> info(r, thread_id);
  //  pthread_mutex_lock(&lock);
  // unordered_map<string, string>::iterator pos = way.find(req);
  string resp(r.begin(), r.end());
  // cout << "to save" << endl;
  // cout << resp << endl;
  response curr(resp);
  // cout << "create response" << endl;
  // curr.parse_response();
  // cout << "parse response" << endl;
  string control = curr.get_cache_control();
  // cout << "control" << endl;
  // cout << control << endl;
  string expire_time = curr.get_expire_time();
  // cout << expire_time << endl;
  string curr_time = curr.get_date();
  // cout << curr_time << endl;
  // cannot be cached
  if (control.find("no cache") != string::npos ||
      control.find("no store") != string::npos) {
    cout << thread_id << ": ";
    cout << "not cacheable because " << control << endl;
    std::ofstream file;
    file.open("proxy.log", std::ios_base::app | std::ios_base::out);
    file << thread_id << " : "
         << "not cacheable because " << control << endl;
    file.close();
    return;
  }
  // not 200 OK
  if (!curr.get_status()) {
    cout << thread_id << ": ";
    cout << "not cacheable because not 200 OK" << endl;
    std::ofstream file;
    file.open("proxy.log", std::ios_base::app | std::ios_base::out);
    file << thread_id << " : "
         << "not cacheable because not 200 OK" << endl;
    file.close();
    return;
  }
  // evict by LRU
  if (size == cap) {
    string to_evict = way.begin()->first;
    string url = to_evict.substr(to_evict.find(' ') + 1);
    url = url.substr(0, url.find(' '));
    cout << "(no id) ";
    cout << "NOTE evicted " << url << " from cache" << endl;
    std::ofstream file;
    file.open("proxy.log", std::ios_base::app | std::ios_base::out);
    file << "(no id)"
         << "Note evicted  " << url << " from cache" << endl;
    file.close();
    way.erase(way.begin());
    pair<string, pair<vector<char>, int>> curr = make_pair(req, info);
    way.insert(curr);
    return;
  } else {
    // will expire
    if (!expire_time.empty()) {
      // need revalidate
      if (control.find("must-revalidate") != string::npos) {
        cout << thread_id << ": ";
        cout << "cached, but requires revalidation" << endl;
        std::ofstream file;
        file.open("proxy.log", std::ios_base::app | std::ios_base::out);
        file << thread_id << " : "
             << "cached, but requires revalidation" << endl;
        file.close();
      }
      cout << thread_id << ": ";
      cout << "cached, but expires at " << expire_time << endl;
      std::ofstream file;
      file.open("proxy.log", std::ios_base::app | std::ios_base::out);
      file << thread_id << " : "
           << "cached, but expires at " << expire_time << endl;
      file.close();
    } else {
      cout << thread_id << ": ";
      cout << "cached" << endl;
      std::ofstream file;
      file.open("proxy.log", std::ios_base::app | std::ios_base::out);
      file << thread_id << " : "
           << "cached" << endl;
      file.close();
    }
    pair<string, pair<vector<char>, int>> curr = make_pair(req, info);
    way.insert(curr);
    for (auto c : way) {
      cout << "in way: " << endl;
      cout << c.second.second << endl;
      cout << c.first << endl;
      cout << c.second.first.data() << endl;
    }
    //    cout << req << endl;
    // cout << resp << "has been cached" << endl;
    size++;
    return;
  }
  // pthread_mutex_unlock(&lock);
}

bool cache::getCache(string req, int client_fd, int thread_id) {
  // find req in cache
  // pthread_mutex_lock(&lock);
  for (auto c : way) {
    cout << "in way: " << endl;
    cout << c.second.second << endl;
    cout << c.first << endl;
    cout << c.second.first.data() << endl;
  }
  if (way.find(req) != way.end()) {
    int id = way[req].second;
    vector<char> r = way[req].first;
    string resp(r.begin(), r.end());
    response curr(resp);
    // curr.parse_response();
    string control = curr.get_cache_control();
    if (control.find("must-revalidate") != string::npos) {
      cout << id << ": ";
      cout << "in cache, requires validation" << endl;
      std::ofstream file;
      file.open("proxy.log", std::ios_base::app | std::ios_base::out);
      file << id << " : "
           << "in cache, requires validation" << endl;
      file << id << " : "
           << "NOTE evicted " << req << " from cache" << endl;
      file.close();
      way.erase(way.find(req));
      return false;
    }
    // never expires
    string expire_time = curr.get_expire_time();
    string curr_time = curr.get_date();
    if (expire_time.empty()) {
      cout << id << ": ";
      cout << "in cache, valid" << endl;
      cout << "Responding HTTP/1.1 200 OK" << endl;
      std::ofstream file;
      file.open("proxy.log", std::ios_base::app | std::ios_base::out);
      file << id << " : "
           << "in cache, valid" << endl;
      file << id << " : "
           << "Responding HTTP/1.1 200 OK" << endl;

      file.close();
      //      send(client_fd, &resp, resp.size(), 0);

      int sbyte;
      int send_index = 0;
      int size = r.size();

      while (send_index < size) {
        sbyte = send(client_fd, &r[send_index], size - send_index, 0);
        send_index += sbyte;
        cout << sbyte << endl;
      }

      /*
      while ((sbyte = send(client_fd, &r[send_index], 100, 0)) == 100) {
        send_index += 100;
      }
      */
      return true;
    } else {
      if (curr.check_expire()) {
        cout << id << ": ";
        cout << "in cache, but expires at " << expire_time << endl;
        std::ofstream file;
        file.open("proxy.log", std::ios_base::app | std::ios_base::out);
        file << id << " : "
             << "in cache, but expires at " << expire_time << endl;
        file << id << " : "
             << "NOTE evicted " << req << " from cache" << endl;
        file.close();

        way.erase(way.find(req));
        return false;
      } else {
        cout << id << ": ";
        cout << "in cache, valid" << endl;
        std::ofstream file;
        file.open("proxy.log", std::ios_base::app | std::ios_base::out);
        file << id << " : "
             << "in cache, valid" << endl;
        file.close();

        //        send(client_fd, &resp, resp.size(), 0);

        int sbyte;
        int send_index = 0;
        // const char *r = resp.c_str();
        int size = r.size();
        while (send_index < size) {
          sbyte = send(client_fd, &r[send_index], size - send_index, 0);
          send_index += sbyte;
          cout << sbyte << endl;
        }

        /*
          while ((sbyte = send(client_fd, &r[send_index], 100, 0)) == 100) {
            send_index += 100;
          }
        */
        // cout << resp << endl;
        close(client_fd);
        return true;
      }
    }
  } else {
    cout << thread_id << ": ";
    cout << "not in cache" << endl;
    std::ofstream file;
    file.open("proxy.log", std::ios_base::app | std::ios_base::out);
    file << thread_id << " : "
         << "not in cache" << endl;
    file.close();

    return false;
  }
  // pthread_mutex_unlock(&lock);
}
