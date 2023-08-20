FROM alpine:3.17

RUN apk --update --no-cache add \
 build-base clang15 make cmake valgrind gdb lldb


WORKDIR /app

COPY . .

RUN make config build BUILD_TYPE=Release
