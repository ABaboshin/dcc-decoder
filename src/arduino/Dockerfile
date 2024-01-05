FROM ubuntu as build
RUN apt update && apt install -y curl python3
RUN curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh

RUN arduino-cli core update-index --additional-urls https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
RUN arduino-cli core install rp2040:rp2040 --additional-urls https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
WORKDIR /sketch
COPY . .
WORKDIR /
RUN arduino-cli compile --fqbn rp2040:rp2040:rpipico --output-dir /build sketch
FROM scratch
COPY --from=build /build/*.uf2 /
