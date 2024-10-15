FROM ubuntu:18.04

WORKDIR /app
RUN apt-get update
RUN apt-get install cmake -y

RUN apt-get install wget bzip2 unzip g++ m4 libboost-all-dev nasm -y

COPY . .
RUN cd thirdparty/linux && bash boost.get && bash miracl.get && bash ntl.get

RUN cmake .
RUN make -j
# RUN sleep infinity
RUN ./bin/frontend.exe -u