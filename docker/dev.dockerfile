FROM alpine:latest AS base_image

RUN apk update
RUN apk add git make gcc g++ gdb cmake clang-extra-tools valgrind
RUN apk add python3 python3-dev py3-pip
RUN pip install --upgrade --break-system-packages protobuf grpcio-tools
RUN apk add bash openssh

ARG UID=1000
ARG GID=1000

RUN addgroup -g ${GID} developer
RUN adduser -D -u ${UID} -G developer -s /bin/bash developer

ENV SHELL=bash

ENV HOME=/home/developer
COPY --chown=${UID}:${GID} docker/shell_additions ${HOME}
RUN echo "source ${HOME}/shell_additions" >> ${HOME}/.bashrc

USER developer

WORKDIR /usr/app/src


ENTRYPOINT ["$SHELL"]