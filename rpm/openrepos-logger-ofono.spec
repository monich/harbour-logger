Name:       openrepos-logger-ofono
Summary:    Ofono logger
Version:    1.0.17
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

%changelog
* Sun Jul 18 2021 Slava Monich <slava.monich@jolla.com> 1.0.17
- Disable broken in-app sharing
- Freshed up app icon
- Added Chinese translation

* Sat Dec 12 2020 Slava Monich <slava.monich@jolla.com> 1.0.16
- Show page header when scrolling
- Added "Jump to bottom" button
- Fixed email sharing with latest Sailfish OS
- Fixed build of translations with newer Qt

* Sun Mar 10 2019 Slava Monich <slava.monich@jolla.com> 1.0.14
- Dump versions of plugins and related libraries

* Tue Jun 19 2018 Slava Monich <slava.monich@jolla.com> 1.0.12
- Save Modem properties
- Freshened up sharing menu
- Updated Polish translations
- Added Dutch translations

* Sat Aug 26 2017 Slava Monich <slava.monich@jolla.com> 1.0.11
- Run as nemo:privileged

* Sat Aug 20 2017 Slava Monich <slava.monich@jolla.com> 1.0.10
- Filter for categories
- Polish translations

* Sat Jun 10 2017 Slava Monich <slava.monich@jolla.com> 1.0.9
- Added Swedish translations
- Add /etc/hw-release to the tarball
- Added autoEnableLogging and autoResetLogging options
- A few settings page fixes

* Mon Jan 23 2017 Slava Monich <slava.monich@jolla.com> 1.0.8
- Save the state of the connmans's cellular technology
- Another connectivity fix

* Wed Jan 11 2017 Slava Monich <slava.monich@jolla.com> 1.0.7
- Don't directly invoke Qt code from glib callbacks
- Make sure that verbose trace is enabled
- Save rilerror file

* Tue Oct 25 2016 Slava Monich <slava.monich@jolla.com> 1.0.6
- Save connection context configuration

* Tue Oct 25 2016 Slava Monich <slava.monich@jolla.com> 1.0.5
- Added "Fix mobile data" menu item
- Collect more ofono information

* Mon Oct 10 2016 Slava Monich <slava.monich@jolla.com> 1.0.4
- Added default email address

* Sun Oct 09 2016 Slava Monich <slava.monich@jolla.com> 1.0.3
- Compatibility with Qt 5.6

* Thu Sep 08 2016 Slava Monich <slava.monich@jolla.com> 1.0.2
- Made font size configurable

* Thu Sep 08 2016 Slava Monich <slava.monich@jolla.com> 1.0.1
- Fixed a few visual glitches
- Made screen buffer size configurable
- Added openrepos build with settings plugin

* Wed May 25 2016 Slava Monich <slava.monich@jolla.com> 1.0.0
- Initial release
