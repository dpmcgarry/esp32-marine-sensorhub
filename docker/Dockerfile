FROM ubuntu:latest
RUN apt update && apt upgrade -y && apt install unzip zip curl git wget flex bison gperf python3 python3-pip python3-venv cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0 python3-serial -y && apt-get clean && rm -rf /var/lib/apt/lists/*
RUN mkdir -p ~/esp && cd ~/esp && git clone -b v5.4 --recursive https://github.com/espressif/esp-idf.git
RUN cd ~/esp/esp-idf && ./install.sh esp32c6
RUN cd ~ && curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=/usr/bin sh && arduino-cli config init
COPY ./arduino-cli.yaml ~/.arduino15/arduino-cli.yaml
RUN arduino-cli core update-index && arduino-cli board listall && arduino-cli core install esp32:esp32