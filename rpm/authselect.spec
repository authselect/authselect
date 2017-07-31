
Name:           authselect
Version:        0.1
Release:        1%{?dist}
Summary:        Configures authentication and identity sources from supported profiles

License:        GPLv3+
URL:            https://github.com/pbrezina/authselect
# FIXME
Source0:        https://github.com/pbrezina/authselect/archive/%{name}-%{version}.tar.gz

BuildRequires:  autoconf
BuildRequires:  automake
BuildRequires:  libtool
BuildRequires:  m4
BuildRequires:  pkgconfig
BuildRequires:  popt-devel
BuildRequires:  gettext-devel
BuildRequires:  asciidoc


%description
Authconfig is designed to be a replacement for authconfig but it takes a different
approach to configure the system. Instead of letting the administrator
build the pam stack with a tool (which may potentially end up with a
broken configuration), it would ship several tested stacks (profiles)
that solve a use-case and are well tested and supported. At the same time,
some obsolete features of authconfig are not supported by authselect.

%package libs
Summary: Utility library used by the authselect tool
Group: Applications/System
License: GPLv3+

%description libs
Common library files for authselect. This package is used by the authselect
command line tool and any other potential front-ends.

%package devel
Summary: Development libraries and headers for authselect
Group: Applications/System
License: GPLv3+
Requires: authselect-libs = %{version}-%{release}

%description devel
System header files and development libraries for authselect. Useful if
you develop a front-end for the authselect library.


%prep
%setup -q


%build
autoreconf -if
%configure

make %{?_smp_mflags}


%install
make install DESTDIR=$RPM_BUILD_ROOT

# Remove .la and .a files created by libtool
find $RPM_BUILD_ROOT -name "*.la" -exec rm -f {} \;
find $RPM_BUILD_ROOT -name "*.a" -exec rm -f {} \;

%files libs
%defattr(-,root,root,-)
%dir %{_sysconfdir}/authselect
%dir %{_datadir}/authselect
%dir %{_datadir}/authselect/custom
%dir %{_datadir}/authselect/vendor
%dir %{_datadir}/authselect/default
%dir %{_datadir}/authselect/default/sssd/
%{_datadir}/authselect/default/sssd/README
%{_datadir}/authselect/default/sssd/nsswitch.conf
%{_datadir}/authselect/default/sssd/password-auth
%{_datadir}/authselect/default/sssd/system-auth
%{_datadir}/authselect/default/winbind/README
%{_datadir}/authselect/default/winbind/nsswitch.conf
%{_datadir}/authselect/default/winbind/password-auth
%{_datadir}/authselect/default/winbind/system-auth
%{_datadir}/authselect/default/sssd+fprint/README
%{_datadir}/authselect/default/sssd+fprint/nsswitch.conf
%{_datadir}/authselect/default/sssd+fprint/password-auth
%{_datadir}/authselect/default/sssd+fprint/system-auth
%{_datadir}/authselect/default/sssd+fprint/fingerprint-auth
%{_datadir}/authselect/default/winbind+fprint/README
%{_datadir}/authselect/default/winbind+fprint/nsswitch.conf
%{_datadir}/authselect/default/winbind+fprint/password-auth
%{_datadir}/authselect/default/winbind+fprint/system-auth
%{_datadir}/authselect/default/winbind+fprint/fingerprint-auth
%{_libdir}/libauthselect.so.*
%{_datadir}/doc/authselect/COPYING
%license COPYING

%files devel
%defattr(-,root,root,-)
%{_includedir}/authselect.h
%{_libdir}/libauthselect.so

%files
%defattr(-,root,root,-)
%{_bindir}/authselect
%{_mandir}/man8/authselect.8*

%post libs -p /sbin/ldconfig

%changelog
* Mon Jul 31 2017 Jakub Hrozek <jakub.hrozek@posteo.se> - 0.1-1
- initial packaging
