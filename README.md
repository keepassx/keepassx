# KeePassX

## About

KeePassX is a password manager for people with extremely high demands on security.
It is light weight, cross platform and published under the terms of the GNU General Public License.

Along with passwords, each entry in KeePassX can include associated usernames, urls, an attachement and comments.
Entries can be grouped and labeled with user-defined titles and icons to simplify organization.
The search function helps find passwords, and it can be limited to single groups.
And generating new passwords is, easy with the customizable password generator.

KeePassX uses a database format that is compatible with [KeePass Password Safe](http://keepass.info/).
The database is encrypted with the AES block cipher using a 256 bit key.

## Installation

KeePassX can be downloaded and installed using an assortment of installers available on the [KeePassX website](http://www.keepassx.org).
If you wish to build KeePassX from source, you may wish to read up on the _From Source_ section.
KeePassX can also be installed from the software repositories of many Linux distributions.
Instructions for installing KeePassX on various operating systems are below.


### Debian based distributions (including Ubuntu, Linux Mint, etc.)

At the terminal enter:

```bash
sudo apt-get install keepassx
```

### Red Hat based distributions (including Fedora, CentOS, etc.)

At the terminal enter:

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

## Contribute

Coordination of work between developers is handled through the [KeePassX development](https://www.keepassx.org/dev/) site.
Requests for enhancements, or reports of bugs encountered, can also be reported through the KeePassX development site.
However, members of the open-source community are encouraged to submit pull requests directly through GitHub.

### Clone Repository

Clone the repository to a suitable location where you can extend and build this project.

```bash
git clone https://github.com/keepassx/keepassx.git
```

**Note:** This will clone the entire contents of the repository at the HEAD revision.

To update the project from within the project's folder you can run the following command:

```bash
git pull
```

### Feature Requests

We're always looking for suggestions to improve our application. If you have a suggestion for improving an existing feature,
or would like to suggest a completely new feature for KeePassX, please file a ticket on the [KeePassX development](https://www.keepassx.org/dev/) site.

### Bug Reports

Our software isn't always perfect, but we strive to always improve our work. You may file bug reports on the [KeePassX development](https://www.keepassx.org/dev/) site.

### Pull Requests

In addition to feedback and suggestions, we welcome pull requests against our [GitHub repository](https://github.com/keepassx/keepassx).

### Translations

To help make KeePassX accessible to everyone, you contribute translations on our [Transifex](https://www.transifex.com/projects/p/keepassx/) page.
