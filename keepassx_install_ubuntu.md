Installing KeePassX 2 Beta2 on Ubuntu 14.04 (LTS)
=================================================

These instructions are for compiling KeePassX 2 on Ubuntu 14.04 (LTS) from the source code. This is tested for KeePassX 2 Beta 2. Linux binaries are not available by default yet. Hopefully, Debian / Arch / Ubuntu will include deb files in subsequent releases.

1. Update repositories and Install Git. If you have Git installed already, skip this

    sudo apt-get update && sudo apt-get install git-core

2. Install software required to compile C++ code

    sudo apt-get -y install build-essential cmake 

3. Install essential QT libraries. KeePassX GUI is based on QT framework. (libqt5x11extras5-dev is optional but nice to have)

    sudo apt-get -y install qtbase5-dev libqt4-dev libqt5x11extras5-dev qttools5-dev qttools5-dev-tools

4. Install other required libraries for cryptography and compression. (libxtst-dev is optional; Required for testing)

    sudo apt-get -y install zlib1g-dev libgcrypt20-dev libxtst-dev

5. Clone the KeePassX GitHub repository to your local machine

    git clone https://github.com/keepassx/keepassx.git

6. Create a build directory and build the software from within there. The install step should be done as root.

    mkdir keepassx/build
    pushd keepassx/build
    cmake -DCMAKE_VERBOSE_MAKEFILE=ON -DCMAKE_INSTALL_PREFIX=/usr/local ..
    make
    sudo make install

7. KeePassX binary should have been installed at /usr/local/bin/keepassx. You can create a symbolic link to it at /usr/bin. Just make sure that it doesn't conflict with any previous installations of KeePassX by naming the symbolic link differently. You can also modify the CMAKE_INSTALL_PREFIX parameter to /usr if you want the binary to directly be in /usr/bin

