#ifndef __PROXY_H_
#define __PROXY_H_
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

class MyProxy {
private:
  int status;
  struct addrinfo host_info;
  struct addrinfo *host_info_list;
  int socket_fd;
  const char *hostname = NULL;
  const char *port = "12345";

  const char *server_port = "80";
  int conn_fd;
  int client_connection_fd;

public:
  int get_addr_info(const char *host_name, const char *port_num);
  int create_socket();
  void set_socket_opt(int fd);
  int bind_socket();
  int listen_socket();

  int set_up_socket();

  int accept_socket();
  void close_socket();
  int connect_socket();
  int connect_with_server(const char *Host,const char*port_num);
  int get_client_fd();
  int get_socket_fd();
};

#endif
