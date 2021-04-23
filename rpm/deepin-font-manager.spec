%define libname libdeepin-font-manager
%define pkgrelease  1
%if 0%{?openeuler}
%define specrelease %{pkgrelease}
%else
## allow specrelease to have configurable %%{?dist} tag in other distribution
%define specrelease %{pkgrelease}%{?dist}
%endif

Name:           deepin-font-manager
Version:        5.8.0.25
Release:        %{specrelease}
Summary:        Deepin Font Manager is used to install and uninstall font file for users with bulk install function
License:        GPLv3+
URL:            https://github.com/linuxdeepin/%{name}
Source0:        %{url}/archive/%{version}/%{name}-%{version}.tar.gz

BuildRequires: gcc-c++
BuildRequires: qt5-devel

BuildRequires: pkgconfig(dtkwidget)
BuildRequires: pkgconfig(dtkgui)
BuildRequires: pkgconfig(gsettings-qt)
BuildRequires: pkgconfig(freetype2)
BuildRequires: pkgconfig(fontconfig)
BuildRequires: pkgconfig(dde-file-manager)
BuildRequires: deepin-gettext-tools
BuildRequires: gtest-devel

%description
%{summary}.

%package -n %{libname}
Summary:        %{summary}
%description -n %{libname}
%{summary}.

%package -n %{libname}-devel
Summary:        %{summary}
%description -n %{libname}-devel
%{summary}.


%prep
%autosetup

%build
export PATH=%{_qt5_bindir}:$PATH
sed -i "s|^cmake_minimum_required.*|cmake_minimum_required(VERSION 3.0)|" $(find . -name "CMakeLists.txt")
mkdir build && pushd build 
%cmake -DCMAKE_BUILD_TYPE=Release ../  -DAPP_VERSION=%{version} -DVERSION=%{version} 
%make_build  
popd

%install
%make_install -C build INSTALL_ROOT="%buildroot"

%files
%doc README.md
%license LICENSE
%{_bindir}/%{name}
%{_libdir}/dde-file-manager/plugins/previews/libdeepin-font-preview-plugin.so
%{_datadir}/icons/hicolor/scalable/apps/%{name}.svg
%{_datadir}/%{name}/translations/*.qm
%{_datadir}/applications/%{name}.desktop
%{_datadir}/glib-2.0/schemas/com.deepin.font-manager.gschema.xml


%files -n %{libname}
%{_libdir}/%{libname}.so.*
%{_datadir}/deepin-font-manager/CONTENTS.txt

%files -n %{libname}-devel
%{_includedir}/%{name}/
%{_libdir}/%{libname}.so
%{_libdir}/pkgconfig/%{name}.pc

%changelog
* Tue Apr 20 2021 zhangdingwen <zhangdingwen@uniontech.com> - 5.8.0.25-1
- init spec for euler
