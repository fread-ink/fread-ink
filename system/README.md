
# Convert ext2 image to ext4 image

tune2fs -O extents,uninit_bg,dir_index,has_journal freadfs.ext2
mv freadfs.ext2 freadfs.ext4
fsck.ext4 freadfs.ext4
