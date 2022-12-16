FROM ubuntu:jammy

WORKDIR /

RUN apt update -y

RUN apt install python3 python-is-python3 build-essential git cmake libasound2-dev mesa-common-dev libx11-dev libxrandr-dev libxi-dev xorg-dev libgl1-mesa-dev libglu1-mesa-dev -y

RUN git clone https://github.com/emscripten-core/emsdk.git 

RUN ./emsdk/emsdk install latest
RUN ./emsdk/emsdk activate latest --permanent

WORKDIR /app

CMD [ "bash", "docker-web.sh" ]
