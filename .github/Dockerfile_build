FROM ubuntu:latest

RUN apt-get update
RUN apt-get install -y gcc-arm-none-eabi binutils-arm-none-eabi libnewlib-arm-none-eabi make

COPY . /app

WORKDIR /app

# '-j' is to run in parallel
RUN make build -j
