#!/bin/bash
# filepath: /home/mike/assignments-3-and-later-MikeDanSan/finder-app/manual-linux.sh
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

# Default output directory
DEFAULT_OUTDIR=/tmp/aeld

# Check if an argument is provided, otherwise use the default directory
if [ $# -lt 1 ]; then
    OUTDIR=$DEFAULT_OUTDIR
    echo "Using default directory ${OUTDIR} for output"
else
    OUTDIR=$(realpath $1)
    echo "Using passed directory ${OUTDIR} for output"
fi

# Create the output directory if it doesn't exist
if ! mkdir -p ${OUTDIR}; then
    echo "ERROR: Could not create output directory ${OUTDIR}"
    exit 1
fi

# Ensure required dependencies are installed
echo "Checking and installing required dependencies..."
sudo apt-get update
sudo apt-get install -y \
    build-essential cmake flex bison libssl-dev bc kmod u-boot-tools psmisc \
    cpio tar gzip bzip2 wget git bsdmainutils qemu-system-arm

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    # Clone only if the repository does not exist
    echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
    git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # Kernel build steps

    
    # echo "Cleaning the kernel build tree"
    # make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper

    echo "Configuring the kernel for 'virt' ARM dev board"
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig

    echo "Building the kernel image"
    make -j$(nproc) ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all

    echo "Building kernel modules"
    make -j$(nproc) ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules

    echo "Building devicetree"
    make -j$(nproc) ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs

    echo "Installing kernel modules"
    make -j$(nproc) ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules_install INSTALL_MOD_PATH=${OUTDIR}/rootfs
else
    echo "Kernel image already exists. Skipping kernel build."
fi

echo "Adding the Image in outdir"
if [ ! -e ${OUTDIR}/Image ]; then
    echo "Copying kernel image to ${OUTDIR}"
    cp ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}
else
    echo "Kernel image already exists in ${OUTDIR}. Skipping copy."
fi

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]; then
    echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories
echo "Creating necessary base directories in rootfs"
mkdir -p ${OUTDIR}/rootfs
# Create the directory structure for the root filesystem
# The following directories are created:
cd "$OUTDIR/rootfs"
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin
mkdir -p var/log

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]; then
    git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    echo "Configuring BusyBox"
    make distclean
    make defconfig
else
    cd busybox
fi

# Check if BusyBox is already built
if [ ! -e "${OUTDIR}/busybox/busybox" ] || [ ! -e "${OUTDIR}/rootfs/bin/busybox" ]; then
    echo "Building and installing BusyBox"
    make -j$(nproc) ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
    make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install
else
    echo "BusyBox is already built. Skipping this step."
fi

# TODO: Add library dependencies to rootfs
echo "Adding library dependencies to rootfs"

# Identify the sysroot of the cross-compiler
echo "Copying necessary shared libraries and program interpreter"
SYSROOT=$(${CROSS_COMPILE}gcc -print-sysroot)

# Copy the program interpreter to /lib
cp -a ${SYSROOT}/lib/ld-linux-aarch64.so.1 ${OUTDIR}/rootfs/lib/

# Copy shared libraries to /lib64
cp -a ${SYSROOT}/lib64/libm.so.6 ${OUTDIR}/rootfs/lib64/
cp -a ${SYSROOT}/lib64/libresolv.so.2 ${OUTDIR}/rootfs/lib64/
cp -a ${SYSROOT}/lib64/libc.so.6 ${OUTDIR}/rootfs/lib64/

echo "Library dependencies added to rootfs"

# TODO: Make device nodes
echo "Creating device nodes in rootfs"

# Create /dev/null
sudo mknod -m 666 ${OUTDIR}/rootfs/dev/null c 1 3

# Create /dev/console
sudo mknod -m 600 ${OUTDIR}/rootfs/dev/console c 5 1

echo "Device nodes created in rootfs"

# TODO: Clean and build the writer utility
echo "Cleaning and building the writer utility"
cd ${FINDER_APP_DIR}
make clean
make CROSS_COMPILE=${CROSS_COMPILE}

# Copy the writer utility to the root filesystem
cp writer ${OUTDIR}/rootfs/home/

echo "Writer utility built and copied to rootfs"

# TODO: Copy the finder related scripts and executables to the /home directory on the target rootfs
echo "Copying finder related scripts and executables to /home directory on rootfs"

# Copy finder scripts and executables
cp ${FINDER_APP_DIR}/autorun-qemu.sh ${OUTDIR}/rootfs/home/
cp ${FINDER_APP_DIR}/finder.sh ${OUTDIR}/rootfs/home/
cp ${FINDER_APP_DIR}/finder-test.sh ${OUTDIR}/rootfs/home/
cp -rL ${FINDER_APP_DIR}/conf ${OUTDIR}/rootfs/home/

echo "Finder related scripts and executables copied to rootfs"

# TODO: Chown the root directory
echo "Changing ownership of the root directory to root:root"
sudo chown -R root:root ${OUTDIR}/rootfs
sudo chmod -R u+rwX ${OUTDIR}/rootfs

echo "Ownership of root directory changed to root:root"

# TODO: Create initramfs.cpio.gz
cd "$OUTDIR/rootfs"

find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio

gzip -f "$OUTDIR/initramfs.cpio"
echo "Created initramfs.cpio.gz in ${OUTDIR}"