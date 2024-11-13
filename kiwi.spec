%global debug_package %{nil}

Name: kiwi
Version: 2.0.9
Release: 1%{?dist}
Summary: The Kiwi Programming Language

License: Your License Here
URL: https://github.com/fuseraft/kiwi
Source0: %{name}-%{version}.tar.gz

%description
The Kiwi programming language interpreter and its standard library.

%prep
%setup -q

%build
# Building handled by Makefile

%install
mkdir -p %{buildroot}/usr/bin
mkdir -p %{buildroot}/usr/lib/kiwi
install -m 0755 bin/kiwi %{buildroot}/usr/bin/kiwi
cp -r lib/* %{buildroot}/usr/lib/kiwi/

%files
/usr/bin/kiwi
/usr/lib/kiwi/*

%changelog
* Tue Nov 5 2024 Scott Stauffer <scott@fuseraft.com> - 2.0.9-1
- Version 2.0.9 release
