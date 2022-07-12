# The Abstract Machine (AM)

## How to build benchmark flash image
- clone `nexus-am` from github:
``` shell
git clone git@github.com:OpenXiangShan/nexus-am.git
```
- enter benchmark directory (using `apps/coremark` as an example):
```
cd /apps/coremark
make ARCH=riscv64-xs-flash
cd ./build
ls
```
- you will find a ".bin" file, this is a benchmark image in flash

## How to use the prepared flash image to do simulation
- assuming you have a `XiangShan` repo, the commit ID should be newer than 188f739de96af363761c0f2b80b95b70ad01e0fc
- make `emu` build
- use `-F` to load the image in flash:
```
./emu -F $AM_HOME//apps/coremark/build/coremark-riscv64-xs-flash.bin  -i ../ready-to-run/coremark-2-iteration.bin 2>debug.log
```
> NOTE: use `-i` to specify the initial ram image, or a ramdom file if you do not care.