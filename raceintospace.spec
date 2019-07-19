%global commit 9f84568
%global date 20190719
%global snapinfo %{date}git%{commit}

Name:		Race into space
Version:	0
Release:	1%{?snapinfo:.%{snapinfo}}%{?dist}
Summary:	Race into Space is the free software version of Interplay's Buzz Aldrin's Race into Space

License:	GPLv2+
URL:		https://github.com/raceintospace/raceintospace
Source0:	

BuildRequires:	cmake
BuildRequires:	SDL-devel protobuf-devel
BuildRequires:	libogg-devel libvorbis-devel libtheora-devel jsoncpp-devel
Requires:	SDL

%description
Race into Space is the free software version of Interplay's Buzz Aldrin's Race into Space.
This is the reworked version following the source release for the computer version of
the Liftoff! board game by Fritz Bronner. This was developed by Strategic Visions
and published by Interplay as a disk-based game in 1992 and a CD-ROM in 1994.

%prep
%cmake


%build
make %{?_smp_mflags}


%install
%make_install


%files
%doc



%changelog

