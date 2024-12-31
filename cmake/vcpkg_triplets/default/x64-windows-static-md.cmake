set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_C_FLAGS "/MP /DWIN32 /D_WINDOWS /DWINAPI_FAMILY=100 /DWINVER=0x0A00 /D_WIN32_WINNT=0x0A00 /DNTDDI_VERSION=0x0A000000")
set(VCPKG_CXX_FLAGS "/MP /DWIN32 /D_WINDOWS /DWINAPI_FAMILY=100 /DWINVER=0x0A00 /D_WIN32_WINNT=0x0A00 /DNTDDI_VERSION=0x0A000000")
list(APPEND VCPKG_CMAKE_CONFIGURE_OPTIONS --compile-no-warning-as-error -DBENCHMARK_ENABLE_WERROR=OFF)
