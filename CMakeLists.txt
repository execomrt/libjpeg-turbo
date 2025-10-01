cmake_minimum_required(VERSION 2.8.12...3.28)

if(CMAKE_EXECUTABLE_SUFFIX)
  set(CMAKE_EXECUTABLE_SUFFIX_TMP ${CMAKE_EXECUTABLE_SUFFIX})
endif()

set(CMAKE_C_FLAGS_RELEASE_INITIALIZED_TO_DEFAULT 1)
if(CMAKE_C_FLAGS_RELEASE)
  set(CMAKE_C_FLAGS_RELEASE_INITIALIZED_TO_DEFAULT 0)
endif()
set(CMAKE_C_FLAGS_RELWITHDEBINFO_INITIALIZED_TO_DEFAULT 1)
if(CMAKE_C_FLAGS_RELWITHDEBINFO)
  set(CMAKE_C_FLAGS_RELWITHDEBINFO_INITIALIZED_TO_DEFAULT 0)
endif()

project(libjpeg-turbo C)
set(VERSION 3.1.2)
set(COPYRIGHT_YEAR "1991-2025")
string(REPLACE "." ";" VERSION_TRIPLET ${VERSION})
list(GET VERSION_TRIPLET 0 VERSION_MAJOR)
list(GET VERSION_TRIPLET 1 VERSION_MINOR)
list(GET VERSION_TRIPLET 2 VERSION_REVISION)
function(pad_number NUMBER OUTPUT_LEN)
  string(LENGTH "${${NUMBER}}" INPUT_LEN)
  if(INPUT_LEN LESS OUTPUT_LEN)
    math(EXPR ZEROES "${OUTPUT_LEN} - ${INPUT_LEN} - 1")
    set(NUM ${${NUMBER}})
    foreach(C RANGE ${ZEROES})
      set(NUM "0${NUM}")
    endforeach()
    set(${NUMBER} ${NUM} PARENT_SCOPE)
  endif()
endfunction()
set(TMP_VERSION_MINOR ${VERSION_MINOR})
set(TMP_VERSION_REVISION ${VERSION_REVISION})
pad_number(TMP_VERSION_MINOR 3)
pad_number(TMP_VERSION_REVISION 3)
set(LIBJPEG_TURBO_VERSION_NUMBER
  ${VERSION_MAJOR}${TMP_VERSION_MINOR}${TMP_VERSION_REVISION})
set(MSVC_LIKE 0)
if(MSVC OR CMAKE_C_SIMULATE_ID STREQUAL "MSVC")
  set(MSVC_LIKE 1)
endif()

