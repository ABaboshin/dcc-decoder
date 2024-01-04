FROM ubuntu as build
RUN apt update && apt install -y cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib git python3 build-essential wget ffmpeg
WORKDIR /
RUN git clone https://github.com/raspberrypi/pico-sdk.git
WORKDIR /pico-sdk
RUN git submodule update --init
# WORKDIR /
# RUN git clone https://github.com/ikjordan/picomp3lib.git
WORKDIR /dcc
COPY . .
RUN cp /pico-sdk/external/pico_sdk_import.cmake .
ENV PICO_SDK_PATH=/pico-sdk
RUN ffmpeg -i happy.mp3 -codec:a libmp3lame -b:a 40000 -s 44.1 output.mp3
# RUN ffmpeg -i happy.mp3 -acodec pcm_u8 -ar 44100 output.wav
RUN cmake .
RUN make

FROM scratch
COPY --from=build /dcc/dcc.uf2 /
COPY --from=build /dcc/output.mp3 /
# COPY --from=build /dcc/output.wav /
