#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "concurrent/job.h"

#ifndef WI_SERVER_CONNECTION_H_
#define WI_SERVER_CONNECTION_H_

namespace wi {
namespace server {

class connection : public concurrent::job {
public:
  connection(int client_socket) : client_socket_(client_socket) { 
    start(); 
  }
  
  virtual ~connection() {
    end();
  }

  virtual void main() { 

    static char srv_rsp[]="HTTP/1.1 200 OK\nServer: csrv/0.1.0\nContent-type=text/html\nContent-length=12\n\nHello world!\n\0";


    // A connection is estabilished. Just waits for some data from the
    // other side.
    ssize_t recv_bytes = 0;
    char recv_buff[1024];
    recv_bytes = recv(client_socket_, recv_buff, 1024, 0);
    
    // This code doesn't care about actual incoming message size.
    // It only takes into account the first packet. And sends an
    // hello world back.
    ssize_t sent_bytes = send(client_socket_, srv_rsp, strlen(srv_rsp), 0);
    if (sent_bytes == -1) {
      std::cout << "Could not send data to client.\n";
    } else {
      std::cout << "Sending data to client.\n";
    }

    // Closes the connection and wait for other ones.
    close(client_socket_);
    end();
  }
private:
  int client_socket_;
};


}; // server
}; // wi

#endif // WI_SERVER_CONNECTION_H_