# FETCHCONTENT COMPATIBILITY: Allow use as subdirectory
if(NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(LIBJPEG_TURBO_IS_SUBPROJECT TRUE)
  message(STATUS "libjpeg-turbo: Building as subproject")
else()
  set(LIBJPEG_TURBO_IS_SUBPROJECT FALSE)
endif()

# CMake 3.14 and later sets CMAKE_MACOSX_BUNDLE to TRUE by default when
# CMAKE_SYSTEM_NAME is iOS, tvOS, or watchOS, which breaks the libjpeg-turbo
# build.
set(CMAKE_MACOSX_BUNDLE FALSE)

get_property(GENERATOR_IS_MULTI_CONFIG GLOBAL PROPERTY
  GENERATOR_IS_MULTI_CONFIG)
if(NOT GENERATOR_IS_MULTI_CONFIG AND (MSVC_IDE OR XCODE))
  set(GENERATOR_IS_MULTI_CONFIG TRUE)
endif()

string(TIMESTAMP DEFAULT_BUILD "%Y%m%d")
set(BUILD ${DEFAULT_BUILD} CACHE STRING "Build string (default: ${DEFAULT_BUILD})")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

message(STATUS "VERSION = ${VERSION}, BUILD = ${BUILD}")

include(cmakescripts/PackageInfo.cmake)

# Detect CPU type and whether we're building 64-bit or 32-bit code
math(EXPR BITS "${CMAKE_SIZEOF_VOID_P} * 8")
string(TOLOWER ${CMAKE_SYSTEM_PROCESSOR} CMAKE_SYSTEM_PROCESSOR_LC)
set(COUNT 1)
foreach(ARCH ${CMAKE_OSX_ARCHITECTURES})
  if(COUNT GREATER 1)
    message(FATAL_ERROR "libjpeg-turbo contains assembly code, so it cannot be built with multiple values in CMAKE_OSX_ARCHITECTURES.")
  endif()
  math(EXPR COUNT "${COUNT}+1")
endforeach()
if(CMAKE_SYSTEM_PROCESSOR_LC MATCHES "x86_64" OR
  CMAKE_SYSTEM_PROCESSOR_LC MATCHES "amd64" OR
  CMAKE_SYSTEM_PROCESSOR_LC MATCHES "i[0-9]86" OR
  CMAKE_SYSTEM_PROCESSOR_LC MATCHES "x86" OR
  CMAKE_SYSTEM_PROCESSOR_LC MATCHES "ia32")
  if(BITS EQUAL 64 OR CMAKE_C_COMPILER_ABI MATCHES "ELF X32")
    set(CPU_TYPE x86_64)
  else()
    set(CPU_TYPE i386)
  endif()
  if(NOT CMAKE_SYSTEM_PROCESSOR STREQUAL ${CPU_TYPE})
    set(CMAKE_SYSTEM_PROCESSOR ${CPU_TYPE})
  endif()
elseif(CMAKE_SYSTEM_PROCESSOR_LC STREQUAL "aarch64" OR
  CMAKE_SYSTEM_PROCESSOR_LC MATCHES "^arm")
  if(BITS EQUAL 64)
    set(CPU_TYPE arm64)
  else()
    set(CPU_TYPE arm)
  endif()
elseif(CMAKE_SYSTEM_PROCESSOR_LC MATCHES "^ppc" OR
  CMAKE_SYSTEM_PROCESSOR_LC MATCHES "^powerpc")
  set(CPU_TYPE powerpc)
else()
  set(CPU_TYPE ${CMAKE_SYSTEM_PROCESSOR_LC})
endif()
if(CMAKE_OSX_ARCHITECTURES MATCHES "x86_64" OR
  CMAKE_OSX_ARCHITECTURES MATCHES "arm64" OR
  CMAKE_OSX_ARCHITECTURES MATCHES "i386")
  set(CPU_TYPE ${CMAKE_OSX_ARCHITECTURES})
endif()
if(CMAKE_OSX_ARCHITECTURES MATCHES "ppc")
  set(CPU_TYPE powerpc)
endif()
if(MSVC_IDE AND CMAKE_GENERATOR_PLATFORM MATCHES "arm64")
  set(CPU_TYPE arm64)
endif()

message(STATUS "${BITS}-bit build (${CPU_TYPE})")


###############################################################################
# INSTALL DIRECTORIES
###############################################################################

# FETCHCONTENT: Skip installation setup when used as subproject
if(NOT LIBJPEG_TURBO_IS_SUBPROJECT)
  if(WIN32)
    if(MSVC_LIKE)
      set(CMAKE_INSTALL_DEFAULT_PREFIX "c:/${CMAKE_PROJECT_NAME}")
    else()
      set(CMAKE_INSTALL_DEFAULT_PREFIX "c:/${CMAKE_PROJECT_NAME}-gcc")
    endif()
    if(BITS EQUAL 64)
      set(CMAKE_INSTALL_DEFAULT_PREFIX "${CMAKE_INSTALL_DEFAULT_PREFIX}64")
    endif()
  else()
    if(NOT CMAKE_INSTALL_DEFAULT_PREFIX)
      set(CMAKE_INSTALL_DEFAULT_PREFIX /opt/${CMAKE_PROJECT_NAME})
    endif()
  endif()
  if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_DEFAULT_PREFIX}" CACHE PATH
      "Directory into which to install ${CMAKE_PROJECT_NAME} (default: ${CMAKE_INSTALL_DEFAULT_PREFIX})"
      FORCE)
  endif()
  message(STATUS "CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}")

  if(CMAKE_INSTALL_PREFIX STREQUAL "${CMAKE_INSTALL_DEFAULT_PREFIX}")
    set(CMAKE_INSTALL_DEFAULT_DATAROOTDIR "")
    set(CMAKE_INSTALL_DEFAULT_DOCDIR "<CMAKE_INSTALL_DATAROOTDIR>/doc")
    set(CMAKE_INSTALL_DEFAULT_JAVADIR "<CMAKE_INSTALL_DATAROOTDIR>/classes")
    if(UNIX AND NOT APPLE)
      if(BITS EQUAL 64)
        set(CMAKE_INSTALL_DEFAULT_LIBDIR "lib64")
      elseif(CMAKE_C_COMPILER_ABI MATCHES "ELF X32")
        set(CMAKE_INSTALL_DEFAULT_LIBDIR "libx32")
      else()
        set(CMAKE_INSTALL_DEFAULT_LIBDIR "lib32")
      endif()
    endif()
  endif()

  include(cmakescripts/GNUInstallDirs.cmake)

  macro(report_directory var)
    if(CMAKE_INSTALL_${var} STREQUAL CMAKE_INSTALL_FULL_${var})
      message(STATUS "CMAKE_INSTALL_${var} = ${CMAKE_INSTALL_${var}}")
    else()
      message(STATUS "CMAKE_INSTALL_${var} = ${CMAKE_INSTALL_${var}} (${CMAKE_INSTALL_FULL_${var}})")
    endif()
    mark_as_advanced(CLEAR CMAKE_INSTALL_${var})
  endmacro()

  set(DIRLIST "BINDIR;DATAROOTDIR;DOCDIR;INCLUDEDIR;LIBDIR")
  if(UNIX)
    list(APPEND DIRLIST "MANDIR")
  endif()
  foreach(dir ${DIRLIST})
    report_directory(${dir})
  endforeach()
