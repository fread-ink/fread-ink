
# Convert ext2 image to ext4 image

tune2fs -O extents,uninit_bg,dir_index,has_journal freadfs.ext2
mv freadfs.ext2 freadfs.ext4
fsck.ext4 freadfs.ext4

# Create new ext4 image

Create a sparse file:

```
truncate -s 256M freadfs.ext4
```

Create filesystem:

```
mkfs.ext4 -F freadfs.ext4
```

# inode usage

If df -h shows that there is still space available on the mounted disk image but you get "No space left on device" errors, then you are probably out of inodes. Check with:

```
df -i
```

There is no way to make more inodes. You will have to create a new disk image and copy the contents.

