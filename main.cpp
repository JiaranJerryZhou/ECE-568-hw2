#include "cache.h"
#include "proxy.h"
#include "request.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <time.h>
#include <unistd.h>
#include <vector>

cache mycache;

// using namespace std;
void my_proxy_func(int client_connection_fd, MyProxy myproxy) {
  int find = 0;
  std::vector<char> header(1, 0);
  int index = 0;
  int nbytes;
  while ((nbytes = recv(client_connection_fd, &header.data()[index], 1,
                        MSG_WAITALL)) > 0) {
    if (header.size() > 4) {
      if (header.back() == '\n' && header[header.size() - 2] == '\r' &&
          header[header.size() - 3] == '\n' &&
          header[header.size() - 4] == '\r') {
        std::cout << "GOT HEADER!" << std::endl;
        find = 1;
        break;
      }
    }

    header.resize(header.size() + 1);
    index += nbytes;
  }
  // if(header.size()<2){
  // continue;
  //}
  if (find == 1) {
    std::cout << header.data() << std::endl;
    std::string header_mes(header.begin(), header.end());
    if (header_mes == "") {
      return;
    }
    std::cout << "~~~~~~```ALL RECEVIE~~~~" << std::endl;
    ClientRequest clientrequest;
    //clientrequest.parse_request(header_mes.c_str());
    clientrequest.handle_request(myproxy, mycache,header_mes.c_str());
    return;
  } else {
    std::cerr << "`````INVAILD HEADER!``````" << std::endl;
    return;
  }
}

int main(int argc, char *argv[]) {
  MyProxy myproxy;
  cache mycache;
  int socket_fd = myproxy.set_up_socket();
  std::cout << "Socket " << socket_fd << " set up done!" << std::endl;
  std::cout << "Waiting for connection...on port 12345" << std::endl;
  while (1) {

    std::cout << "SOCKET FD: " << socket_fd << std::endl;
    int client_connection_fd = myproxy.accept_socket();
    // std::cout << "Client is " << client_connection_fd << endl;
    if (client_connection_fd == -1) {
      continue;
    }
    try {
      
      int find=0;
      std::vector<char> header(1, 0);
      int index = 0;
      int nbytes;
      while ((nbytes = recv(client_connection_fd, &header.data()[index], 1,
      MSG_WAITALL)) > 0) { if(header.size()>4){
          if(header.back()=='\n'&&header[header.size()-2]=='\r'&&header[header.size()-3]=='\n'&&header[header.size()-4]=='\r'){
            std::cout<<"GOT HEADER!"<<std::endl;
            find = 1;
            break;
          }
        }

        header.resize(header.size() + 1);
        index += nbytes;
      }
      //if(header.size()<2){
      //continue;
      //}
      if(find==1){
        std::cout << header.data() << std::endl;
        std::string header_mes(header.begin(),header.end());
        if(header_mes==""){
          continue;
        }
        std::cout<< "~~~~~~```ALL RECEVIE~~~~"<<std::endl;
        ClientRequest clientrequest;
        //clientrequest.parse_request(header_mes.c_str());
        //std::cout <<"**************xAfter parse************"<<std::endl;
        clientrequest.handle_request(myproxy,mycache,header_mes.c_str());
      }
      else{
        std::cerr<<"$$$$`````INVAILD HEADER!``````$$$$"<<std::endl;
        continue;
      }
      /*
      std::thread my_thread(my_proxy_func, client_connection_fd, myproxy);
      // int check = my_proxy_func(client_connection_fd,myproxy);
      my_thread.detach();
      // if(check==-1){
      // continue;
      //}
      */
      std::cout << "-------DONE-------" << std::endl;
    } catch (std::exception &e) {
      close(socket_fd);
    }
  }
  return 0;
}