endif()


###############################################################################
# CONFIGURATION OPTIONS
###############################################################################

macro(boolean_number var)
  if(${var})
    set(${var} 1 ${ARGN})
  else()
    set(${var} 0 ${ARGN})
  endif()
endmacro()

option(ENABLE_SHARED "Build shared libraries" TRUE)
boolean_number(ENABLE_SHARED)
option(ENABLE_STATIC "Build static libraries" TRUE)
boolean_number(ENABLE_STATIC)

# FETCHCONTENT: Default to disabled SIMD and tests when subproject
if(LIBJPEG_TURBO_IS_SUBPROJECT)
  option(REQUIRE_SIMD "Generate a fatal error if SIMD extensions are not available" FALSE)
  option(WITH_SIMD "Include SIMD extensions, if available for this platform" FALSE)
  option(WITH_TOOLS "Build command-line tools" FALSE)
  option(WITH_TESTS "Enable regression tests" FALSE)
else()
  option(REQUIRE_SIMD "Generate a fatal error if SIMD extensions are not available" FALSE)
  option(WITH_SIMD "Include SIMD extensions, if available for this platform" TRUE)
  option(WITH_TOOLS "Build command-line tools" TRUE)
  option(WITH_TESTS "Enable regression tests" TRUE)
endif()

boolean_number(REQUIRE_SIMD)
option(WITH_ARITH_DEC "Include arithmetic decoding support when emulating the libjpeg v6b API/ABI" TRUE)
boolean_number(WITH_ARITH_DEC)
option(WITH_ARITH_ENC "Include arithmetic encoding support when emulating the libjpeg v6b API/ABI" TRUE)
boolean_number(WITH_ARITH_ENC)
if(CMAKE_C_COMPILER_ABI MATCHES "ELF X32")
  set(WITH_JAVA 0)
else()
  option(WITH_JAVA "Build Java wrapper for the TurboJPEG API library (implies ENABLE_SHARED=1)" FALSE)
  boolean_number(WITH_JAVA)
endif()
option(WITH_JPEG7 "Emulate libjpeg v7 API/ABI" FALSE)
boolean_number(WITH_JPEG7)
option(WITH_JPEG8 "Emulate libjpeg v8 API/ABI" FALSE)
boolean_number(WITH_JPEG8)
boolean_number(WITH_SIMD)
option(WITH_TURBOJPEG "Include the TurboJPEG API library and associated command-line tools/test programs" TRUE)
boolean_number(WITH_TURBOJPEG)
boolean_number(WITH_TOOLS)
if(NOT WITH_TOOLS)
  set(WITH_TESTS 0)
endif()
boolean_number(WITH_TESTS)
option(WITH_FUZZ "Build fuzz targets" FALSE)

