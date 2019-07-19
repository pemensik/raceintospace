%global commit bf6c86a
%global date 20190719
%global snapinfo %{date}git%{commit}
%global pkgversion %{version}-git%{commit}

Name:		raceintospace
Version:	1.1.0
Release:	1%{?snapinfo:.%{snapinfo}}%{?dist}
Summary:	Race into Space game

License:	GPLv2+
URL:		https://github.com/raceintospace/raceintospace
Source0:	raceintospace-%{pkgversion}.tar.bz2

BuildRequires:	cmake gcc-c++
BuildRequires:	SDL-devel protobuf-devel boost-devel
BuildRequires:	libogg-devel libvorbis-devel libtheora-devel jsoncpp-devel
BuildRequires:	physfs-devel libpng-devel
Requires:	SDL
Requires:	%{name}-data = %{version}-%{release}

%description
Race into Space is the free software version of Interplay's Buzz Aldrin's Race into Space.
This is the reworked version following the source release for the computer version of
the Liftoff! board game by Fritz Bronner. This was developed by Strategic Visions
and published by Interplay as a disk-based game in 1992 and a CD-ROM in 1994.

%package data
BuildArch:	noarch
Summary:	Race into Space game data

%description data
Race into Space is the free software version of Interplay's Buzz Aldrin's Race into Space.
This is the reworked version following the source release for the computer version of
the Liftoff! board game by Fritz Bronner. This was developed by Strategic Visions
and published by Interplay as a disk-based game in 1992 and a CD-ROM in 1994.

Contains shared game data.

%prep
%autosetup -p1 -n %{name}-%{pkgversion}
mkdir build
pushd build
%cmake -DBUILD_PHYSFS=OFF ..
popd

%build
pushd build
make %{?_smp_mflags}
popd

%install
pushd build
%make_install
popd

%files
%doc AUTHORS README.md
%license COPYING
%{_bindir}/raceintospace

%files data
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/pixmaps/%{name}.*

%changelog
* Fri Jul 19 2019 Petr Menšík <pemensik@redhat.com> - 1.1.0-1.20190719gitbf6c86a
- Initial version


