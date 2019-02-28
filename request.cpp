#include "request.h"
#include "cache.h"
#include "proxy.h"
#include <arpa/inet.h>
#include <fstream>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <ostream>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

using namespace std;
void ClientRequest::loop_recv_length(int fd, vector<char> &to_recv,
                                     int recv_length) {
  int i = 0;
  int s;
  while ((s = recv(fd, &to_recv.data()[i], 1, 0)) > 0 && s < recv_length) {
    to_recv.resize(to_recv.size() + 1);
    i += s;
    if (i == recv_length) {
      break;
    }
  }
}
void ClientRequest::loop_send() {}
void ClientRequest::parse_request(const char *request) {
  client_request = request;
  string curr(request);
  int j = 0;
  string temp;
  size_t header_end = curr.find("\r\n\r\n");
  temp = curr.substr(0, header_end + 4);
  header = temp;
  while (temp.size() > 2) {
    std::size_t pos = temp.find("\r");
    // temp[pos - 1] = '\0';
    my_request.push_back(temp.substr(0, pos));
    temp = temp.substr(pos + 2);
    j++;
  }
}

int ClientRequest::find_content_length() {
  int length = 0;
  size_t num_header_vect = my_request.size();
  for (size_t i = 0; i < num_header_vect; i++) {
    size_t pos = my_request[i].find("Content-Length");
    if (pos != std::string::npos) {
      size_t pos1 = my_request[i].find(":");
      content_length = my_request[i].substr(pos1 + 1);
      length = atoi(content_length.c_str());
      return length;
    }
  }
  return 0;
}

void ClientRequest::get_method() {
  size_t pos = my_request[0].find(" ");
  Method = my_request[0].substr(0, pos);
  // cout << "M: " << Method << endl;
}

void ClientRequest::check_http() {
  /*
  if (client_request.find("https") != std::string::npos) {
    protocal_type = 1;
  } else if (client_request.find("http") != std::string::npos) {
    protocal_type = 0;
  }
  */
  /*
  size_t pos1 = my_request[0].find(" ");
  http = my_request[0].substr(pos1 + 1);
  size_t pos2 = http.find(":");
  http = http.substr(0, pos2);
  cout << "HTTP TYPE: " << http << endl;
  if (http == "http") {
    protocal_type = 0;
  } else if (http == "https") {
    protocal_type = 1;
  }
  */
}

void ClientRequest::get_hostname() {
  get_method();
  if (Method == "CONNECT") {
    size_t pos = my_request[0].find(" ");
    Host = my_request[0].substr(pos + 1);
    pos = Host.find(" ");
    Host = Host.substr(0, pos);
    pos = Host.find(":");
    Host = Host.substr(0, pos);
    cout << "Host Name: " << Host << endl;
  }
  if (Method == "GET" || Method == "POST") {
    size_t pos1 = my_request[1].find(" ");
    size_t pos2 = my_request[1].find("\r");
    Host = my_request[1].substr(pos1 + 1, pos2 - pos1);
    // cout << "H: " << Host << endl;
  }
}

void ClientRequest::merge_request() {}

void ClientRequest::handle_request(MyProxy myproxy, cache &mycache,
                                   int thread_id, const char *request) {
  client_request = request;
  if (client_request.find("GET") != std::string::npos) {
    Method = "GET";
  }
  if (client_request.find("POST") != std::string::npos) {
    Method = "POST";
  }
  if (client_request.find("CONNECT") != std::string::npos) {
    Method = "CONNECT";
  }
  size_t pos = client_request.find("Host");
  if (pos != std::string::npos) {
    Host = client_request.substr(pos);
    size_t pos2 = Host.find(" ");
    Host = Host.substr(pos2 + 1);
    size_t pos3 = Host.find("\r");
    // Host[pos3] = '\0';
    Host = Host.substr(0, pos3);

    if (Method == "CONNECT") {
      size_t com = Host.find(":");
      if (com != std::string::npos) {
        Host = Host.substr(0, com);
        // Host.push_back('\0');
        // cout << com << endl;
      }
    }
    // cout << Host << endl;
  }

  // print current time
  time_t curr_time;
  struct tm *time_info;
  time(&curr_time);
  time_info = localtime(&curr_time);
  cout << thread_id << ": ";
  cout << '"' << my_request[0] << " from ";
  struct hostent *host_entry;
  char myhostname[512];
  gethostname(myhostname, sizeof(myhostname));
  host_entry = gethostbyname(myhostname);
  char *ip = inet_ntoa(*((struct in_addr *)host_entry->h_addr_list[0]));
  cout << ip << " @ ";
  cout << asctime(time_info) << endl;
  std::ofstream file;
  file.open("proxy.log", std::ios_base::app | std::ios_base::out);
  file << thread_id << " : " << '"' << my_request[0] << '"' << " from ";
  file << ip << " @ " << asctime(time_info) << endl;
  file.close();

  // get_hostname();
  if (Method == "GET") {
    handle_get(myproxy, mycache, thread_id);
  } else if (Method == "POST") {
    handle_post(myproxy);
  } else if (Method == "CONNECT") {
    handle_connect(myproxy);
  }
  return;
}

