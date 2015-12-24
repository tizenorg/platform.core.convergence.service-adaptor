Name:       service-adaptor
Summary:    Service Adaptor Framework for Convergence
Version:    1.1.4
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
BuildRequires:  pkgconfig(libsmack)
#BuildRequires:  pkgconfig(service-discovery)
#BuildRequires:  pkgconfig(service-federation)
#BuildRequires:  service-discovery-devel
#BuildRequires:  service-federation-devel

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
export CFLAGS="${CFLAGS} -fPIC -Wall -g -fvisibility=hidden -fdata-sections -ffunction-sections"
export CXXFLAGS="${CXXFLAGS} -fPIC -Wall -g -fvisibility=hidden"
export LDFLAGS="${LDFLAGS} -Wl,--hash-style=both -Wl,--rpath=%{_prefix}/lib -Wl,--as-needed"

%if 0%{?sec_build_binary_debug_enable}
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"
%endif

cmake . -DCMAKE_INSTALL_PREFIX=%{_prefix} \
        -DLIB_INSTALL_DIR:PATH=%{_libdir} \
	-DTZ_SYS_USER_GROUP=%TZ_SYS_USER_GROUP \
	-DTZ_SYS_DEFAULT_USER=%TZ_SYS_DEFAULT_USER

make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%make_install

mkdir -p %{buildroot}%{_unitdir_user}/default.target.wants
install -m 0644 %SOURCE1 %{buildroot}%{_unitdir_user}/service-adaptor.service
ln -s ../service-adaptor.service %{buildroot}%{_unitdir_user}/default.target.wants/service-adaptor.service

mkdir -p %{buildroot}%{_datadir}/dbus-1/system-services
install -m 0644 %SOURCE2 %{buildroot}%{_datadir}/dbus-1/system-services/org.tizen.serviceadaptor.client.service

mkdir -p %{buildroot}/usr/share/license
cp LICENSE.APLv2 %{buildroot}/usr/share/license/service-adaptor
cp LICENSE.APLv2 %{buildroot}/usr/share/license/service-adaptor-devel

%post -n service-adaptor
/sbin/ldconfig

%postun -p /sbin/ldconfig

%files -n service-adaptor
%manifest service-adaptor.manifest
%defattr(-,root,root,-)
%{_libdir}/lib*.so.*
#%{_bindir}/service-adaptor-server
#%{_bindir}/sal-test
%{_unitdir_user}/service-adaptor.service
%{_unitdir_user}/default.target.wants/service-adaptor.service
%{_datadir}/dbus-1/system-services/org.tizen.serviceadaptor.client.service
%{_sysconfdir}/dbus-1/system.d/org.tizen.serviceadaptor.client.conf
/usr/share/license/%{name}

%files -n service-adaptor-devel
%defattr(-,root,root,-)
%{_libdir}/lib*.so
%{_libdir}/pkgconfig/service-adaptor.pc
%{_includedir}/service-adaptor/*.h
%{_includedir}/service-provider/*.h
/usr/share/license/%{name}-devel
