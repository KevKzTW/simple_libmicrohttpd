1)install tool :
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
sudo apt-get update
sudo apt-get install -y cmake git autoconf g++
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

2)install newer libjson-c :
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
git clone https://github.com/json-c/json-c
cd json-c* && git checkout json-c-0.13.1-20180305
mkdir build && cd build && cmake .. && make && make install
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

3)install libcurl and libmicrohttpd
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
sudo apt-get install -y libmicrohttpd-dev libcurl4-gnutls-dev
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~