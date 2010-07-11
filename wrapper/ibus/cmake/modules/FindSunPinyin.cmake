# - Try to find SunPinyin
# Once done, this will define
#
#  SunPinyin_FOUND - system has iBus
#  SunPinyin_INCLUDE_DIRS - the iBus include directories
#  SunPinyin_LIBRARIES - link these to use iBus

include(LibFindMacros)

# Dependencies

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(SunPinyin_PKGCONF sunpinyin-2.0)

# Include dir
find_path(SunPinyin_INCLUDE_DIR
  NAMES sunpinyin.h
  PATHS ${SunPinyin_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(SunPinyin_LIBRARY
  NAMES sunpinyin
  PATHS ${SunPinyin_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the
# rest.
# NOTE: Singular variables for this library, plural for libraries this this lib
# depends on.
set(SunPinyin_PROCESS_INCLUDES SunPinyin_INCLUDE_DIR)
set(SunPinyin_PROCESS_LIBS SunPinyin_LIBRARY)
libfind_process(SunPinyin)

