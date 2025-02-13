FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    clang-format \
    nano \
    libffi-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/kiwi
COPY . /usr/src/kiwi
RUN make && make install

WORKDIR /home
