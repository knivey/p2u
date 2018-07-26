# p2u

Convert JPG/GIF/PNG/PSD/TIFF/etc to ANSI and mIRC art

![screenshot](https://git.trollforge.org/p2u/plain/screenshot.png)

## Installation

```
make
sudo make install
```

## Usage

```
usage: p2a [options] input

-b percent     Adjust brightness levels, default is 100.
-f a|m         Specify output format ANSI or mirc, default is ANSI.
-p m|v         Specify palette to use, mirc or VGA, default is VGA.
-s percent     Adjust saturation levels, default is 100.
-w width       Specify output width, default is the image width.
```

By default the output will be the same width as the width in pixels of the
original image.  Each character represents 1x2 pixel blocks.  Specifiying
width will adjust the height to preserve the aspect ratio.