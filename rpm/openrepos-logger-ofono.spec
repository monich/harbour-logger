Name:       openrepos-logger-ofono
Summary:    Ofono logger
Version:    1.0.22
Release:    1
Group:      Applications/System
License:    BSD
URL:        http://github.com/monich/harbour-logger
Vendor:     slava
Source0:    %{name}-%{version}.tar.bz2

Requires:   sailfishsilica-qt5
BuildRequires: pkgconfig(sailfishapp)
BuildRequires: pkgconfig(Qt5Quick)
BuildRequires: pkgconfig(Qt5Qml)
BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5DBus)
BuildRequires: pkgconfig(mlite5)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(gio-2.0)
BuildRequires: pkgconfig(gio-unix-2.0)
BuildRequires: desktop-file-utils
BuildRequires: qt5-qttools-linguist

%description
Application for gathering ofono logs on Sailfish OS

%{!?qtc_qmake:%define qtc_qmake %qmake}
%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}
%{?qtc_builddir:%define _builddir %qtc_builddir}

%if "%{?vendor}" == "chum"
Categories:
 - Utility
Icon: https://raw.githubusercontent.com/monich/harbour-logger/master/ofono/icons/harbour-logger-ofono.svg
Screenshots:
- https://home.monich.net/chum/openrepos-logger-ofono/screenshots/screenshot-001.png
- https://home.monich.net/chum/openrepos-logger-ofono/screenshots/screenshot-002.png
- https://home.monich.net/chum/openrepos-logger-ofono/screenshots/screenshot-003.png
- https://home.monich.net/chum/openrepos-logger-ofono/screenshots/screenshot-004.png
Url:
  Homepage: https://openrepos.net/content/slava/ofono-logger
%endif

%prep
%setup -q -n %{name}-%{version}

%build
%qtc_qmake5 CONFIG+=ofono CONFIG+=openrepos CONFIG+=app_settings
%qtc_make %{?_smp_mflags} logger-ofono

%install
rm -rf %{buildroot}
%qmake5_install -C ofono

desktop-file-install --delete-original \
  --dir %{buildroot}%{_datadir}/applications \
   %{buildroot}%{_datadir}/applications/*.desktop

%files
%global privileges_dir %{_datarootdir}/mapplauncherd/privileges.d
%dir %{privileges_dir}
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_datadir}/%{name}/qml
%{_datadir}/%{name}/settings
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
%{_datadir}/translations/%{name}*.qm
%{_datadir}/jolla-settings/entries/%{name}.json
%{privileges_dir}/%{name}
