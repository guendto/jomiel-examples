# -*- coding: utf-8 -*-
#
# jomiel-client-demos
#
# Copyright
#  2019-2021 Toni Gündoğdu
#
#
# SPDX-License-Identifier: Apache-2.0
#

# stage: bootstrap-image
#
FROM rust:1.51-slim AS bootstrap-image

WORKDIR /app

COPY rust/Cargo.toml .
COPY rust/Cargo.lock .

COPY rust/src/ ./src/
COPY rust/build.rs .

COPY proto/ ./proto/

RUN set -eux \
    # We want protobuf-compiler 3.12 from buster-backports.
    #
    ; echo 'deb http://deb.debian.org/debian buster-backports main' \
        >> /etc/apt/sources.list \
    && apt-get -qq update \
    && apt-get -t buster-backports -qq --no-install-recommends install \
        protobuf-compiler \
        libzmq3-dev \
        pkgconf \
    && sed -i 's/\.\.\/proto/\.\/proto/' build.rs

# stage: compile-image
#
FROM bootstrap-image AS compile-image

RUN set -eux \
    ; cargo build --release

# stage: runtime-image
#
FROM debian:buster-slim as runtime-image

RUN set -eux \
    ; apt-get -qq update \
    && apt-get -qq --no-install-recommends install \
        libprotobuf-lite17 \
        libprotobuf17 \
        libzmq5 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=compile-image /app/target/release/demo /app/demo

ENTRYPOINT ["/app/demo"]

CMD ["-h"]
