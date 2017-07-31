
Name:           authselect
Version:	0.1
Release:	1%{?dist}
Summary:	Authselect is a tool to select system authentication and identity sources from a list of supported profiles.

License:	GPLv3+
URL:		https://github.com/pbrezina/authselect
# FIXME
#Source0:	https://github.com/pbrezina/authselect/archive/%{version}.tar.gz

BuildRequires:	autoconf
BuildRequires:	automake
BuildRequires:	libtool
BuildRequires:	m4
BuildRequires:	pkgconfig

BuildRequires:	popt-devel


%description
It is designed to be a replacement for authconfig but it takes a different
approach to configure the system. Instead of letting the administrator
build the pam stack with a tool (which may potentially end up with a
broken configuration), it would ship several tested stacks (profiles)
that solve a use-case and are well tested and supported. At the same time,
some obsolete features of authconfig are not supported by authselect.

This tool aims to eventually replace authconfig.

%package libs
Summary: Utility library used by the authselect tool
Group: Applications/System
License: GPLv3+

%description libs
Common library files for authselect. This package is used by the authselect
command line tool and any other potential front-ends.

%prep
%setup -q


%build
autoreconf -if
%configure --with-nsswitch-conf=%{_sysconfdir}/nsswitch.conf \
           ${null}

make %{?_smp_mflags}


%install
make install DESTDIR=$RPM_BUILD_ROOT

%files libs
%defattr(-,root,root,-)
%license COPYING

%files
%defattr(-,root,root,-)
%{_bindir}/authselect
%license COPYING

%changelog
* Mon Jul 31 2017 Jakub Hrozek <jakub.hrozek@posteo.se> - 0.1-1
- initial packaging
