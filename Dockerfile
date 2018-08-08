FROM busybox:glibc

RUN mkdir /usr/lib
COPY build/lib* /usr/lib/
COPY build/my_httpd /bin
RUN ln -s /usr/lib/libmicrohttpd.so.10.32.0 /usr/lib/libmicrohttpd.so.10



