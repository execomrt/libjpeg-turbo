cmake_minimum_required(VERSION 3.10)
project(libjpeg-turbo C)

set(VERSION 3.1.2)

# Detect if used as subproject
if(NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(LIBJPEG_TURBO_IS_SUBPROJECT TRUE)
  message(STATUS "libjpeg-turbo: Building as subproject")
else()
  set(LIBJPEG_TURBO_IS_SUBPROJECT FALSE)
endif()

# Options
option(ENABLE_STATIC "Build static libraries" ON)
option(WITH_ARITH_ENC "Include arithmetic encoding" ON)
option(WITH_ARITH_DEC "Include arithmetic decoding" ON)

# Configuration
include(CheckIncludeFiles)
include(CheckTypeSize)

check_type_size("size_t" SIZE_T)
if(MSVC)
  check_include_files("intrin.h" HAVE_INTRIN_H)
endif()

# Set variables for config files
set(JPEG_LIB_VERSION 62)
set(BITS 64)
if(WITH_ARITH_DEC)
  set(D_ARITH_CODING_SUPPORTED 1)
endif()
if(WITH_ARITH_ENC)
  set(C_ARITH_CODING_SUPPORTED 1)
endif()
set(MEM_SRCDST_SUPPORTED 1)
set(INLINE "__inline")
if(MSVC)
  set(INLINE "__inline")
  set(THREAD_LOCAL "__declspec(thread)")
else()
  set(INLINE "__inline__")
  set(THREAD_LOCAL "__thread")
endif()

# Generate config headers
configure_file(src/jconfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/jconfig.h)
configure_file(src/jconfigint.h.in ${CMAKE_CURRENT_BINARY_DIR}/jconfigint.h)
configure_file(src/jversion.h.in ${CMAKE_CURRENT_BINARY_DIR}/jversion.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Source files
set(JPEG_SOURCES 
  src/jcapimin.c
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
  src/jmemmgr.c src/jmemnobs.c src/jpeg_nbits.c
  src/wrapper/jquant1-8.c src/wrapper/jquant1-12.c
  src/wrapper/jquant2-8.c src/wrapper/jquant2-12.c
  src/wrapper/jutils-8.c src/wrapper/jutils-12.c src/wrapper/jutils-16.c
)

# Add arithmetic coding sources if enabled
if(WITH_ARITH_ENC OR WITH_ARITH_DEC)
  list(APPEND JPEG_SOURCES src/jaricom.c)
endif()
if(WITH_ARITH_ENC)
  list(APPEND JPEG_SOURCES src/jcarith.c)
endif()
if(WITH_ARITH_DEC)
  list(APPEND JPEG_SOURCES src/jdarith.c)
endif()

# Build static library
if(ENABLE_STATIC)
  add_library(jpeg-static STATIC ${JPEG_SOURCES})
  
  # Set output name
  if(NOT MSVC)
    set_target_properties(jpeg-static PROPERTIES OUTPUT_NAME jpeg)
  endif()
  
  # Add alias for FetchContent compatibility
  if(LIBJPEG_TURBO_IS_SUBPROJECT)
    add_library(libjpeg-turbo::jpeg ALIAS jpeg-static)
  endif()
  
  # Export headers
  target_include_directories(jpeg-static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:include>
  )
endif()

# Installation (skip when used as subproject)
if(NOT LIBJPEG_TURBO_IS_SUBPROJECT)
  include(GNUInstallDirs)
  
  install(TARGETS jpeg-static
    EXPORT libjpeg-turbo-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )
  
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/jconfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/jerror.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/jmorecfg.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/jpeglib.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )
  
  install(EXPORT libjpeg-turbo-targets
    FILE libjpeg-turbo-targets.cmake
    NAMESPACE libjpeg-turbo::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libjpeg-turbo
  )
endif()