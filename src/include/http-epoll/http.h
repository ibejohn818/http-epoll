
#ifndef HTTP_H_
#define HTTP_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define HTTP_HASH_MAP_DEFAULT_BUCKETS 64

/**
 * Http hash table data type
 */
typedef enum {
    HTTP_NODE_HEADER,
    HTTP_NODE_COOKIE,
    HTTP_NODE_QUERY,
    HTTP_NODE_POST,
} http_node_type_t;

typedef enum {
  HTTP_METHOD_GET,
  HTTP_METHOD_PUT,
  HTTP_METHOD_POST,
  HTTP_METHOD_DELETE,
  HTTP_METHOD_HEAD,
  HTTP_METHOD_CONNECT,
} http_method_t;

/**
 * Hash table node acts as a linked list to handle collisions
 */
typedef struct http_hash_node_t {
    http_node_type_t type;
    const char *key;
    void *value;
    struct http_hash_node_t *next;
} http_hash_node_t;

/**
 * Hash table kv store for http request data
 */
typedef struct {
    // index's allocated
    size_t size;
    // number of buckets (keys)
    size_t buckets_count;
    // number of items
    size_t items_count;
    // array of nodes
    http_hash_node_t **nodes;
} http_hash_map_t;

typedef enum {
  HEADER_TYPE_CHAR,
  HEADER_TYPE_NUMBER,
} HeaderType;

typedef enum {
  HTTP_MSG_REQUEST,
  HTTP_MSG_RESPONSE,
} http_msg_type_t;

typedef struct {

} HttpToken;

typedef struct {
  char key[128];
  char value[4096];
} http_header_t;

typedef struct {
    uint32_t size;
    uint32_t count;
    http_header_t **headers;
} HttpHeaders;

typedef struct {
  const char *current;
  const char *start;
} http_scanner_t;

typedef struct {
  http_msg_type_t type;
  http_method_t method;
  char uri[2048];
  char proto[16];

  http_hash_map_t *headers;
  
} http_msg_t;

// #############
//  HTTP request scanner
// #############

/**
 * create a header pointer
 */
http_header_t *http_header_create(const char *key, const char *value);

void scanner_read_until(http_scanner_t *s, const char mark);

void http_scanner_free(http_scanner_t *s);

http_msg_t *http_msg_create(http_msg_type_t type);

http_msg_t *http_msg_scan_request(const char *input);

void http_scanner_scan_request_proto(http_scanner_t *s, http_msg_t *m);

http_msg_t *http_msg_scan(const char *input);

http_header_t *http_scanner_scan_header_line(http_scanner_t *s);

//////////////

http_scanner_t *http_scanner_init(const char *input);


// #############
//  Hash table
// #############



/**
 * initialize http hash table
 */
http_hash_map_t *http_hash_map_init(size_t size);
/**
 * insert node into hash table
 */
void http_hash_map_insert(http_hash_map_t *m,
                          http_node_type_t type,
                          const char *key, void *value);
/**
 * Get node from hash table by key
 */
http_hash_node_t *http_hash_map_get(http_hash_map_t *m, const char *key);

/**
 * return a http_header_t* from a http_msg_t* by key
 */
http_header_t *http_msg_header_get(http_msg_t *m, const char *key);

void http_hash_map_for_each(http_hash_map_t *t, void (*handler)(http_hash_node_t *i));

/**
 * free http kv map
 */
void http_hash_map_free(http_hash_map_t *m);

// ############
//  HTTP request
// ############
typedef struct {

} HttpRequest;

#endif
