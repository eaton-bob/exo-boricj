#
#    exo-boricj - exercice jean-baptiste B
#
#    Copyright (C) 2014 - 2017 Eaton
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along
#    with this program; if not, write to the Free Software Foundation, Inc.,
#    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#

# To build with draft APIs, use "--with drafts" in rpmbuild for local builds or add
#   Macros:
#   %_with_drafts 1
# at the BOTTOM of the OBS prjconf
%bcond_with drafts
%if %{with drafts}
%define DRAFTS yes
%else
%define DRAFTS no
%endif
%define SYSTEMD_UNIT_DIR %(pkg-config --variable=systemdsystemunitdir systemd)
Name:           exo-boricj
Version:        1.0.0
Release:        1
Summary:        exercice jean-baptiste b
License:        GPL-2.0+
URL:            http://example.com/
Source0:        %{name}-%{version}.tar.gz
Group:          System/Libraries
# Note: ghostscript is required by graphviz which is required by
#       asciidoc. On Fedora 24 the ghostscript dependencies cannot
#       be resolved automatically. Thus add working dependency here!
BuildRequires:  ghostscript
BuildRequires:  asciidoc
BuildRequires:  automake
BuildRequires:  autoconf
BuildRequires:  libtool
BuildRequires:  pkgconfig
BuildRequires:  systemd-devel
BuildRequires:  systemd
%{?systemd_requires}
BuildRequires:  xmlto
BuildRequires:  zeromq-devel
BuildRequires:  czmq-devel
BuildRequires:  malamute-devel
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

%description
exo-boricj exercice jean-baptiste b.

%package -n libexo_boricj0
Group:          System/Libraries
Summary:        exercice jean-baptiste b shared library

%description -n libexo_boricj0
This package contains shared library for exo-boricj: exercice jean-baptiste b

%post -n libexo_boricj0 -p /sbin/ldconfig
%postun -n libexo_boricj0 -p /sbin/ldconfig

%files -n libexo_boricj0
%defattr(-,root,root)
%{_libdir}/libexo_boricj.so.*

%package devel
Summary:        exercice jean-baptiste b
Group:          System/Libraries
Requires:       libexo_boricj0 = %{version}
Requires:       zeromq-devel
Requires:       czmq-devel
Requires:       malamute-devel

%description devel
exercice jean-baptiste b development tools
This package contains development files for exo-boricj: exercice jean-baptiste b

%files devel
%defattr(-,root,root)
%{_includedir}/*
%{_libdir}/libexo_boricj.so
%{_libdir}/pkgconfig/libexo_boricj.pc
%{_mandir}/man3/*
%{_mandir}/man7/*

%prep

%setup -q

%build
sh autogen.sh
%{configure} --enable-drafts=%{DRAFTS} --with-systemd-units
make %{_smp_mflags}

%install
make install DESTDIR=%{buildroot} %{?_smp_mflags}

# remove static libraries
find %{buildroot} -name '*.a' | xargs rm -f
find %{buildroot} -name '*.la' | xargs rm -f

%files
%defattr(-,root,root)
%{_bindir}/exo-boricj
%{_mandir}/man1/exo-boricj*
%{_bindir}/exo-boricj-client
%{_mandir}/man1/exo-boricj-client*
%config(noreplace) %{_sysconfdir}/exo-boricj/exo-boricj.cfg
%{SYSTEMD_UNIT_DIR}/exo-boricj.service
%config(noreplace) %{_sysconfdir}/exo-boricj/exo-boricj-client.cfg
%{SYSTEMD_UNIT_DIR}/exo-boricj-client.service
%dir %{_sysconfdir}/exo-boricj
%if 0%{?suse_version} > 1315
%post
%systemd_post exo-boricj.service exo-boricj-client.service
%preun
%systemd_preun exo-boricj.service exo-boricj-client.service
%postun
%systemd_postun_with_restart exo-boricj.service exo-boricj-client.service
%endif

%changelog
