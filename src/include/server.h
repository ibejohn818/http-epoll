#ifndef SERVER_H_
#define SERVER_H_

#include <arpa/inet.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>

#define MAX_CONN 1000
#define MAX_EVENTS 1024
#define HEADER_BUF 8192

#ifndef THEAD_POOL
#define THREAD_POOL 10
#endif

typedef enum {
  READ,
  WRITE,
  CLOSE,
} http_state_t;

typedef struct {
  int listener_fd;
  int epoll_fd;
  sig_atomic_t count;
} server_ctx_t;

typedef struct {
  size_t id;
  server_ctx_t *ctx;
  pthread_t thread;
} server_thread_t;

typedef struct {
  http_state_t state;
  int client_socket;
  size_t buffer_pos;
  size_t buffer_size;
  char *buffer;
} http_request_t;

int create_listener(uint16_t port);

void set_nonblocking(int fd);

/**
 * the servers main loop to be executed in a thread pool in main,
 * targs will be a pointer to server_thread_t created in main's stack
 */
void *server_loop(void *targs);

void handler(http_request_t *r, server_ctx_t *ctx);


bool http_body_check(const char *buffer, size_t pos);
#endif
