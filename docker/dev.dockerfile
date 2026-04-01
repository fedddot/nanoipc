FROM alpine:latest AS base_image

RUN apk update
RUN apk add git make gcc g++ gdb cmake clang-extra-tools
RUN apk add python3 python3-dev py3-pip
RUN pip install --upgrade --break-system-packages protobuf grpcio-tools

ENV HOME=/root
COPY docker/shell_additions ${HOME}
RUN echo "source ${HOME}/shell_additions" >> ${HOME}/.bashrc

WORKDIR /usr/app/src

ENTRYPOINT ["sh"]