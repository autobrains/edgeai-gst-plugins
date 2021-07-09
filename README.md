# GstTIOVX
Repository to host GStreamer plugins for TI's EdgeAI class of devices

## Cross compiling the project

```
# Load the PSDKR path
PSDKR_PATH=<...>

# Create an environment with the:
* Source path
* Build path
* PSDKR path

mkdir build && cd build
source ../crossbuild/environment  $PWD/.. $PWD  $PSDKR_PATH

# Build in cross compilation mode
meson .. --prefix=/usr --cross-file crossbuild/aarch64.ini --cross-file ../crossbuild/crosscompile.txt
ninja -j8
```

## Installing GstTIOVX

```
meson build --prefix=/usr
ninja -C build/ -j8
ninja -C build/ install
```

<div style="color:gray">
    <img src="https://developer.ridgerun.com/wiki/images/2/2c/Underconstruction.png">
    This project is currently under construction.
</div>

