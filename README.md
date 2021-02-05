# Authselect

*Authselect is a tool to select system authentication and identity sources from a list of supported profiles.*

It is designed to be **a replacement for authconfig** (which is the default tool for this job on Fedora and RHEL based systems) but it takes a different approach to configure the system. Instead of letting the administrator build the PAM stack with a tool (which may potentially end up with a broken configuration), *it would ship several tested stacks* (profiles) that solve a use-case and are **well tested and supported**. At the same time, some obsolete features of authconfig would not be supported by **authselect**.

This tool aims to be first shipped along and later deprecate and later replace authconfig in a future Fedora release.

## Prerequisites

Authselect requires few packages to be installed during build time. To install them on a Fedora machine, run:

```bash
$ sudo dnf install \
    autoconf       \
    automake       \
    libtool        \
    m4             \
    pkgconfig      \
    gettext-devel  \
    popt-devel     \
    asciidoc
```

## Checkout the code

To check out the code from a GitHub git repository to your local machine, run the following command:

```bash
$ git clone https://github.com/authselect/authselect.git
```

## Compile, build and install authselect

After you checkout the code, you can build, install and run **authselect** on your system with these commands:

```bash
$ cd authselect
$ autoreconf -iv
$ ./configure --enable-silent-rules
$ make
$ sudo make install
```

This will make **authselect** available on your system and running this tool will modify your system configuration. If you only want to test this tool to see how it behaves and what it does without actually modifying anything on your system, we recommend you to replace `./configure --enable-silent-rules` with `./configure --enable-silent-rules --prefix="/path/to/install/directory"` which makes installation location in `/path/to/install/directory` and all changes will be confined inside this directory.

```bash
$ cd authselect
$ autoreconf -iv
$ ./configure --enable-silent-rules --prefix="/path/to/install/directory"
$ make
$ make install
```

## Testing authselect

Before you tryout the tool, checkout its manual pages `man authselect` and its command line help with `sudo authselect --help`. Authselect needs to be run as root so it can perform system-wide changes.

The most important commands are:

```bash
# List all available profiles
$ sudo authselect list

# See what changes will be done by activating profile named $profilename
$ sudo authselect test $profilename

# Activate a profile named $profilename on the system
$ sudo authselect select $profilename
```

## Contribution

Any contribution to authselect is welcome. We use git and [GitHub flow](https://help.github.com/articles/github-flow/) for development.

* If you want to report a bug or request a new feature to be implemented, please open an issue here: [Issues](https://github.com/authselect/authselect/issues).
* If you want to submit a patch, please open a new pull request here: [Pull Requests](https://github.com/authselect/authselect/pulls).