void ClientRequest::handle_get(MyProxy myproxy, cache &mycache, int thread_id) {
  // check_http();
  int conn_fd;
  int client_connection_fd = myproxy.get_client_fd();
  /*
  if (protocal_type == 0) {
    conn_fd = myproxy.connect_with_server(Host.c_str(), "80");
  } else if (protocal_type == 1) {
    conn_fd = myproxy.connect_with_server(Host.c_str(), "443");
  }
  */
  conn_fd = myproxy.connect_with_server(Host.c_str(), "80");
  int se = 0;
  int by;
  size_t fhost = client_request.find("Host");
  string temp = client_request.substr(fhost);
  size_t sec = temp.find("\r\n");
  size_t fagent = client_request.find("User-Agent");
  string cu_request = client_request.substr(0, fhost + sec);
  if (fagent != std::string::npos) {
    string agent = client_request.substr(fagent);
    size_t agend = agent.find("\r\n");
    agent = agent.substr(0, agend);
    cu_request.append("\r\n");
    cu_request.append(agent);
  }
  // cout << "My_request now: " << cu_request << endl;
  cu_request.append("\r\n\r\n");
  // int end = client_request.find("\n");
  // string url = client_request.substr(0, end);
  string url = my_request[0];
  if (!mycache.getCache(url, client_connection_fd, thread_id)) {
    vector<char> response(1, 0);
    int nbytes;
    int index = 0;
    // send request to server
    while (se < (int)cu_request.size() - 1) {
      by = send(conn_fd, cu_request.c_str(), cu_request.size() - se, 0);
      se += by;
    }
    //-------recv header first--------
    while ((nbytes = recv(conn_fd, &response.data()[index], 1, 0)) > 0) {
      if (response.size() > 4) {
        size_t size = response.size();
        if (response.back() == '\n' && response[size - 2] == '\r' &&
            response[size - 3] == '\n' &&
            response[size - 4] == '\r') { // find the end of header
          // close(conn_fd);
          break;
        }
      }
      response.resize(response.size() + 1);
      index += nbytes;
    }
    std::ofstream file;
    file.open("proxy.log", std::ios_base::app | std::ios_base::out);
    file << thread_id << " : Received HTTP 200 OK from " << Host << endl;
    file.close();
    // cout << "Header size: " << response.size() << endl;
    string response_header(response.begin(), response.end());

    int sbyte;
    int send_index = 0;
    // cout << "Response Size: before merge " << response.size() << endl;

    while (send_index < (int)response.size() - 1) {
      sbyte = send(client_connection_fd, &response.data()[send_index],
                   response.size() - send_index, 0);
      send_index += sbyte;
      // cout << sbyte << endl;
    }
    vector<char> response_body(1, 0);
    int receiving;
    int i = 0;
    //---------find the content-length------
    std::size_t found = response_header.find("Content-Length");
    if (found != std::string::npos) {
      string temp = response_header.substr(found);
      found = temp.find(" ");
      temp = temp.substr(found + 1);
      found = temp.find("\r");
      temp = temp.substr(0, found);
      int response_body_len = atoi(temp.c_str());
      if (response_body_len != 0) {
        // cout << "Found len!" << endl;
        /*
        while ((receiving = recv(conn_fd, &response_body.data()[i], 1, 0)) >
        0&&i<response_body_len) { cout<<receiving<<endl; if(i ==
        response_body_len-1){ break;
          }
          response_body.resize(response_body.size() + 1);
          i += receiving;
        }
        */
        vector<char> sth;
        sth.resize(response_body_len);
        receiving =
            recv(conn_fd, &sth.data()[0], response_body_len, MSG_WAITALL);
        // cout << receiving << endl;
        // response.insert(response.end(), sth.begin(), sth.end());
        // cout << sth.data() << endl;
        close(conn_fd);
        // int s =
        send(client_connection_fd, &sth.data()[0], response_body_len, 0);

        // cout << s << endl;
      }

    } else if (response_header.find(
                   "chunked")) { //----------recv till the end------
      while ((receiving = recv(conn_fd, &response_body.data()[i], 1, 0)) > 0) {
        if (response_body.size() > 5) {
          size_t size = response_body.size();
          if (response_body.back() == '\n' && response_body[size - 2] == '\r' &&
              response_body[size - 3] == '\n' &&
              response_body[size - 4] == '\r' &&
              response_body[size - 5] == '0') { // find the end of header
            close(conn_fd);
            break;
          }
        }
        // response_body.push_back('\0');
        response_body.resize(response_body.size() + 1);
        i += receiving;
      }
      send(client_connection_fd, &response_body.data()[0], response_body.size(),
           0);
      // response.insert(response.end(), response_body.begin(),
      // response_body.end());
    } else {
      while ((receiving = recv(conn_fd, &response_body.data()[i], 1, 0)) > 0) {
        if (response_body.size() > 4) {
          size_t size = response_body.size();
          if (response_body.back() == '\n' && response_body[size - 2] == '\r' &&
              response_body[size - 3] == '\n' &&
              response_body[size - 4] == '\r') { // find the end of header
            close(conn_fd);
            break;
          }
        }
        // response_body.push_back('\0');
        response_body.resize(response_body.size() + 1);
        i += receiving;
      }

      send(client_connection_fd, &response_body.data()[0], response_body.size(),
           0);
    }
    //--------send all--------------
    /*
    int client_connection_fd = myproxy.get_client_fd();
    int sbyte;
    int send_index = 0;
    cout<<"Response Size: before merge "<<response.size()<<endl;


    cout<<response.size()<<endl;

    while(send_index<(int)response.size()-1){
      sbyte = send(client_connection_fd,
    &response.data()[send_index],response.size()-send_index,0); send_index
    +=sbyte; cout<<sbyte<<endl;
    }
    */
    // close(conn_fd);
    // close(client_connection_fd);
    // cout << response.data() << endl;
    mycache.saveCache(url, response, thread_id);
    cout << "returned from save cache" << endl;
  }
  return;
}

