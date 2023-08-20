#include "http-epoll/server.h"
#include <signal.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/epoll.h>


void wait_handler() {}

int main(int argc, char **argv) {

  signal(SIGPIPE, SIG_IGN);

  int listener_fd = create_listener(8080);

  set_nonblocking(listener_fd);

  int epoll_fd = epoll_create1(0);
  if (epoll_fd < 0) {
    fprintf(stderr, "error creating epoll\n");
    exit(EXIT_FAILURE);
  }

  struct epoll_event epoll_evt;
  epoll_evt.events = EPOLLIN | EPOLLET;
  epoll_evt.data.fd = listener_fd;

  if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listener_fd, &epoll_evt) < 0) {
    fprintf(stderr, "error adding listener_fd to epoll list\n");
    exit(EXIT_FAILURE);
  }

  server_ctx_t ctx;
  ctx.listener_fd = listener_fd;
  ctx.epoll_fd = epoll_fd;
  ctx.count = 0;

  // pthread_t pool[THREAD_POOL];
  server_thread_t pool[THREAD_POOL];

  printf("starting thread pool: %d\n", THREAD_POOL);

  // server_loop((void *)&ctx);
  for (uint8_t i = 0; i < THREAD_POOL; i++) {
    server_thread_t thread;
    thread.id = (i + 1);
    thread.ctx = &ctx;
    pool[i] = thread;

    int res = pthread_create(&thread.thread, NULL, server_loop, (void *)&pool[i]);
    if (res < 0) {
      fprintf(stderr, "error starting thread #%d\n", i);
      exit(EXIT_FAILURE);
    }
  }

  sigset_t sigs;
  int sig_ptr;
  int res;
  sigaddset(&sigs, SIGUSR2);
  res = sigwait(&sigs, &sig_ptr);

  printf("Sig Res: %d \n", res);
  printf("Sig Ptr: %d \n", sig_ptr);
  return 0;
}
