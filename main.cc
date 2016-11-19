#include "server/looper.h"

static short listen_port = 4040;

int main() {
  wi::server::looper srv_looper;
  srv_looper.start(listen_port);
  return 0;
}

