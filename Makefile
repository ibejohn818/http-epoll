
BUILD_TYPE ?= debug
BUILD_TYPE_LOWER ?= $(shell echo $(BUILD_TYPE) | tr A-Z a-z)
PLATFORM ?= linux/amd64

# build options
THREAD_POOL ?= 30
USE_MEM_POOL ?= 0

clean:
	rm -rf cmake-*-*
	rm -rf bin
	rm -rf dist

config:
	cmake -B cmake-$(BUILD_TYPE_LOWER)-build \
		-DTHREAD_POOL:STRING=$(THREAD_POOL) \
		-DUSE_MEM_POOL:STRING=$(USE_MEM_POOL) \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -G "Unix Makefiles" .

build:
	$(MAKE) -C cmake-$(BUILD_TYPE_LOWER)-build

package-rocky9:
	docker buildx build \
					--platform $(PLATFORM) \
					-t http-epoll:rpm \
					--output type=local,dest=./dist \
					-f docker/rocky9.Dockerfile .

alpine-build:
	docker buildx build \
					--platform $(PLATFORM) \
					-t http-epoll:alpine \
					-f docker/alpine.Dockerfile .
