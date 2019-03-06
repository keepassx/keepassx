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

7. KeePassX binary should have been installed at `/usr/local/bin/keepassx`. You can create a symbolic link to it at /usr/bin. Just make sure that it doesn't conflict with any previous installations of KeePassX by naming the symbolic link differently. You can also modify the `CMAKE_INSTALL_PREFIX` parameter to /usr if you want the binary to directly be in /usr/bin

Migrating from KeePassX 0.4.x
=============================

KeePassX 2 uses a new database file format `.kdbx`. The old format `.kdb` is no longer compatible with KeePassX 2. However KeePassX 2 provides a way of importing the data from the old format. Instructions below:

1. Launch KeePassX 2. 
2. Click on Database --> Import KeePass 1 Database
3. Browse and pick your .kdb file and provide the passphrase and/or key file
4. Save the database again with a different name. This saves the database as a .kdbx file. 

Obligatory Caution:
-------------------

> KeePassX 2 is Beta Software. It's quite stable (as of Sept. 22, 2015) but it is recommended that you take frequent backups of your KeePassX database just to be safe.  
