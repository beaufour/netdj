# -*- rpm-spec -*-
%define name	netdj
%define version	0.12
%define release 1

Summary:	NetDJ  - Fully automated streaming radio
Name:		%{name}
Version:	%{version}
Release:	%{release}
Group:		Applications/Multimedia
Copyright:	GPL
URL:		http://netdj.sf.net
Source:		http://prdownloads.sourceforge.net/%{name}/%{name}-%{version}.tar.gz
BuildRoot:	%{_tmppath}/%{name}-root
Prefix:		%{_prefix}
Requires: libshout
BuildRequires: libshout-devel

%description
NetDJ is an fully automated streaming radio, that fetches music through
the Gnutella-network and plays it.

It features both a HTML and XML interface, accessible through an
internal web-server.

%prep
%setup -q

%build
%configure

%install
rm -rf  %{buildroot}
make {?buildroot:DESTDIR=%{buildroot}} install

# System menu entry
mkdir -p %{buildroot}%{_sysconfdir}/X11/applnk/Multimedia
cat > %{buildroot}%{_sysconfdir}/X11/applnk/Multimedia/netdj.desktop << EOF
[Desktop Entry]
Name=NetDJ
Comment=%{summary}
Icon=
Exec=netdj
Terminal=1
Type=Application
EOF


%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%doc AUTHORS COPYING README 
%{_bindir}/*
#%{_sysconfdir}/
%config /etc/netdj/config
%config /etc/netdj/users
%{_mandir}/man1/*

%changelog
* Mon Jan 15 2002 Allan Beaufour Larsen
- Initial RPM release.
