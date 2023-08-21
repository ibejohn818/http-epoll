#include "http-epoll/http.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void *reallocate(void *pointer, size_t old_size, size_t new_size) {

  if (new_size == 0) {
    free(pointer);
    return NULL;
  }

  void *result = realloc(pointer, new_size);
  if (result == NULL) {
    fprintf(stderr, "realloc error: %lu\n", (unsigned long)new_size);
    exit(EXIT_FAILURE);
  }

  return result;
}

#define HTTP_GROW_ARRAY(type, pointer, new_size, old_size)                     \
  (type *)reallocate(pointer, new_size, old_size)


http_scanner_t *http_scanner_init(const char *input) {
  http_scanner_t *s;
  s = (http_scanner_t *)malloc(sizeof(*s));
  s->start = input;
  s->current = input;
  return s;
}

static bool http_scanner_is_eof(http_scanner_t *s) {
  bool eof = *s->current == '\0'; 
  if (eof) {
    puts("EOF Found!");
  }
  return eof;
}

char advance(http_scanner_t *s) {
  s->current++;
  return s->current[-1];
}


void http_scanner_free(http_scanner_t *s) {
  free((void *)s->current);
  free(s); 
}

http_method_t http_msg_char_to_method(const char *m) {
  char method[16] = {0};
  uint8_t i = 0;
  for(; *m; m++) {
      method[i] = tolower(*m);
      i++;
  }

  if (strcmp("get", method) == 0) {
    return HTTP_METHOD_GET;
  }
  else if (strcmp("post", method) == 0) {
    return HTTP_METHOD_POST;
  }
  else if (strcmp("put", method) == 0) {
    return HTTP_METHOD_PUT;
  } 
  else if (strcmp("delete", method) == 0) {
    return HTTP_METHOD_DELETE;
  }

  return HTTP_METHOD_HEAD;
}

void http_scanner_scan_request_proto(http_scanner_t *s, http_msg_t *m) {

  uint8_t found = 0;
  for(;;) {
    char buff[2048] = {0};
    char c = advance(s);

    if(http_scanner_is_eof(s)) {
      break;
    }

    if(c == ' ') {
      int len = s->current - s->start;
      memcpy(&buff, s->start, (len-1));
      printf("B: %s\n", buff);
      s->start = s->current;

      if (found == 0) {
          // first block is verb
          m->method = http_msg_char_to_method(buff);
          found++;
      } else if (found == 1) {
          // second block is URI
        strcpy(m->uri, buff); 
        found++;
      }
    }

    if (c == '\n') {
      // get protocol version
      int len = s->current - s->start;
      memcpy(&buff, s->start, (len-1));
      strcpy(m->proto, buff);
      break;
    }
  }

}
http_msg_t *http_msg_create(http_msg_type_t type) {
  http_msg_t *m;
  m = (http_msg_t *)calloc(1, sizeof(*m));
  m->type = type;
  return m;
}

char http_scanner_peek(http_scanner_t *s) {
    if (http_scanner_is_eof(s)) {
        return '\0';
    }

    char c = *s->current;
    return c;
}

static void http_scanner_eat_whitespace(http_scanner_t *s) {
    for(;;) {
        char c = *s->current;
        switch (c) {
            case ' ':
                advance(s);
            default:
                return;
        }
    }

}

http_header_t *http_scanner_scan_header_line(http_scanner_t *s) {

    // peek at begining to see if we have double \n
    if (http_scanner_peek(s) == '\n') {
        puts("start of body found");
        return NULL;
    }

    char key[128] = {0};
    char val[2048] = {0};
    bool key_found = false;
    s->start = s->current;

    for(;;) {

        char c = advance(s);
        if (http_scanner_is_eof(s)) {
            return NULL;
        }

        if (!key_found && http_scanner_peek(s) == ':') {

            uint32_t l = s->current - s->start;
            // copy to key buff
            memcpy(key, s->start, l);
            key_found = true;
            // move past colon
            advance(s);
            // reset start
            s->start = s->current;
            continue;
        } else if(key_found && http_scanner_peek(s) == '\n') {

//            http_scanner_eat_whitespace(s);
//            s->start = s->current;

            uint32_t l = s->current - s->start;
            // copy to key buff
            memcpy(val, s->start, l);
            key_found = true;
            // move past newline
            advance(s);
            break;
        }
    }

    if (!key_found) {
        return NULL;
    }

    http_header_t *h;
    h = (http_header_t *)calloc(1, sizeof(*h));
    strcpy(h->key, key);
    strcpy(h->value, val);


    return h;
}

static void http_scanner_scan_to_eof(http_scanner_t *s) {
  uint64_t c = 0;
  for(;;) {
    if (http_scanner_is_eof(s)) {
      printf("Count: %lu \n", c);
      break;
    }
    advance(s);
    c++;
  }
}

http_msg_t *http_msg_scan_request(const char *input) {
    // http_scanner_t *s = http_scanner_init(input);
    http_scanner_t s = {.start = input, .current = input};
    http_msg_t *m = http_msg_create(HTTP_MSG_REQUEST);
    m->headers = http_hash_map_init(HTTP_HASH_MAP_DEFAULT_BUCKETS);

    // scan protocol line
    http_scanner_scan_request_proto(&s, m);
    // scan headers

    for(;;) {
        http_header_t *header = http_scanner_scan_header_line(&s);
        if (header == NULL) {
            break;
        } else {
            http_hash_map_insert(m->headers, HTTP_NODE_HEADER, header->key, (void *)header);
        }
    }

  // http_scanner_scan_to_eof(&s);

  // http_scanner_free(s);

    return m;
}

