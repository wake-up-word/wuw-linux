FROM gcc:10.4

RUN apt-get update -y && \
    apt-get install git cmake -y

RUN echo $(gcc --version)
RUN echo $(cmake --version)

WORKDIR /app
COPY . .

RUN mkdir build && \
    cd build && \
    cmake ../ && \
    cmake --build .
