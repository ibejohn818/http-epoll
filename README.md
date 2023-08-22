## epoll-http

An example of an http server using `epoll` and thread pools written in C.    

The server will read up to 8k of header data into a hash table attached to each request.   

In addition there's an automic request counter attached the the `server_ctx_t` that counts the number of requests and sends them back in the response headers.


## Configure / Build

The project requires `cmake` and can be built using `make`   
The following make parameters can be passed into configure the demo.   

```shell
# number of thread workers to pre allocate
THREAD_POOL={NUMBER OF THREADS}

# use a memory pool for the http read buffers
# they are off by default
USE_MEM_POOL=1 

# example of release build w/threadpool of 16 with http buffer memory pool

make config build BUILD_TYPE=Release THREAD_POOL=16 USE_MEM_POOL=1

# binary will be in the repos bin dir

./bin/server 8080
```
