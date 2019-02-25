#include "request.h"
#include "cache.h"
#include "proxy.h"
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
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
    std::size_t pos = temp.find("\n");
    temp[pos - 1] = '\0';
    my_request.push_back(temp.substr(0, pos - 1));
    temp = temp.substr(pos + 1);
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
    Host = my_request[1].substr(pos1 + 1, pos2);
    // cout << "H: " << Host << endl;
  }
}

void ClientRequest::merge_request() {}

void ClientRequest::handle_request(MyProxy myproxy, cache &mycache) {
  get_method();
  get_hostname();
  // merge_request();
  if (Method == "GET") {
    handle_get(myproxy, mycache);
  } else if (Method == "POST") {
    handle_post(myproxy);
  } else if (Method == "CONNECT") {
    handle_connect(myproxy);
  }
}

void ClientRequest::handle_get(MyProxy myproxy, cache &mycache) {
  check_http();
  int conn_fd;
  int client_connection_fd = myproxy.get_client_fd();
  if (protocal_type == 0) {
    conn_fd = myproxy.connect_with_server(Host.c_str(), "80");
  }
  if (protocal_type == 1) {
    conn_fd = myproxy.connect_with_server(Host.c_str(), "443");
  }
  cout << "request is" << endl;
  cout << my_request[0] << endl;
  if (!mycache.getCache(my_request[0], client_connection_fd)) {
    send(conn_fd, client_request.c_str(), client_request.size(), 0);
    vector<char> response(1, 0);
    int nbytes;
    int index = 0;
    while ((nbytes = recv(conn_fd, &response.data()[index], 1, MSG_WAITALL)) >
           0) {
      // cout<<"index "<<index<<"response[index]: "<<response.data()<<endl;
      if (response.size() > 5) {
        size_t size = response.size();
        if (response.back() == '\n' && response[size - 2] == '\r' &&
            response[size - 3] == '\n' && response[size - 4] == '\r' &&
            response[size - 5] == '0') { // find the end of chunk
          break;
        }
      }
      response.resize(response.size() + 1);
      index += nbytes;
    }
    cout << "Respose: " << endl;
    int sbyte;
    int send_index = 0;
    while ((sbyte = send(client_connection_fd, &response.data()[send_index],
                         100, 0)) == 100) {
      send_index += 100;
    }
    cout << response.data() << endl;
    mycache.saveCache(my_request[0], response);
    close(conn_fd);
  }
}

string ClientRequest::get_header() { return header; }

void ClientRequest::handle_post(MyProxy myproxy) {
  check_http();
  int conn_fd;
  if (protocal_type == 0) {
    conn_fd = myproxy.connect_with_server(Host.c_str(), "80");
  }
  if (protocal_type == 1) {
    conn_fd = myproxy.connect_with_server(Host.c_str(), "443");
  }
  merge_request();
  int length = find_content_length();
  vector<char> content(1, 0);

  int i = 0;
  int s;
  while ((s = recv(myproxy.get_client_fd(), &content.data()[i], 1, 0)) > 0 &&
         s < length) {
    content.resize(content.size() + 1);
    i += s;
    if (i == length) {
      break;
      cout << i << endl;
    }
  }
  send(conn_fd, header.c_str(), header.size(), 0);
  int j = 0;
  while ((s = send(conn_fd, &content.data()[j], 100, 0)) == 100) {
    j += s;
    if (j == length) {
      break;
    }
  }

  if (length == 0) {
    cerr << "Wrong length" << endl;
    exit(EXIT_FAILURE);
  }

  vector<char> response(100, 0);
  int nbytes;
  int index = 0;

  while ((nbytes = recv(conn_fd, &response.data()[index], 100, 0)) > 0) {
    response.resize(response.size() + 100);
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
                       0)) == 100) {
    send_index += 100;
  }
  cout << response.data() << endl;
  close(conn_fd);
}

void ClientRequest::handle_connect(MyProxy myproxy) {
  cout << "CONNECT HOST: " << Host << endl;
  int conn_fd = myproxy.connect_with_server(Host.c_str(), "443");
  int client_fd = myproxy.get_client_fd();
  cout << client_fd << endl;
  cout << conn_fd << endl;
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
        // response.resize(500000);
        int nbytes;
        // int index = 0;
        nbytes = recv(fd_list[i], &response.data()[0], 500000, 0);
        response.resize(nbytes);
        if (nbytes == 0) {
          sign_break = 1;
          break;
        }
        int other;
        if (i == 0) {
          other = 1;
        } else {
          other = 0;
        }
        send(fd_list[other], &response.data()[0], nbytes, 0);
      }
    }
    if (sign_break == 1) {
      break;
    }
  }
  cout << "CONNECT DONE!" << endl;
  close(conn_fd);
}
