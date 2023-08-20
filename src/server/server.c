#include "server.h"
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

int create_listener(uint16_t port) {
  struct sockaddr_in server_addr;

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);

  // TODO: addr make configurable
  server_addr.sin_addr.s_addr = INADDR_ANY;

  int listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (listenfd < 0) {
    fprintf(stderr, "error creating listener socket\n");
    return -1;
  }

  int reuse =
      setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

  if (reuse < 0) {
    fprintf(stderr, "error setting reuse addr on listener socket\n");
    return -1;
  }

  if (bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) !=
      0) {
    fprintf(stderr, "Error while binding listenfd to address\n");
    return -1;
  }
  if (listen(listenfd, MAX_CONN) < 0) {
    fprintf(stderr, "Error while listening on listenfd\n");
    return -1;
  }

  return listenfd;
}

void set_nonblocking(int fd) {

  int old_option = fcntl(fd, F_GETFL);
  int new_option = old_option | O_NONBLOCK;

  fcntl(fd, F_SETFL, new_option);
}

bool http_body_check(const char *buffer, size_t pos) {
  const char nl = '\n';
  const char cr = '\r';

  if (buffer[pos - 1] == nl && buffer[pos - 2] == nl) {
    return true;
  }

  if (buffer[pos - 1] == cr && buffer[pos - 2] == nl && buffer[pos - 3] == cr &&
      buffer[pos - 4] == nl) {
    return true;
  }

  if (buffer[pos - 1] == nl && buffer[pos - 2] == cr && buffer[pos - 3] == nl &&
      buffer[pos - 4] == cr) {
    return true;
  }

  return false;
}

void handler(http_request_t *r, server_ctx_t *ctx) {

  if (r->state == READ) {

    // read the incoming request stream
    // NOTE: here we'll track the read state in http_request_t->buffer_pos
    //      in-case it takes multiple reads to read the entire buffer
    bool end = false;
    while (1) {
      ssize_t res = read(r->client_socket, r->buffer + r->buffer_pos, 1);
      if (r->buffer_pos >= HEADER_BUF) {
        printf("buffer over max: \n");
        r->state = CLOSE;
        break;
      }
      if (res < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
          break;
        } else {
          perror("handler read");
          break;
        }
      } else if (res == 0) {
        // EOF? should we move to the next state?
        break;
      } else {
        r->buffer_pos++;
        // check for body markers
        if (r->buffer_pos >= 4 && http_body_check(r->buffer, r->buffer_pos)) {
          end = true;
        }
        break;
      }
    }

    if (!end) {
      fprintf(stderr, "read !end \n");
      return;
    }

    printf("Read Buffer: %s\n", r->buffer);
    r->state = WRITE;
  }
  if (r->state == WRITE) {
    // write data back to the client
    // NOTE: here we'll track writes back to the client in-case it takes
    //      multiple writes, this example "should" only take a single write
    ctx->count += 1;
    char msg[128] = {0};
    sprintf(msg, "HTTP/1.1 200 OK\n"
                 "request-count: %d\n"
                 "context-length: 3\n\n"
                 ":-D", ctx->count);
    size_t msg_len = strlen(msg);

    int res = write(r->client_socket, msg, msg_len);
    printf("write res: %d | count %d\n", res, ctx->count);

    if (res < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // read end normally
        return;
      } else {
        fprintf(stderr, "error writing to client socket\n");
        r->state = CLOSE;
        return;
      }
    } else if (res == 0) {
      r->state = CLOSE;
    } else {
      r->state = CLOSE;
    }
  }
}

void *server_loop(void *targs) {
  // server_ctx_t *ctx = (server_ctx_t *) targs;
  server_thread_t *thread = (server_thread_t *)targs;
  server_ctx_t *ctx = thread->ctx;

  printf("starting thread #%lu \n", thread->id);

  struct epoll_event *events = malloc(sizeof(*events) * MAX_EVENTS);

  if (events == NULL) {
    fprintf(stderr, "unable to allocated epoll_events\n");
    exit(EXIT_FAILURE);
  }

  int epoll_fd = ctx->epoll_fd;
  int listener_fd = ctx->listener_fd;

  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);

  int num_fds;
  for (;;) {

    num_fds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

    if (num_fds < 0) {
      fprintf(stderr, "epoll_wait returned an error\n");
      continue;
    }

    for (uint32_t i = 0; i < num_fds; i++) {

      if (events[i].data.fd == listener_fd) {
        // polled socket is the root listener, accept connection
        struct epoll_event evt;

        for (;;) {

          int client_socket = accept(
              listener_fd, (struct sockaddr *)&client_addr, &client_addr_len);
          if (client_socket < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
              break;
            } else {
              fprintf(stderr, "error accepting client socket\n");
              break;
            }
          }

          set_nonblocking(client_socket);

          evt.events = EPOLLET | EPOLLIN | EPOLLONESHOT;

          // TODO/NOTE: this would be a good place to implement a memory 
          //            of requests and buffers  
          http_request_t *request = malloc(sizeof(*request));
          request->buffer = malloc(sizeof(char) * HEADER_BUF);
          if (request == NULL || request->buffer == NULL) {
            fprintf(stderr, "unable to allocate http_reqeust_t \n");
            exit(EXIT_FAILURE);
          }

          request->buffer_size = HEADER_BUF;
          request->buffer_pos = 0;
          request->client_socket = client_socket;
          request->state = READ;
          evt.data.ptr = request;

          if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &evt) < 0) {
            fprintf(stderr, "unable to add client_socket to epoll list\n");
            continue;
          }
        }
      } else {

        struct epoll_event ee;

        http_request_t *request = (http_request_t *)events[i].data.ptr;

        handler(request, ctx);

        if (request->state == READ) {
          ee.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
          ee.data.ptr = request;
          if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, request->client_socket, &ee) <
              0) {
            fprintf(stderr, "epoll mod error in request read state \n");
            continue;
          }
        } else if (request->state == WRITE) {
          ee.events = EPOLLOUT | EPOLLET | EPOLLONESHOT;
          ee.data.ptr = request;
          if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, request->client_socket, &ee) <
              0) {
            fprintf(stderr, "epoll mod error in request write state \n");
            continue;
          }
        } else if (request->state == CLOSE) {
          close(request->client_socket);
          free(request->buffer);
          free(request);
        }
      }
    }
  }
}
