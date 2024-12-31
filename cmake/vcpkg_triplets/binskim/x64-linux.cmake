set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_C_FLAGS "-Wp,-D_FORTIFY_SOURCE=2 -Wp,-D_GLIBCXX_ASSERTIONS -fstack-protector-strong -fstack-clash-protection -fcf-protection")
set(VCPKG_CXX_FLAGS "-Wp,-D_FORTIFY_SOURCE=2 -Wp,-D_GLIBCXX_ASSERTIONS -fstack-protector-strong -fstack-clash-protection -fcf-protection")
set(VCPKG_CMAKE_SYSTEM_NAME Linux)
set(VCPKG_LINKER_FLAGS "-Wl,-Bsymbolic-functions -Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack")