8. Check the linked libraries for the binary (Optional)

    vagrant@vagrant-ubuntu-trusty-64:/usr/local/bin$ ldd keepassx 
            linux-vdso.so.1 =>  (0x00007fffb6144000)
            libQt5Core.so.5 => /usr/lib/x86_64-linux-gnu/libQt5Core.so.5 (0x00007fa14b933000)
            libQt5Widgets.so.5 => /usr/lib/x86_64-linux-gnu/libQt5Widgets.so.5 (0x00007fa14b10a000)
            libgcrypt.so.20 => /lib/x86_64-linux-gnu/libgcrypt.so.20 (0x00007fa14ae2a000)
            libz.so.1 => /lib/x86_64-linux-gnu/libz.so.1 (0x00007fa14ac11000)
            libQt5Gui.so.5 => /usr/lib/x86_64-linux-gnu/libQt5Gui.so.5 (0x00007fa14a5c4000)
            libstdc++.so.6 => /usr/lib/x86_64-linux-gnu/libstdc++.so.6 (0x00007fa14a2bf000)
            libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007fa149efa000)
            libpthread.so.0 => /lib/x86_64-linux-gnu/libpthread.so.0 (0x00007fa149cdc000)
            libicui18n.so.52 => /usr/lib/x86_64-linux-gnu/libicui18n.so.52 (0x00007fa1498d4000)
            libicuuc.so.52 => /usr/lib/x86_64-linux-gnu/libicuuc.so.52 (0x00007fa14955b000)
            libdl.so.2 => /lib/x86_64-linux-gnu/libdl.so.2 (0x00007fa149357000)
            libglib-2.0.so.0 => /lib/x86_64-linux-gnu/libglib-2.0.so.0 (0x00007fa14904e000)
            librt.so.1 => /lib/x86_64-linux-gnu/librt.so.1 (0x00007fa148e46000)
            libm.so.6 => /lib/x86_64-linux-gnu/libm.so.6 (0x00007fa148b40000)
            libgcc_s.so.1 => /lib/x86_64-linux-gnu/libgcc_s.so.1 (0x00007fa148929000)
            /lib64/ld-linux-x86-64.so.2 (0x00007fa14bfe6000)
            libgobject-2.0.so.0 => /usr/lib/x86_64-linux-gnu/libgobject-2.0.so.0 (0x00007fa1486d8000)
            libX11.so.6 => /usr/lib/x86_64-linux-gnu/libX11.so.6 (0x00007fa1483a3000)
            libgpg-error.so.0 => /lib/x86_64-linux-gnu/libgpg-error.so.0 (0x00007fa14819d000)
            libpng12.so.0 => /lib/x86_64-linux-gnu/libpng12.so.0 (0x00007fa147f77000)
            libharfbuzz.so.0 => /usr/lib/x86_64-linux-gnu/libharfbuzz.so.0 (0x00007fa147d22000)
            libGL.so.1 => /usr/lib/x86_64-linux-gnu/mesa/libGL.so.1 (0x00007fa147abb000)
            libicudata.so.52 => /usr/lib/x86_64-linux-gnu/libicudata.so.52 (0x00007fa14624e000)
            libpcre.so.3 => /lib/x86_64-linux-gnu/libpcre.so.3 (0x00007fa14600f000)
            libffi.so.6 => /usr/lib/x86_64-linux-gnu/libffi.so.6 (0x00007fa145e07000)
            libxcb.so.1 => /usr/lib/x86_64-linux-gnu/libxcb.so.1 (0x00007fa145be8000)
            libfreetype.so.6 => /usr/lib/x86_64-linux-gnu/libfreetype.so.6 (0x00007fa145944000)
            libgraphite2.so.3 => /usr/lib/x86_64-linux-gnu/libgraphite2.so.3 (0x00007fa145728000)
            libglapi.so.0 => /usr/lib/x86_64-linux-gnu/libglapi.so.0 (0x00007fa145500000)
            libXext.so.6 => /usr/lib/x86_64-linux-gnu/libXext.so.6 (0x00007fa1452ee000)
            libXdamage.so.1 => /usr/lib/x86_64-linux-gnu/libXdamage.so.1 (0x00007fa1450eb000)
            libXfixes.so.3 => /usr/lib/x86_64-linux-gnu/libXfixes.so.3 (0x00007fa144ee5000)
            libX11-xcb.so.1 => /usr/lib/x86_64-linux-gnu/libX11-xcb.so.1 (0x00007fa144ce2000)
            libxcb-glx.so.0 => /usr/lib/x86_64-linux-gnu/libxcb-glx.so.0 (0x00007fa144acb000)
            libxcb-dri2.so.0 => /usr/lib/x86_64-linux-gnu/libxcb-dri2.so.0 (0x00007fa1448c6000)
            libxcb-dri3.so.0 => /usr/lib/x86_64-linux-gnu/libxcb-dri3.so.0 (0x00007fa1446c2000)
            libxcb-present.so.0 => /usr/lib/x86_64-linux-gnu/libxcb-present.so.0 (0x00007fa1444bf000)
            libxcb-sync.so.1 => /usr/lib/x86_64-linux-gnu/libxcb-sync.so.1 (0x00007fa1442b9000)
            libxshmfence.so.1 => /usr/lib/x86_64-linux-gnu/libxshmfence.so.1 (0x00007fa1440b6000)
            libXxf86vm.so.1 => /usr/lib/x86_64-linux-gnu/libXxf86vm.so.1 (0x00007fa143eb0000)
            libdrm.so.2 => /usr/lib/x86_64-linux-gnu/libdrm.so.2 (0x00007fa143ca4000)
            libXau.so.6 => /usr/lib/x86_64-linux-gnu/libXau.so.6 (0x00007fa143a9f000)
            libXdmcp.so.6 => /usr/lib/x86_64-linux-gnu/libXdmcp.so.6 (0x00007fa143899000)

Migrating from KeePassX 0.4.x
=============================

KeePassX 2 uses a new database file format .kdbx. The old format .kdb is no longer compatible with KeePassX 2. However KeePassX 2 provides a way of importing the data from the old format. Instructions below:

1. Launch KeePassX 2. 
2. Click on Database --> Import KeePass 1 Database
3. Browse and pick your .kdb file and provide the passphrase and/or key file
4. Save the database again with a different name. This saves the database as a .kdbx file. 

Obligatory Caution:
-------------------

> KeePassX 2 is Beta Software. It's quite stable (as of Sept. 22, 2015) but it is recommended that you take frequent backups of your KeePassX database just to be safe.  
