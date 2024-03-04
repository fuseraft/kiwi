FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
    build-essential \
    clang-format \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/kiwi
COPY . /usr/src/kiwi
RUN make && make install
WORKDIR /home

CMD ["bash"]