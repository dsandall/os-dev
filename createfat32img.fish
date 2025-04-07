#!/usr/bin/fish
# run with:
# fish --init-command='set fish_trace on' 
# for debugging

set IMG build/fat32.img
rm -f $IMG
# make 64 MiB disk
dd if=/dev/zero of=$IMG bs=512 count=131072
sudo parted -s $IMG mklabel msdos
# sudo parted -s build/fat32.img mkpart primary ext2 2048s 30720s
# or do fat32:
sudo parted -s $IMG mkpart primary fat32 2048s 30720s
sudo parted -s $IMG set 1 boot on

# link using loopback disks
set first $(sudo losetup -f)
sudo losetup $first $IMG
set second $(sudo losetup -f)
sudo losetup $second $IMG -o 262144

# make filesystem
sudo mkdosfs -F32 -f 2 $second
sudo mkdir /mnt/osfiles
sudo mount $second /mnt/osfiles

# heres what we have, before grub makes a bunch of files:
echo BEFORE_GRUB-INSTALL
#tree /mnt/osfiles/ -L 8

# install grub
# (it is critical to include the i386-pc target info, because if you build on an EFI/x86_64efi machine, it will not work on BIOS/i386)
sudo grub-install --root-directory=/mnt/osfiles --no-floppy --modules="normal part_msdos ext2 multiboot" --target=i386-pc $first
sudo cp -r ./build/isofiles/* /mnt/osfiles/

echo AFTER_GRUB
#tree /mnt/osfiles/ -L 8

# teardown loopback devices
sudo losetup -d $first
sudo losetup -d $second

# unmount
sudo umount /mnt/osfiles/
