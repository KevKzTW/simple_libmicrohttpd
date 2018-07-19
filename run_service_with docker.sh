#!/bin/bash

docker build -t "httpd/test" .

docker network create --subnet=172.18.0.0/24 internal

docker run -t -i --rm --network internal --ip 172.18.0.2 httpd/test my_httpd