macro(report_option var desc)
  if(${var})
    message(STATUS "${desc} enabled (${var} = ${${var}})")
  else()
    message(STATUS "${desc} disabled (${var} = ${${var}})")
  endif()
endmacro()

if(WITH_JAVA)
  set(ENABLE_SHARED 1)
endif()

if(DEFINED CMAKE_POSITION_INDEPENDENT_CODE AND
  NOT CMAKE_POSITION_INDEPENDENT_CODE AND ENABLE_SHARED)
  unset(CMAKE_POSITION_INDEPENDENT_CODE CACHE)
endif()

report_option(ENABLE_SHARED "Shared libraries")
report_option(ENABLE_STATIC "Static libraries")

if(ENABLE_SHARED)
  set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_FULL_LIBDIR})
endif()

if(WITH_JPEG8 OR WITH_JPEG7)
  set(WITH_ARITH_ENC 1)
  set(WITH_ARITH_DEC 1)
endif()

if(WITH_ARITH_DEC)
  set(D_ARITH_CODING_SUPPORTED 1)
endif()
report_option(WITH_ARITH_DEC "Arithmetic decoding support")

if(WITH_ARITH_ENC)
  set(C_ARITH_CODING_SUPPORTED 1)
endif()
report_option(WITH_ARITH_ENC "Arithmetic encoding support")

report_option(WITH_TURBOJPEG "TurboJPEG API library")
report_option(WITH_JAVA "TurboJPEG Java wrapper")
report_option(WITH_TOOLS "Command-line tools")
report_option(WITH_TESTS "Regression tests")

if(NOT WITH_JPEG8)
  set(MEM_SRCDST_FUNCTIONS "global:  jpeg_mem_dest;  jpeg_mem_src;")
endif()

set(SO_AGE 3)
if(NOT WITH_JPEG8)
  set(SO_AGE 4)
endif()

if(WITH_JPEG8)
  set(JPEG_LIB_VERSION 80)
elseif(WITH_JPEG7)
  set(JPEG_LIB_VERSION 70)
else()
  set(JPEG_LIB_VERSION 62)
endif()

math(EXPR JPEG_LIB_VERSION_DIV10 "${JPEG_LIB_VERSION} / 10")
math(EXPR JPEG_LIB_VERSION_MOD10 "${JPEG_LIB_VERSION} % 10")
if(JPEG_LIB_VERSION STREQUAL "62")
  set(DEFAULT_SO_MAJOR_VERSION ${JPEG_LIB_VERSION})
else()
  set(DEFAULT_SO_MAJOR_VERSION ${JPEG_LIB_VERSION_DIV10})
endif()
if(JPEG_LIB_VERSION STREQUAL "80")
  set(DEFAULT_SO_MINOR_VERSION 2)
else()
  set(DEFAULT_SO_MINOR_VERSION 0)
endif()

if((DEFINED WITH_JPEG7_INT AND NOT WITH_JPEG7 EQUAL WITH_JPEG7_INT) OR
  (DEFINED WITH_JPEG8_INT AND NOT WITH_JPEG8 EQUAL WITH_JPEG8_INT))
  set(FORCE_SO_VERSION "FORCE")
endif()
set(WITH_JPEG7_INT ${WITH_JPEG7} CACHE INTERNAL "")
set(WITH_JPEG8_INT ${WITH_JPEG8} CACHE INTERNAL "")

set(SO_MAJOR_VERSION ${DEFAULT_SO_MAJOR_VERSION} CACHE STRING
  "Major version of the libjpeg API shared library (default: ${DEFAULT_SO_MAJOR_VERSION})"
  ${FORCE_SO_VERSION})
set(SO_MINOR_VERSION ${DEFAULT_SO_MINOR_VERSION} CACHE STRING
  "Minor version of the libjpeg API shared library (default: ${DEFAULT_SO_MINOR_VERSION})"
  ${FORCE_SO_VERSION})

set(JPEG_LIB_VERSION_DECIMAL "${JPEG_LIB_VERSION_DIV10}.${JPEG_LIB_VERSION_MOD10}")
message(STATUS "Emulating libjpeg API/ABI v${JPEG_LIB_VERSION_DECIMAL} (WITH_JPEG7 = ${WITH_JPEG7}, WITH_JPEG8 = ${WITH_JPEG8})")
message(STATUS "libjpeg API shared library version = ${SO_MAJOR_VERSION}.${SO_AGE}.${SO_MINOR_VERSION}")

