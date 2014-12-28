
#!/bin/sh
arm-none-eabi-readelf -a $1 | grep "Shared library:"
