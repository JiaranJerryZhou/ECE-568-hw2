#ifndef __REQUEST_H_
#define __REQUEST_H_
#include "cache.h"
#include "proxy.h"
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
class ClientRequest {
private:
  char *hostname;
  std::string client_request;
  std::string Method;
  std::string Host;
  std::vector<std::string> my_request;
  std::string Request;
  std::string http;
  int protocal_type;
  std::string content_length;
  std::string header;
  std::string body;

public:
  void loop_recv_length(int fd, std::vector<char> &to_recv, int recv_length);
  void loop_send();
  void parse_request(const char *request);
  std::string get_header();
  int find_content_length();
  void get_method();
  void check_http();
  void get_hostname();
  void merge_request();
  void handle_request(MyProxy myproxy, cache &mycache);
  void handle_get(MyProxy myproxy, cache &mycache);
  void handle_post(MyProxy myproxy);
  void handle_connect(MyProxy myproxy);
};

#endif