set(TURBOJPEG_SO_MAJOR_VERSION 0)
set(TURBOJPEG_SO_AGE 4)
set(TURBOJPEG_SO_VERSION 0.${TURBOJPEG_SO_AGE}.0)


###############################################################################
# COMPILER SETTINGS
###############################################################################

string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_UC)

if(MSVC_LIKE)
  option(WITH_CRT_DLL "Link all ${CMAKE_PROJECT_NAME} libraries and executables with the C run-time DLL" FALSE)
  boolean_number(WITH_CRT_DLL)
  if(NOT WITH_CRT_DLL)
    if(CMAKE_VERSION VERSION_EQUAL "3.15" OR CMAKE_VERSION VERSION_GREATER "3.15")
      if(CMAKE_BUILD_TYPE_UC STREQUAL "DEBUG")
        set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDebug)
      elseif(MSVC_IDE)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
      else()
        set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded)
      endif()
      message(STATUS "Visual C++ run-time library: ${CMAKE_MSVC_RUNTIME_LIBRARY} (WITH_CRT_DLL = ${WITH_CRT_DLL})")
    elseif(MSVC)
      foreach(var CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
        if(${var} MATCHES "/MD")
          string(REGEX REPLACE "/MD" "/MT" ${var} "${${var}}")
        endif()
      endforeach()
    endif()
  endif()
  add_definitions(-D_CRT_NONSTDC_NO_WARNINGS)
endif()

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
  foreach(build_type RELEASE RELWITHDEBINFO)
    if(CMAKE_C_FLAGS_${build_type} MATCHES "-O2" AND
      CMAKE_C_FLAGS_${build_type}_INITIALIZED_TO_DEFAULT)
      string(REGEX REPLACE "-O2" "-O3" CMAKE_C_FLAGS_${build_type}
        "${CMAKE_C_FLAGS_${build_type}}")
    endif()
  endforeach()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
  if(CMAKE_C_COMPILER_ID MATCHES "SunPro")
    foreach(build_type RELEASE RELWITHDEBINFO)
      if(CMAKE_C_FLAGS_${build_type} MATCHES "-xO3" AND
        CMAKE_C_FLAGS_${build_type}_INITIALIZED_TO_DEFAULT)
        string(REGEX REPLACE "-xO3" "-xO5" CMAKE_C_FLAGS_${build_type}
          "${CMAKE_C_FLAGS_${build_type}}")
      endif()
      if(CMAKE_C_FLAGS_${build_type} MATCHES "-xO2" AND
        CMAKE_C_FLAGS_${build_type}_INITIALIZED_TO_DEFAULT)
        string(REGEX REPLACE "-xO2" "-xO5" CMAKE_C_FLAGS_${build_type}
          "${CMAKE_C_FLAGS_${build_type}}")
      endif()
    endforeach()
  endif()
endif()

set(EFFECTIVE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE_UC}}")
message(STATUS "Compiler flags = ${EFFECTIVE_C_FLAGS}")

set(EFFECTIVE_LD_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS_${CMAKE_BUILD_TYPE_UC}}")
message(STATUS "Linker flags = ${EFFECTIVE_LD_FLAGS}")

include(CheckCSourceCompiles)
include(CheckIncludeFiles)
include(CheckTypeSize)

check_type_size("size_t" SIZE_T)
check_type_size("unsigned long" UNSIGNED_LONG)

if(SIZE_T EQUAL UNSIGNED_LONG)
  check_c_source_compiles("int main(int argc, char **argv) { unsigned long a = argc;  return __builtin_ctzl(a); }"
    HAVE_BUILTIN_CTZL)
endif()
if(MSVC_LIKE)
  check_include_files("intrin.h" HAVE_INTRIN_H)
endif()

