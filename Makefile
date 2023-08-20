
BUILD_TYPE ?= debug
BUILD_TYPE_LOWER ?= $(shell echo $(BUILD_TYPE) | tr A-Z a-z)
THREAD_POOL ?= 30

clean:
	rm -rf cmake-*-*
	rm -rf bin
	rm -rf dist

config:
	cmake -B cmake-$(BUILD_TYPE_LOWER)-build \
		-DTHREAD_POOL:STRING=$(THREAD_POOL) \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -G "Unix Makefiles" .

build:
	$(MAKE) -C cmake-$(BUILD_TYPE_LOWER)-build

package-rocky9:
	docker buildx build \
					-t http-epoll:rpm \
					--output type=local,dest=./dist \
					-f docker/rocky9.Dockerfile .

alpine-build:
	docker buildx build \
					-t http-epoll:alpine \
					-f docker/alpine.Dockerfile .
