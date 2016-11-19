#include "concurrent/job.h"
#include "concurrent/producer.h"
#include "server/connection.h"

#ifndef WI_SERVER_LOOPER_H_
#define WI_SERVER_LOOPER_H_

namespace wi {
namespace server {

class looper : wi::concurrent::job {
public:
  void start(short listening_port) {
    listening_port_ = listening_port;

    std::cout << "Hello! Starting server right now.\n";

    socket_fd_ = socket(PF_INET, SOCK_STREAM, 0);
    int program_state = 0;

    // This means: All possible destinations.
    acpt_address_.sin_family = AF_INET;
    acpt_address_.sin_port = htons(listening_port_);
    acpt_address_.sin_addr.s_addr = 0;
  
    // Binds socket to all possible destinations
    program_state = bind(socket_fd_, (sockaddr*)&acpt_address_, sizeof(acpt_address_));
    if ( program_state == -1 ) {
      std::cout << "Cannot create server.\n";
      end();
    }
  
    wi::concurrent::job::start();
    main();
  }

  virtual void main() {
    // Listens to incoming requests.
    int program_state = listen(socket_fd_, 10);
    if (program_state == -1) {
      std::cout << "Error while initializing incoming connection listening.\n";
      end();
    }

    wi::concurrent::producer<wi::server::connection> scheduler;

    std::cout << "Server begins listening on port " << listening_port_ << "\n";
    // Serverr lifecycle
    while(true) {
      sockaddr_in incoming_addr;
      socklen_t incoming_addr_size = sizeof(incoming_addr);

      // Waits for an incoming connection.
      int incoming_sock_fd = accept(socket_fd_, (sockaddr*)&incoming_addr, &incoming_addr_size);
      if (incoming_sock_fd < 0) {
        // accept function stops only when an incoming connection is found or
        // if a system signal is raised. Being here means the latter case happened,
        // so it should be wise to stop the server right now.
        end();
      }
     
      connection conn(incoming_sock_fd);
      scheduler.dispatch(conn);
    }
  
    std::cout << "Shutting down server. Goodbye!\n";
  }

  virtual void end() {
    close(socket_fd_);
  }
private:
  short listening_port_;
  int socket_fd_;
  sockaddr_in acpt_address_;
};

}; // server
}; // wi

#endif