if(UNIX)
  if(CMAKE_CROSSCOMPILING)
    set(RIGHT_SHIFT_IS_UNSIGNED 0)
  else()
    include(CheckCSourceRuns)
    check_c_source_runs("
      #include <stdio.h>
      #include <stdlib.h>
      static int is_shifting_signed (long arg) {
        long res = arg >> 4;
        if (res == -0x7F7E80CL)
          return 1;
        res |= 0xFFFFFFFFL << (32-4);
        if (res == -0x7F7E80CL)
          return 0;
        printf(\"Right shift isn't acting as I expect it to.\\\\n\");
        printf(\"I fear the JPEG software will not work at all.\\\\n\\\\n\");
        return 0;
      }
      int main (void) {
        exit(is_shifting_signed(-0x7F7E80B1L));
      }" RIGHT_SHIFT_IS_UNSIGNED)
  endif()
endif()

if(NOT WIN32)
  check_c_source_compiles("extern const int table[1]; const int __attribute__((visibility(\"hidden\"))) table[1] = { 0 }; int main(void) { return table[0]; }"
    HIDDEN_WORKS)
  if(HIDDEN_WORKS)
    set(HIDDEN "__attribute__((visibility(\"hidden\")))")
    message(STATUS "HIDDEN = ${HIDDEN}")
  endif()
endif()

if(MSVC)
  set(INLINE_OPTIONS "__inline;inline")
else()
  set(INLINE_OPTIONS "__inline__;inline")
endif()
option(FORCE_INLINE "Force function inlining" TRUE)
boolean_number(FORCE_INLINE)
if(FORCE_INLINE)
  if(MSVC)
    list(INSERT INLINE_OPTIONS 0 "__forceinline")
  else()
    list(INSERT INLINE_OPTIONS 0 "inline __attribute__((always_inline))")
    list(INSERT INLINE_OPTIONS 0 "__inline__ __attribute__((always_inline))")
  endif()
endif()
foreach(inline ${INLINE_OPTIONS})
  check_c_source_compiles("${inline} static int foo(void) { return 0; } int main(void) { return foo(); }"
    INLINE_WORKS)
  if(INLINE_WORKS)
    set(INLINE ${inline})
    break()
  endif()
endforeach()
if(NOT INLINE_WORKS)
  message(FATAL_ERROR "Could not determine how to inline functions.")
endif()
message(STATUS "INLINE = ${INLINE} (FORCE_INLINE = ${FORCE_INLINE})")

if(MSVC)
  set(THREAD_LOCAL "__declspec(thread)")
else()
  set(THREAD_LOCAL "__thread")
endif()
check_c_source_compiles("static ${THREAD_LOCAL} int i;  int main(void) { i = 0;  return i; }"
  HAVE_THREAD_LOCAL)
if(HAVE_THREAD_LOCAL)
  message(STATUS "THREAD_LOCAL = ${THREAD_LOCAL}")
else()
  message(WARNING "Thread-local storage is not available.  The TurboJPEG API library's global error handler will not be thread-safe.")
  unset(THREAD_LOCAL)
endif()

if(UNIX AND NOT APPLE)
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/conftest.map "VERS_1 { global: *; };")
  set(CMAKE_REQUIRED_FLAGS
    "-Wl,--version-script,${CMAKE_CURRENT_BINARY_DIR}/conftest.map")
  check_c_source_compiles("int main(void) { return 0; }" HAVE_VERSION_SCRIPT)
  set(CMAKE_REQUIRED_FLAGS)
  file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/conftest.map)
  if(HAVE_VERSION_SCRIPT)
    message(STATUS "Linker supports GNU-style version scripts")
    set(MAPFLAG "-Wl,--version-script,")
    set(TJMAPFLAG "-Wl,--version-script,")
  else()
    message(STATUS "Linker does not support GNU-style version scripts")
    if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
      file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/conftest.map
        "VERS_1 { global: foo;  local: *; }; VERS_2 { global: foo2; } VERS_1;")
      set(CMAKE_REQUIRED_FLAGS "-Wl,-M,${CMAKE_CURRENT_BINARY_DIR}/conftest.map -shared")
      check_c_source_compiles("int foo() { return 0; } int foo2() { return 2; }"
        HAVE_MAPFILE)
      set(CMAKE_REQUIRED_FLAGS)
      file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/conftest.map)
      if(HAVE_MAPFILE)
        message(STATUS "Linker supports mapfiles")
        set(TJMAPFLAG "-Wl,-M,")
      else()
        message(STATUS "Linker does not support mapfiles")
      endif()
    endif()
  endif()
endif()

