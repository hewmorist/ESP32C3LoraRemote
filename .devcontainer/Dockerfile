FROM mcr.microsoft.com/devcontainers/base:jammy

RUN apt-get update && apt-get install -y xz-utils \
usbutils \
python3-pip \
python3-setuptools \
python3-venv \
udev \
mc \
&& apt-get clean

# RUN pip3 install platformio

# Setup to get Midnight Commander to exit in current directory
RUN echo "alias mc='. /usr/lib/mc/mc-wrapper.sh'" >> home/vscode/.bashrc

# RUN rm -f /etc/udev/rules.d/99-platformio-udev.rules

# Setup to access to the USB port.
# If the Host is a WINDOWS 11 machine with WSl2 installed, run this in a powershell:
#   usbipd attach --wsl --busid <busid>    

COPY 99-usb-uart.rules /etc/udev/rules.d

RUN rm -f /dev/ttyUSB0 && \
    mknod /dev/ttyUSB0 c 188 0 && \
    chmod 666 /dev/ttyUSB0

 COPY entrypoint.sh /entrypoint.sh

# Assign the RemoteUser to the dialout group to get access to the USB port
 RUN usermod -a -G dialout vscode


ENTRYPOINT ["/entrypoint.sh"]
