.PHONY: build run

build:
	podman build . -t cpp-kms
run: build
	podman run -ti cpp-kms:latest
