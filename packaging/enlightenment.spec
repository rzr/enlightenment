Name:           enlightenment
Version:        0.18.2
Release:        0
License:        BSD 2-clause
Summary:        The Enlightenment window manager
Url:            http://www.enlightenment.org/
Group:          Graphics/EFL
Source0:        enlightenment-%{version}.tar.bz2
Source1001: 	enlightenment.manifest
BuildRequires:  doxygen
BuildRequires:  fdupes
BuildRequires:  gettext
BuildRequires:  pam-devel
BuildRequires:  pkgconfig(alsa)
BuildRequires:  pkgconfig(dbus-1)
BuildRequires:  pkgconfig(ecore)
BuildRequires:  pkgconfig(ecore-con)
BuildRequires:  pkgconfig(ecore-evas)
BuildRequires:  pkgconfig(ecore-file)
BuildRequires:  pkgconfig(ecore-input)
BuildRequires:  pkgconfig(ecore-input-evas)
BuildRequires:  pkgconfig(ecore-ipc)
#BuildRequires:  pkgconfig(ecore-x)
BuildRequires:  pkgconfig(edbus)
BuildRequires:  pkgconfig(edje)
BuildRequires:  pkgconfig(eet)
BuildRequires:  pkgconfig(eeze)
BuildRequires:  pkgconfig(efreet)
BuildRequires:  pkgconfig(eina)
BuildRequires:  pkgconfig(eio)
BuildRequires:  pkgconfig(evas)
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(emotion)
BuildRequires:  pkgconfig(ice)
BuildRequires:  pkgconfig(libudev)
BuildRequires:  pkgconfig(udev)
BuildRequires:  pkgconfig(x11)
BuildRequires:  pkgconfig(xext)
BuildRequires:  pkgconfig(xcb-keysyms)
Source1:	e17.service
BuildRequires:  eet-tools

%description
Enlightenment is a window manager.

%package devel
Summary:        Development components for the enlightenment package
Group:          Development/Libraries
Requires:       %{name} = %{version}

%description devel
Development files for enlightenment

%prep
%setup -q -n %{name}-%{version}
cp %{SOURCE1001} .


%build

%reconfigure \
    --enable-device-udev \
    --enable-mount-eeze  \
    --enable-comp \
    --enable-wayland-only \
    --enable-wayland-clients \
    #eol

make %{?_smp_mflags} -k

%install
%make_install


mkdir -p %{buildroot}%{_unitdir_user}/core-efl.target.wants

cat > %{buildroot}%{_unitdir_user}/core-efl.target << EOF
[Unit]
Description=EFL Target
Wants=xorg.target
EOF

install -m 0644 %SOURCE1 %{buildroot}%{_unitdir_user}/
ln -s ../e17.service %{buildroot}%{_unitdir_user}/core-efl.target.wants/e17.service

%find_lang enlightenment
%fdupes  %{buildroot}/%{_libdir}/enlightenment
%fdupes  %{buildroot}/%{_datadir}/enlightenment

%lang_package

%files 
%manifest %{name}.manifest
%defattr(-,root,root,-)
%license COPYING
%config %{_sysconfdir}/enlightenment/sysactions.conf
%{_bindir}/enlightenment*
%{_libdir}/enlightenment/*
%{_datadir}/enlightenment/*
%{_datadir}/xsessions/enlightenment.desktop
%{_sysconfdir}/xdg/menus/enlightenment.menu
/usr/share/applications/enlightenment_filemanager.desktop
%{_unitdir_user}/core-efl.target.wants/*.service
%{_unitdir_user}/core-efl.target
%{_unitdir_user}/*.service

%files devel
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_includedir}/enlightenment/*
%{_libdir}/pkgconfig/*.pc



%changelog
