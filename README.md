# KeePassX

## About

Fork of [KeePassX](https://www.keepassx.org/) with keepasshttp support for use with [PassIFox](https://addons.mozilla.org/en-us/firefox/addon/passifox/) for Mozilla Firefox and [chromeIPass](https://chrome.google.com/webstore/detail/chromeipass/ompiailgknfdndiefoaoiligalphfdae) for Google Chrome.

KeePassHttp implementation has been forked from jdachtera's repository, which in turn was based on code from code with Francois Ferrand's [keepassx-http](https://gitorious.org/keepassx/keepassx-http/source/master:) repository. 

My intention is to keep this repository as up-to-date with the main keePassX repo as possible and, time allowing, clean-up the keepasshttp implementation enough for it to be merged with upstream. I have started removing any additions to the code that were not strictly related to implemeting the keepasshttp protocol in KeePassX.

## Install

KeePassX can be downloaded and installed using an assortment of installers available on the main [KeePassX website](http://www.keepassx.org).
KeePassX can also be installed from the official repositories of many Linux repositories.
If you wish to build KeePassX from source, rather than rely on the pre-compiled binaries, you may wish to read up on the _From Source_ section.

### Debian

To install KeePassX from the Debian repository:

```bash
sudo apt-get install keepassx
```

### Red Hat

Install KeePassX from the Red Hat (or CentOS) repository:

```bash
sudo yum install keepassx
```

### Windows / Mac OS X

Download the installer from the KeePassX [download](https://www.keepassx.org/downloads) page.
Once downloaded, double click on the file to execute the installer.

### From Source

#### Build Dependencies

The following tools must exist within your PATH:

* make
* cmake (>= 2.8.12)
* g++ (>= 4.7) or clang++ (>= 3.0)

The following libraries are required:

* Qt 5 (>= 5.2): qtbase and qttools5
* libgcrypt (>= 1.6)
* zlib
* libxtst, qtx11extras (optional for auto-type on X11)

On Debian you can install them with:

```bash
sudo apt-get install build-essential cmake qtbase5-dev libqt5x11extras5-dev qttools5-dev qttools5-dev-tools libgcrypt20-dev zlib1g-dev
```

#### Build Steps

To compile from source:

```bash
mkdir build
cd build
cmake ..
make [-jX]
```

You will have the compiled KeePassX binary inside the `./build/src/` directory.

To install this binary execute the following:

```bash
sudo make install
```

More detailed instructions available in the INSTALL file.
