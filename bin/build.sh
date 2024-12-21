#!/usr/bin/env bash

set -eu

PWD=$(pwd)
TIMESTAMP="${TIMESTAMP:-$(date -u +"%Y%m%d%H%M")}"
COMMIT="${COMMIT:-$(echo xxxxxx)}"

# West Build (left)
west build -s zmk/app -d build -b nice_nano_v2 -- -DZMK_CONFIG="${PWD}/config" -DSHIELD=pillzmod_pro
# Left Kconfig file
grep -vE '(^#|^$)' build/zephyr/.config
# Rename zmk.uf2
cp build/zephyr/zmk.uf2 "./firmware/${TIMESTAMP}-${COMMIT}.uf2"
