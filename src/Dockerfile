FROM ubuntu as build
RUN apt update && apt install -y cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib git python3 build-essential
WORKDIR /
RUN git clone https://github.com/raspberrypi/pico-sdk.git
WORKDIR /pico-sdk
RUN git submodule update --init
WORKDIR /dcc
COPY . .
RUN cp /pico-sdk/external/pico_sdk_import.cmake .
ENV PICO_SDK_PATH=/pico-sdk
RUN cmake .
RUN make

FROM scratch
COPY --from=build /dcc/dcc.uf2 /
