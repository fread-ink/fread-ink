#!/bin/sh

# created by juul

make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-

make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- uImage

make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- targz-pkg

depmod -ae -F System.map -b tar-install -r 2.6.31-rt11-lab126 -n > modules.dep

sed -i 's/^kernel\//\/lib\/modules\/2.6.31-rt11-lab126\/kernel\//g' modules.dep

mkdir -p COPY_TO_KINDLE

cp arch/arm/boot/uImage COPY_TO_KINDLE/
cp linux-2.6.31-rt11-lab126.tar.gz COPY_TO_KINDLE/
cp modules.dep COPY_TO_KINDLE/

echo " "
echo "Build complete. Find the results in the COPY_TO_KINDLE dir."
echo "It should contain one uImage, one tar.gz file and one modules.dep"
