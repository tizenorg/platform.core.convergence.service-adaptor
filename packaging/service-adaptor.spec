Name:       service-adaptor
Summary:    Service Adaptor Framework for Convergence
Version:    1.1.5
Release:    1
Group:      System/Libraries
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1:    service-adaptor.service
Source2:    org.tizen.serviceadaptor.client.service

BuildRequires:  cmake
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(gio-2.0)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(gobject-2.0)
BuildRequires:  pkgconfig(gthread-2.0)
BuildRequires:  pkgconfig(bundle)
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(capi-appfw-application)
BuildRequires:  pkgconfig(capi-appfw-app-manager)
BuildRequires:  pkgconfig(capi-appfw-package-manager)
BuildRequires:  pkgconfig(capi-appfw-service-application)
BuildRequires:  pkgconfig(json-glib-1.0)
BuildRequires:  pkgconfig(cynara-client)
BuildRequires:  pkgconfig(cynara-session)
BuildRequires:  pkgconfig(cynara-creds-gdbus)
BuildRequires:  pkgconfig(libtzplatform-config)

%description
Service Adaptor Framework Library/Binary package

%package -n service-adaptor-devel
Summary:    Headers for Service Adaptor Framework (devel)
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description -n service-adaptor-devel
This package contains the header and pc files of Service Adaptor.

%prep
%setup -q

%build
export CFLAGS="${CFLAGS} -fPIC -Wall -g -fdata-sections -ffunction-sections"
export CXXFLAGS="${CXXFLAGS} -fPIC -Wall -g"
export LDFLAGS="${LDFLAGS} -Wl,--hash-style=both -Wl,--rpath=%{_prefix}/lib -Wl,--as-needed"

%if 0%{?sec_build_binary_debug_enable}
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"
%endif

cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix} \
        -DLIB_INSTALL_DIR:PATH=%{_libdir}

make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%make_install

mkdir -p %{buildroot}%{_libdir}/service-provider/auth
mkdir -p %{buildroot}%{_libdir}/service-provider/storage
mkdir -p %{buildroot}%{_libdir}/service-provider/contact
mkdir -p %{buildroot}%{_libdir}/service-provider/message
mkdir -p %{buildroot}%{_libdir}/service-provider/push
mkdir -p %{buildroot}%{_libdir}/service-provider/shop

mkdir -p %{buildroot}%{_datadir}/dbus-1/system-services
install -m 0644 %SOURCE2 %{buildroot}%{_datadir}/dbus-1/system-services/org.tizen.serviceadaptor.client.service

mkdir -p %{buildroot}%{_unitdir}/multi-user.target.wants
install -m 0644 %SOURCE1 %{buildroot}%{_unitdir}/service-adaptor.service
%install_service multi-user.target.wants service-adaptor.service

mkdir -p %{buildroot}/%{TZ_SYS_RO_SHARE}/license
cp LICENSE.APLv2 %{buildroot}/%{TZ_SYS_RO_SHARE}/license/service-adaptor
cp LICENSE.APLv2 %{buildroot}/%{TZ_SYS_RO_SHARE}/license/service-adaptor-devel

%post -n service-adaptor
/sbin/ldconfig

%postun -p /sbin/ldconfig

%files -n service-adaptor
%manifest service-adaptor.manifest
%defattr(-,root,root,-)
%{_libdir}/lib*.so.*
%{_libdir}/service-provider/*
%{_bindir}/service-adaptor-server
%{_unitdir}/service-adaptor.service
%{_unitdir}/multi-user.target.wants/service-adaptor.service
%{_datadir}/dbus-1/system-services/org.tizen.serviceadaptor.client.service
%{_sysconfdir}/dbus-1/system.d/org.tizen.serviceadaptor.client.conf
%{TZ_SYS_RO_SHARE}/license/%{name}
%{_includedir}/*.h

%files -n service-adaptor-devel
%defattr(-,root,root,-)
%{_libdir}/lib*.so
%{_libdir}/pkgconfig/service-adaptor.pc
%{_includedir}/*.h
%{_includedir}/service-adaptor/*.h
%{_includedir}/service-provider/*.h
%{TZ_SYS_RO_SHARE}/license/%{name}-devel
