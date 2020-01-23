FROM ubuntu:latest

RUN apt-get update
RUN apt-get install -y \
    make gcc-arm-none-eabi binutils-arm-none-eabi libnewlib-arm-none-eabi
#gdb-multiarch openocd git build-essential sed

COPY . /repo
WORKDIR /repo

RUN make clean

# -j to run in parallel
RUN make build -j
