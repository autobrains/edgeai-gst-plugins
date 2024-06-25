#!/bin/bash

export SOC=am62a
rm -rf build
meson build --prefix=/usr -Dpkg_config_path=pkgconfig
ninja -C build install
ldconfig
