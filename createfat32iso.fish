#!/usr/bin/fish
# run with:
# fish --init-command='set fish_trace on' 
# for debugging

rm -f foo.img

# make 64 MiB disk
dd if=/dev/zero of=foo.img bs=512 count=131072
sudo parted -s foo.img mklabel msdos
# sudo parted -s foo.img mkpart primary ext2 2048s 30720s
# or do fat32:
sudo parted -s foo.img mkpart primary fat32 2048s 30720s
sudo parted -s foo.img set 1 boot on

# link using loopback disks
set first $(sudo losetup -f)
sudo losetup $first foo.img
set second $(sudo losetup -f)
sudo losetup $second foo.img -o 1048576

# make filesystem
sudo mkdosfs -F32 -f 2 $second
sudo mkdir /mnt/osfiles
sudo mount $second /mnt/osfiles

# copy seed files from build
sudo cp -r ./build/isofiles/* /mnt/osfiles/

# heres what we have, before grub makes a bunch of files:
echo BEFORE_GRUB-INSTALL
tree /mnt/osfiles/ -L 8

# install grub
sudo grub-install --root-directory=/mnt/osfiles --no-floppy --modules="normal part_msdos ext2 multiboot" --target="x86_64-efi" $first

echo AFTER_GRUB
tree /mnt/osfiles/ -L 8

# teardown loopback devices
sudo losetup -d $first
sudo losetup -d $second

# unmount
sudo umount /mnt/osfiles/
