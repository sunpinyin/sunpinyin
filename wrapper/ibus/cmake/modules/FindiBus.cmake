# - Try to find iBus
# Once done, this will define
#
#  iBus_FOUND - system has iBus
#  iBus_INCLUDE_DIRS - the iBus include directories
#  iBus_LIBRARIES - link these to use iBus

include(LibFindMacros)

# Dependencies

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(iBus_PKGCONF ibus-1.0)

# Include dir
find_path(iBus_INCLUDE_DIR
  NAMES ibus.h
  PATHS ${iBus_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(iBus_LIBRARY
  NAMES ibus
  PATHS ${iBus_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the
# rest.
# NOTE: Singular variables for this library, plural for libraries this this lib
# depends on.
set(iBus_PROCESS_INCLUDES iBus_INCLUDE_DIR)
set(iBus_PROCESS_LIBS iBus_LIBRARY)
libfind_process(iBus)

