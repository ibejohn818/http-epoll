#include "http-epoll.h"
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <time.h>
#include <unistd.h>

/*
void graceful_stop(server_thread_t *threads) {
  fprintf(stderr, "do a graceful stop \n");
  for(uint8_t i=0; i < THREAD_POOL; i++) {
    printf("Thread: %d", threads[(size_t)i].id);
  }
}
*/

/**
 * user: server [PORT(default:8080)]
 * Start the epoll server 
 */
int main(int argc, char **argv) {

  uint16_t port = 8080;

  if (argc > 1) {
    port = (uint64_t)atoi(argv[1]);
  }

  // get pid to display
  int pid = getpid();

  printf("Using port: %d on pid: %d\n", port, pid);

  // signal(SIGPIPE, SIG_IGN);
  int listener_fd = create_listener(port);

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

  server_thread_t pool[THREAD_POOL];

  printf("starting thread pool: %d\n", THREAD_POOL);

  for (uint8_t i = 0; i < THREAD_POOL; i++) {
    server_thread_t thread;
    thread.id = (i + 1);
    thread.ctx = &ctx;
    thread.events =  malloc(sizeof(*thread.events) * MAX_EVENTS);
    pool[i] = thread;

    int res =
        pthread_create(&thread.thread, NULL, server_loop, (void *)&pool[i]);
    if (res < 0) {
      fprintf(stderr, "error starting thread #%d\n", i);
      exit(EXIT_FAILURE);
    }
  }

  sigset_t sigs = {};
  int sig_ptr = 0;

  sigaddset(&sigs, SIGUSR2);

  sigwait(&sigs, &sig_ptr);

  // graceful_stop(pool);
  for (uint8_t i = 0; i < THREAD_POOL; i++) {
    server_thread_t st = pool[i];
    pthread_kill(st.thread, SIGTERM);
    sleep(1);
    free(st.events);
  }

  return 0;
}
