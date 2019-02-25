#include "proxy.h"
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
//using namespace std;

int MyProxy::get_addr_info(const char *host_name, const char *port_num) {
  memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags = AI_PASSIVE;

  status = getaddrinfo(host_name, port_num, &host_info, &host_info_list);
  if (status != 0) {
    std::cerr << "Error: cannot get address info for host" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    return -1;
  }
  return 0;
}

int MyProxy::create_socket() {
  int fd = socket(host_info_list->ai_family, host_info_list->ai_socktype,
                  host_info_list->ai_protocol);
  if (fd == -1) {
    std::cerr << "Error: cannot create socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    return -1;
  }
  return fd;
}

void MyProxy::set_socket_opt(int fd) {
  /*
  struct timeval timeout;
  timeout.tv_sec = 10;
  timeout.tv_usec = 0;

  if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                 sizeof(timeout)) < 0) {
    cerr << "Setsockopt failed" << endl;
  }
  if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
                 sizeof(timeout)) < 0) {
    cerr << "Setsockopt failed" << endl;
  }
  */
  
  int yes = 1;
  status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  
}

int MyProxy::bind_socket() {
  status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    std::cerr << "Error: cannot bind socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    return -1;
  }
  return 0;
}
int MyProxy::listen_socket() {
  status = listen(socket_fd, 100);
  if (status == -1) {
    std::cerr << "Error: cannot listen on socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    return -1;
  }
  return 0;
}

int MyProxy::accept_socket() {
  struct sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);
  // int client_connection_fd;
  client_connection_fd =
      accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
  if (client_connection_fd == -1) {
    std::cerr << "Error: cannot accept connection on socket" << std::endl;
    return -1;
  }
  return client_connection_fd;
}

int MyProxy::set_up_socket() {
  get_addr_info(hostname, port);
  socket_fd = create_socket();
  set_socket_opt(socket_fd);
  bind_socket();
  listen_socket();

  return socket_fd;
}

void MyProxy::close_socket() {
  freeaddrinfo(host_info_list);
  close(socket_fd);
}

int MyProxy::connect_socket() {
  status =
      connect(conn_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    std::cerr << "Error: cannot connect to socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    close(conn_fd);
    return -1;
  }
  return 0;
}

int MyProxy::connect_with_server(const char *Host,const char*port_num) {
  int client_status;
  int client_fd;
  struct addrinfo client_info;
  struct addrinfo *client_info_list;
 
 
  memset(&client_info, 0, sizeof(client_info));
  client_info.ai_family   = AF_UNSPEC;
  client_info.ai_socktype = SOCK_STREAM;

  client_status = getaddrinfo(Host, port_num, &client_info, &client_info_list);
  if (client_status != 0) {
    std::cerr << "Error: cannot get address info for host" << std::endl;
    std::cerr << "  (" << Host << "," << port_num << ")" << std::endl;
    return -1;
  }

  client_fd = socket(client_info_list->ai_family, 
		     client_info_list->ai_socktype, 
		     client_info_list->ai_protocol);
  if (client_fd == -1) {
    std::cerr << "Error: cannot create socket" << std::endl;
    std::cerr << "  (" << Host << "," << port_num << ")" << std::endl;
    return -1;
  }
  
  std::cout << "Connecting to " << Host << " on port " << port_num << "..." << std::endl;
  
  client_status = connect(client_fd, client_info_list->ai_addr, client_info_list->ai_addrlen);
  if (client_status == -1) {
    std::cerr << "Error: cannot connect to socket" << std::endl;
    std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
    return -1;
  }




  //get_addr_info(Host, port_num);
  //conn_fd = create_socket();
  //set_socket_opt(conn_fd);
  //connect_socket();
  conn_fd = client_fd;
  return client_fd;
}

int MyProxy::get_client_fd() { return client_connection_fd; }
int MyProxy::get_socket_fd() { return socket_fd; }
