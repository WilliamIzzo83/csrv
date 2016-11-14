#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <list>
#include <array>
#include <assert.h>
#include <thread>
#include <mutex>

static short listen_port = 4040;
static char srv_rsp[]="HTTP/1.1 200 OK\nServer: csrv/0.1.0\nContent-type=text/html\nContent-length=12\n\nHello world!\n\0";

class Job {
public:
  enum Status {
    CREATED = 0,
    EXECUTING,
    ENDED
  };
  Job() : status_(CREATED) {}
  virtual ~Job() {}
  virtual void main() = 0;
protected:
  void start() { status_ = EXECUTING; }
  void end() { status_ = ENDED; }
private:
  Status status_;
};

static void jobExecutorFn(Job* job) {
  job->main();
}

class Connection : public Job {
public:
  Connection(int host_socket, sockaddr_in host_address, int client_socket, sockaddr_in client_address) :
  host_address_(host_address), client_address_(client_address), host_socket_(host_socket), client_socket_(client_socket) {}
  
  virtual void main() { 
    start();
    // A connection is estabilished. Just waits for some data from the
    // other side.
    ssize_t recv_bytes = 0;
    char recv_buff[1024];
    recv_bytes = recv(client_socket_, recv_buff, 1024, 0);
    
    // This code doesn't care about actual incoming message size.
    // It only takes into account the first packet. Obviously a real
    // world scenario would start to decode the incoming data and
    // correlate with a defined protocol.
    // As we said we don't care and proceed to send some data to
    // our peer
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
  sockaddr_in host_address_;
  sockaddr_in client_address_;
  int host_socket_;
  int client_socket_;
  Status status_;
};

class ConnectionQueue {
public:
  void enqueue(Connection conn) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    queue_.push_back(conn);
    queue_wait_.notify_one();
  }

  Connection dequeue() {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    if ( queue_.empty() ) {
      queue_wait_.wait(lock, [this]() { return !queue_.empty(); });
    }
    
    Connection conn = queue_.front();
    queue_.pop_front();
    return conn;  
  }
 
private:
  std::list<Connection> queue_;
  std::mutex queue_mutex_;
  std::condition_variable queue_wait_;
};

static size_t boia = 0;
class Worker : Job {

public:
  ~Worker() {
    end();
  }

  void start(ConnectionQueue* queue) {
    id_ = ++boia;
    start();
    queue_ = queue;
    worker_thread_ = std::thread(jobExecutorFn, (Job*)this);  
    worker_thread_.detach(); 
  }

  virtual void start() {
    Job::start();
    std::cout << "Worker[" << id_ << "] started.\n";
  }

  virtual void end() {
    Job::end();
    std::cout << "Worker[" << id_ << "] stopped.\n";
  }

  virtual void main() {
    while(true) {
      Connection conn = queue_->dequeue();
      std::cout << "Worker[" << id_ << "] resolves connection.\n";
      conn.main();
    }
  }

private:
  ConnectionQueue* queue_;
  std::thread worker_thread_;
  size_t id_;
  
};


class Scheduler {
public:
  Scheduler() {
    for(int widx = 0; widx < workers_count_; ++widx) {
      workers_[widx].start(&queue_);
    }
  } 

  void dispatch(Connection conn) {
     queue_.enqueue(conn);
  }

private:
  ConnectionQueue queue_;
  static const int workers_count_ = 4;
  std::array<Worker,workers_count_> workers_;
};

int main() {
  std::cout << "Hello! Starting server right now.\n";
  Scheduler connections_scheduler;

  int this_socket_fd = socket(PF_INET, SOCK_STREAM, 0);
  int program_state = 0;
  sockaddr_in dst_address;

  // This means: All possible destinations.
  dst_address.sin_family = AF_INET;
  dst_address.sin_port = htons(listen_port);
  dst_address.sin_addr.s_addr = 0;
  
  // Binds socket to all possible destinations
  program_state = bind(this_socket_fd, (sockaddr*)&dst_address, sizeof(dst_address));
  if ( program_state == -1 ) {
     std::cout << "Cannot create server.\n";
     goto shut_down;
  }
  
  // Listens to incoming requests.
  program_state = listen(this_socket_fd, 10);
  if (program_state == -1) {
    std::cout << "Error while initializing incoming connection listening.\n";
    goto shut_down;
  }

  std::cout << "Server begins listening on port " << listen_port << "\n";
  
  // Serverr lifecycle
  while(true) {
    sockaddr_in incoming_addr;
    socklen_t incoming_addr_size = sizeof(incoming_addr);

    // Waits for an incoming connection.
    int incoming_sock_fd = accept(this_socket_fd, (sockaddr*)&incoming_addr, &incoming_addr_size);
    if (incoming_sock_fd < 0) {
      // accept function stops only when an incoming connection is found or
      // if a system signal is raised. Being here means the latter case happened,
      // so it should be wise to stop the server right now.
      goto shut_down;
    }
     
    Connection conn(this_socket_fd, dst_address, incoming_sock_fd, incoming_addr);
    connections_scheduler.dispatch(conn);
  }
  
  std::cout << "Shutting down server. Goodbye!\n";  

shut_down:
  close(this_socket_fd);
  return program_state;
}
