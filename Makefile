
BUILD_TYPE ?= debug
BUILD_TYPE_LOWER ?= $(shell echo $(BUILD_TYPE) | tr A-Z a-z)

clean:
	rm -rf cmake-*-*
	rm -rf bin
	rm -rf dist

config:
	cmake -B cmake-$(BUILD_TYPE_LOWER)-build -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -G "Unix Makefiles" .

build:
	$(MAKE) -C cmake-$(BUILD_TYPE_LOWER)-build

gcc-build-image:
	docker buildx build -f docker/gcc.ubuntu.Dockerfile -t tools/ubuntu:gcc .