string ClientRequest::get_header() { return header; }

void ClientRequest::handle_post(MyProxy myproxy) {
  //  check_http();
  int conn_fd;
  /*
  if (protocal_type == 0) {
    conn_fd = myproxy.connect_with_server(Host.c_str(), "80");
  }
  if (protocal_type == 1) {
    conn_fd = myproxy.connect_with_server(Host.c_str(), "443");
  }
  */
  conn_fd = myproxy.connect_with_server(Host.c_str(), "80");
  //  merge_request();
  int length = find_content_length();
  cout << length << endl;
  vector<char> content(1, 0);

  int i = 0;
  int s;
  while ((s = recv(myproxy.get_client_fd(), &content.data()[i], 1, 0)) > 0 &&
         i < length) {
    content.resize(content.size() + 1);
    i += s;
    if (i == length) {
      break;
      cout << i << endl;
    }
  }
  cout << i << endl;
  // content.resize(content.size() - 1);
  send(conn_fd, header.c_str(), header.size(), 0);
  int j = 0;
  while ((s = send(conn_fd, &content.data()[j], 1, 0)) > 0) {
    if (j == length - 1) {
      break;
    }
    j += s;
    // content.resize(content.size() + 1);
  }
  if (length == 0) {
    cerr << "Wrong length" << endl;
    exit(EXIT_FAILURE);
  }

  vector<char> response(1, 0);
  int nbytes;
  int index = 0;

  while ((nbytes = recv(conn_fd, &response.data()[index], 1, 0)) > 0) {
    response.resize(response.size() + 1);
    index += nbytes;
    if (index == length) {
      break;
    }
  }
  cout << "Respose: " << endl;
  int client_connection_fd = myproxy.get_client_fd();
  int sbyte;
  int send_index = 0;
  while ((sbyte = send(client_connection_fd, &response.data()[send_index], 100,
                       0)) > 0) {
    send_index += 100;
  }
  cout << response.data() << endl;
  close(conn_fd);
  return;
}

void ClientRequest::handle_connect(MyProxy myproxy) {
  cout << "CONNECT HOST: " << Host << endl;
  int conn_fd = myproxy.connect_with_server(Host.c_str(), "443");
  int client_fd = myproxy.get_client_fd();
  // cout<<client_fd<<endl;
  // cout<<conn_fd<<endl;
  string mes = "HTTP/1.1 200 OK\r\n\r\n";
  send(myproxy.get_client_fd(), mes.c_str(), mes.size(), 0);
  int ready = 0;
  while (1) {
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(conn_fd, &read_fds);
    FD_SET(client_fd, &read_fds);

    int maxfd = client_fd;
    int fd_list[2] = {client_fd, conn_fd};
    if (maxfd < conn_fd) {
      maxfd = conn_fd;
    }

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    ready = select(maxfd + 1, &read_fds, NULL, NULL, &tv);

    if (ready == -1) {
      cerr << "Failure on select----player\n";
      exit(EXIT_FAILURE);
    }
    if (ready == 0) {
      break;
    }
    int sign_break = 0;
    // cout <<"SELEXT"<<ready<<endl;

    for (int i = 0; i < 2; i++) {
      if (FD_ISSET(fd_list[i], &read_fds)) {
        std::vector<char> response(500000, 0);
        int nbytes;
        nbytes = recv(fd_list[i], &response.data()[0], 500000, 0);
        response.resize(nbytes);
        // std::cout << "Receice " << nbytes;
        if (nbytes == 0) {
          sign_break = 1;
          close(conn_fd);
          close(client_fd);
          break;
        }
        int other;
        if (i == 0) {
          other = 1;
        } else {
          other = 0;
        }
        int sent = 0;
        while (sent < nbytes) {
          int temp =
              send(fd_list[other], &response.data()[sent], nbytes - sent, 0);
          sent += temp;
        }
        // cout << " Sent " << sent << " bytes" << endl;
      }
    }
    if (sign_break == 1) {

      break;
    }
  }
  return;
  // cout<<"CONNECT DONE!"<<endl;
  // close(conn_fd);
}
