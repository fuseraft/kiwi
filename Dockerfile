FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
    build-essential \
    clang-format \
    nano && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/astral
COPY . /usr/src/astral
RUN make && make install
WORKDIR /home