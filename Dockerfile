FROM amd64/ubuntu:16.04

RUN apt-get update

RUN apt-get install -y cmake git autoconf g++
RUN cd /root && git clone https://github.com/json-c/json-c
RUN cd /root/json-c* && git checkout json-c-0.13.1-20180305 && mkdir build && cd build && cmake .. && make && make install
RUN rm -r /root/json-c*

RUN apt-get install -y libmicrohttpd-dev
COPY build/my_httpd /bin
