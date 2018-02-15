FROM debian:9

RUN apt-get update -y && apt-get upgrade -y
RUN apt-get install -y cmake make g++ xorg-dev libssl-dev libx11-dev libsodium-dev libgl1-mesa-glx libegl1-mesa libcurl3-dev
