
epdcc or "EPDC Controller" is a utility for sending various ioctls to the Freescale i.mx series electronic paper display controller (epdc) used in various e-book readers.

# Usage

```
./epdcc [on|off|update]

  Operations:
     on or off: Enable or disable epdc auto-update.
        update: Update entire display once.
  Options:
     -d device: Device file to open (default: /dev/fb0)
            -m: Monochrome update (for update operation only)
```

# Disclaimer

This piece of software is entirely unofficial and was neither created by nor endorsed by Freescale (excepting the mxcfb.h file which includes its own comments with explanations).

# License and copyright for everything other than mxcfb.h

This code was originally based on [Marek Gibek's framebuffer-utils](https://github.com/marek-g/kobo-kernel-2.6.35.3-marek/tree/linux/!Marek/framebuffer-utils)

* Copyright 2015 Marc Juul
* Copyright 2013 Marek Gibek

[MIT](https://opensource.org/licenses/MIT) license