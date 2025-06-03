BUILDING

Dependency installation (Ubuntu and Debian):

sudo apt update

sudo apt install nasm grub-pc-bin grub-common xorriso mtools
                 gcc-multilib g++-multilib qemu
                 binutils-i686-linux-gnu g++-i686-linux-gnu

                

sudo chmod +x build.sh

./build.sh

Run it with QEMU if you don't want to install it. I'm fairly new to this, so please give feedback!

The precompiled ISO is also in there, if you don't want to compile it yourself.
