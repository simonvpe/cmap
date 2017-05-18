FROM gcc-build
RUN apt-get update -yqq && apt-get install -yqq libboost-dev libboost-system1.62-dev