void scanner_read_until(http_scanner_t *s, const char mark) {

  bool loop = true;
  uint32_t i = 0;
  char key[1024] = {0};
  char val[2048] = {0};

  while(loop) {

    char c = advance(s);
    if (c == mark) {
      puts("Mark found");
      loop = false;
    }

    printf("C: %c\n", c);

    i++;

    if(i > 90) {
      loop=false;
    }

  }

}

static uint32_t jenkins_one_at_a_time_hash(const char *key) {
  size_t length = strlen(key);
  printf("len: %lu \n", length);
  // size_t length = 16;
  size_t i = 0;
  uint32_t hash = 0;
  while (i != length) {
    hash += key[i++];
    hash += hash << 10;
    hash ^= hash >> 6;
  }
  hash += hash << 3;
  hash ^= hash >> 11;
  hash += hash << 15;
  return hash;
}

static uint64_t hash_key_bjb2(const char *str) {

  uint64_t hash = 5381;
  uint64_t c;

  while ((c = *str++)) {
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  }

  return hash;
}

static uint64_t hash_map_key(const char *str) {
  return jenkins_one_at_a_time_hash(str);
  // return hash_key_bjb2(str);
}


http_hash_map_t *http_hash_map_init(size_t size) {
  http_hash_map_t *m;
  m = (http_hash_map_t *)malloc(sizeof(*m));
  m->nodes = (http_hash_node_t **)calloc(size, sizeof(http_hash_node_t *));
  m->size = size;
  m->buckets_count = 0;
  m->items_count = 0;
  return m;
}

static http_hash_node_t *http_kv_node_create(http_node_type_t type,
                                             const char *key, void *value) {
  http_hash_node_t *n;
  n = (http_hash_node_t *)calloc(1, sizeof(*n));
  n->type = type;
  n->key = strdup(key);
  n->value = value;
  return n;
}

void http_hash_map_insert(http_hash_map_t *m, http_node_type_t type,
                          const char *key, void *value) {

  uint64_t hash_key = (hash_map_key(key) % m->size);

  http_hash_node_t *bucket;

  if (m->nodes[hash_key] == NULL) {

    bucket = http_kv_node_create(type, key, value);
    m->nodes[hash_key] = bucket;

    m->buckets_count++;
    m->items_count++;

    return;

  } else {

    bucket = m->nodes[hash_key];
  }

  do {
    // check for existing
    if (strcmp(key, bucket->key) == 0) {
      // overwrite existing value
      // free current item
      // FIXME: free value based on previous node type
      free(bucket->value);
      // add incoming value
      bucket->value = value;
      // update type
      bucket->type = type;
      return;
    }

    if (bucket->next != NULL) {
      bucket = bucket->next;
    }

  } while (bucket->next != NULL);

  // if we reached this point, bucket is
  // the last item in the buckets linked list.
  // Add item to next

  http_hash_node_t *n = http_kv_node_create(type, key, value);
  bucket->next = n;
  m->items_count++;
}

http_hash_node_t *http_hash_map_get(http_hash_map_t *m, const char *key) {

  uint64_t hash_key = hash_map_key(key) % m->size;

  if (m->nodes[hash_key] != NULL) {
    http_hash_node_t *n = m->nodes[hash_key];

    while (n != NULL) {
      if (strcmp(key, n->key) == 0) {
        return n;
      }

      n = n->next;
    }
  }

  return NULL;
}

static void http_hash_map_free_node_value(http_node_type_t type,
                                          void *value) {
  switch (type) {

  case HTTP_NODE_HEADER: {
    http_header_t *th = (http_header_t *)value;
    free(th);
    break;
  }
  case HTTP_NODE_COOKIE: {
  }
  case HTTP_NODE_QUERY: {
  }
  case HTTP_NODE_POST: {
  }
  }
}

static void http_kv_map_free_node(http_hash_node_t *n) {

  http_hash_node_t *tmp;

  while (n != NULL) {
    tmp = n->next;

    // free node value based on its type
    http_hash_map_free_node_value(n->type, n->value);
    // free key
    free((void *)n->key);
    // free node
    free(n);

    n = tmp;
  }
}

void http_hash_map_free(http_hash_map_t *m) {

  for (uint64_t i = 0; i < m->size; i++) {
    if (m->nodes[i] != NULL) {
      http_kv_map_free_node(m->nodes[i]);
    }
  }

  free(m->nodes);
  free(m);
}

http_header_t *http_header_create(const char *key, const char *value) {
  http_header_t *h;
  h = (http_header_t *)calloc(1, sizeof(*h));
  strcpy(h->key, key);
  strcpy(h->value, value);
  return h;
}

void http_hash_map_for_each(http_hash_map_t *t, void (*handler)(http_hash_node_t *i)) {

    for (uint64_t i = 0; i < t->size; i++) {
        // skip if no bucket
        if (t->nodes[i] == NULL) {
            continue;
        }

        // get bucket
        http_hash_node_t *b = t->nodes[i];

        // walk the list and execute the callback handler
        while (b != NULL) {
            handler(b);
            b = b->next;
        }
    }
}