# Generate files
configure_file(src/jversion.h.in jversion.h)
if(UNIX)
  configure_file(src/libjpeg.map.in libjpeg.map)
endif()

# Include directories and compiler definitions
include_directories(${CMAKE_CURRENT_BINARY_DIR})


###############################################################################
# TARGETS
###############################################################################

if(CMAKE_EXECUTABLE_SUFFIX_TMP)
  set(CMAKE_EXECUTABLE_SUFFIX ${CMAKE_EXECUTABLE_SUFFIX_TMP})
endif()
message(STATUS "CMAKE_EXECUTABLE_SUFFIX = ${CMAKE_EXECUTABLE_SUFFIX}")

add_custom_target(wrappers COMMAND ${CMAKE_COMMAND}
 -DSOURCE_DIR=${CMAKE_SOURCE_DIR}/src/wrapper
 -P ${CMAKE_CURRENT_SOURCE_DIR}/cmakescripts/GenerateWrappers.cmake)

set(JPEG_SOURCES src/jcapimin.c
  src/wrapper/jcapistd-8.c src/wrapper/jcapistd-12.c src/wrapper/jcapistd-16.c
  src/wrapper/jccoefct-8.c src/wrapper/jccoefct-12.c
  src/wrapper/jccolor-8.c src/wrapper/jccolor-12.c src/wrapper/jccolor-16.c
  src/wrapper/jcdctmgr-8.c src/wrapper/jcdctmgr-12.c
  src/wrapper/jcdiffct-8.c src/wrapper/jcdiffct-12.c src/wrapper/jcdiffct-16.c
  src/jchuff.c src/jcicc.c src/jcinit.c src/jclhuff.c
  src/wrapper/jclossls-8.c src/wrapper/jclossls-12.c src/wrapper/jclossls-16.c
  src/wrapper/jcmainct-8.c src/wrapper/jcmainct-12.c src/wrapper/jcmainct-16.c
  src/jcmarker.c src/jcmaster.c src/jcomapi.c src/jcparam.c src/jcphuff.c
  src/wrapper/jcprepct-8.c src/wrapper/jcprepct-12.c src/wrapper/jcprepct-16.c
  src/wrapper/jcsample-8.c src/wrapper/jcsample-12.c src/wrapper/jcsample-16.c
  src/jctrans.c src/jdapimin.c
  src/wrapper/jdapistd-8.c src/wrapper/jdapistd-12.c src/wrapper/jdapistd-16.c
  src/jdatadst.c src/jdatasrc.c
  src/wrapper/jdcoefct-8.c src/wrapper/jdcoefct-12.c
  src/wrapper/jdcolor-8.c src/wrapper/jdcolor-12.c src/wrapper/jdcolor-16.c
  src/wrapper/jddctmgr-8.c src/wrapper/jddctmgr-12.c
  src/wrapper/jddiffct-8.c src/wrapper/jddiffct-12.c src/wrapper/jddiffct-16.c
  src/jdhuff.c src/jdicc.c src/jdinput.c src/jdlhuff.c
  src/wrapper/jdlossls-8.c src/wrapper/jdlossls-12.c src/wrapper/jdlossls-16.c
  src/wrapper/jdmainct-8.c src/wrapper/jdmainct-12.c src/wrapper/jdmainct-16.c
  src/jdmarker.c src/jdmaster.c
  src/wrapper/jdmerge-8.c src/wrapper/jdmerge-12.c
  src/jdphuff.c
  src/wrapper/jdpostct-8.c src/wrapper/jdpostct-12.c src/wrapper/jdpostct-16.c
  src/wrapper/jdsample-8.c src/wrapper/jdsample-12.c src/wrapper/jdsample-16.c
  src/jdtrans.c src/jerror.c src/jfdctflt.c
  src/wrapper/jfdctfst-8.c src/wrapper/jfdctfst-12.c
  src/wrapper/jfdctint-8.c src/wrapper/jfdctint-12.c
  src/wrapper/jidctflt-8.c src/wrapper/jidctflt-12.c
  src/wrapper/jidctfst-8.c src/wrapper/jidctfst-12.c
  src/wrapper/jidctint-8.c src/wrapper/jidctint-12.c
  src/wrapper/jidctred-8.c src/wrapper/jidctred-12.c
  src/jm