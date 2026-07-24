#!/bin/bash

set -euo pipefail

LINUX_GNU_TOOLCHAIN=${LINUX_GNU_TOOLCHAIN:-0}

for mode in 39 39x4 48 48x4 57; do
  for jump in 0 1 2 3; do
    make ARCH=riscv64-xs LINUX_GNU_TOOLCHAIN=${LINUX_GNU_TOOLCHAIN} MODE=sv$mode JUMP=$jump HALF_RVI=0
  done
  make ARCH=riscv64-xs LINUX_GNU_TOOLCHAIN=${LINUX_GNU_TOOLCHAIN} MODE=sv$mode JUMP=0 HALF_RVI=1
done
