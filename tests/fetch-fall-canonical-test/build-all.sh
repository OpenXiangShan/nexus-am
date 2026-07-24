#!/bin/bash

set -euo pipefail

LINUX_GNU_TOOLCHAIN=${LINUX_GNU_TOOLCHAIN:-0}

for mode in 39 39x4 48 48x4 57; do
  make ARCH=riscv64-xs LINUX_GNU_TOOLCHAIN=${LINUX_GNU_TOOLCHAIN} MODE=sv$mode
  make ARCH=riscv64-xs LINUX_GNU_TOOLCHAIN=${LINUX_GNU_TOOLCHAIN} MODE=sv$mode HALF_RVI=1
done
