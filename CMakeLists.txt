cmake_minimum_required(VERSION 3.4.3)

set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 11)

# Set the project name
project(t31-rtspd)

# live555
find_package(PkgConfig)
PKG_CHECK_MODULES(LIVEMEDIA REQUIRED live555)
include_directories(${LIVEMEDIA_INCLUDE_DIRS})

#get_cmake_property(_variableNames VARIABLES)
#list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#    message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()

add_compile_options("-DNO_OPENSSL=1")

#find_package(LiveMedia REQUIRED)
#find_package(UsageEnvironment REQUIRED)
#find_package(BasicUsageEnvironment REQUIRED)
#find_package(GroupSock REQUIRED)
#find_package(live555 REQUIRED)

# Tell CMake where to look for header files
include_directories(${CMAKE_SOURCE_DIR}/)
include_directories(${CMAKE_SOURCE_DIR}/include/imp_sys)
# include_directories(${CMAKE_SOURCE_DIR}/include/live555/basicUsageEnvironment)
# include_directories(${CMAKE_SOURCE_DIR}/include/live555/usageEnvironment)
# include_directories(${CMAKE_SOURCE_DIR}/include/live555/groupsock)
# include_directories(${CMAKE_SOURCE_DIR}/include/live555/liveMedia)
# include_directories(live555/basicUsageEnvironment)
# include_directories(live555/usageEnvironment)
# include_directories(live555/groupsock)
# include_directories(live555/liveMedia)

#include_directories( ${LIBLIVEMEDIA_INCLUDE_DIR} )
#include_directories( ${LIBBASICUSAGEENVIRONMENT_INCLUDE_DIR} )
#include_directories( ${LIBUSAGEENVIRONMENT_INCLUDE_DIR} )
#include_directories( ${LIBGROUPSOCK_INCLUDE_DIR} )

# The source files are all the *.c files
file(GLOB T31_RTSPD_SRC_FILES "*.c*")

message(STATUS "Source files for t31-rtspd binary: ${T31_RTSPD_SRC_FILES}")

######################
# Target Executables #
######################

add_executable(t31-rtspd ${T31_RTSPD_SRC_FILES})
set_property(TARGET t31-rtspd PROPERTY C_STANDARD 99)

#########################
# Linking Configuration #
#########################

#set_target_properties( t31-rtspd PROPERTIES LINK_FLAGS "--verbose -Wl,--unresolved-symbols=ignore-in-shared-libs,-z,norelro,-z,lazy")


set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package (Threads)
target_link_libraries (${PROJECT_NAME} Threads::Threads) 


# static link of stdc++ if available
if (STATICSTDCPP)
  include(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG("-static-libgcc" CXX_SUPPORTS_STATIC_LIBGCC)
  if (CXX_SUPPORTS_STATIC_LIBGCC) 
    target_link_libraries (${CMAKE_PROJECT_NAME} -static-libgcc)
  endif()    
  CHECK_CXX_COMPILER_FLAG("-static-libstdc++" CXX_SUPPORTS_STATIC_LIBSTDCPP)
  if (CXX_SUPPORTS_STATIC_LIBSTDCPP) 
    target_link_libraries (${CMAKE_PROJECT_NAME} -static-libstdc++)
  endif()
endif()


find_package( Threads REQUIRED )
target_link_libraries( t31-rtspd ${CMAKE_THREAD_LIBS_INIT} )
target_link_libraries( Threads::Threads )
target_link_libraries( t31-rtspd rt )
target_link_libraries( t31-rtspd m )
target_link_libraries( t31-rtspd pthread )

# The libimp.so library links to some code in C++. However this is a C project
# so we need to include the C++ standard libraries. So find and link it.
find_library (LIBSTDCXX NAMES stdc++)
target_link_libraries(t31-rtspd ${LIBSTDCXX})
message(STATUS "LIBSTDCXX path: ${LIBSTDCXX}")

# ALSA Sound Library
#find_library (LIB_ALSA_SOUND NAMES asound)
#target_link_libraries(t31-rtspd ${LIB_ALSA_SOUND})
#message(STATUS "libalsasound.so path: ${LIB_ALSA_SOUND}")

target_link_libraries(t31-rtspd ${CMAKE_SOURCE_DIR}/lib/imp_sys/uclibc/libimp.so )
target_link_libraries(t31-rtspd ${CMAKE_SOURCE_DIR}/lib/imp_sys/uclibc/libsysutils.so )
target_link_libraries(t31-rtspd ${CMAKE_SOURCE_DIR}/lib/imp_sys/uclibc/libalog.so )

find_library( LIB_BASICUSAGEENVIRONMENT NAMES BasicUsageEnvironment )
target_link_libraries(t31-rtspd ${LIB_BASICUSAGEENVIRONMENT})

find_library( LIB_USAGEENVIRONMENT NAMES UsageEnvironment )
target_link_libraries(t31-rtspd ${LIB_USAGEENVIRONMENT})

find_library( LIB_GROUPSOCK NAMES groupsock )
target_link_libraries(t31-rtspd ${LIB_GROUPSOCK})

find_library( LIB_LIVEMEDIA NAMES liveMedia )
target_link_libraries(t31-rtspd ${LIB_LIVEMEDIA})


#target_link_libraries(t31-rtspd ${CMAKE_SOURCE_DIR}/lib/livelib/libBasicUsageEnvironment.a )
#target_link_libraries(t31-rtspd ${CMAKE_SOURCE_DIR}/lib/livelib/libgroupsock.a )
#target_link_libraries(t31-rtspd ${CMAKE_SOURCE_DIR}/lib/livelib/libliveMedia.a )
#target_link_libraries(t31-rtspd ${CMAKE_SOURCE_DIR}/lib/livelib/libUsageEnvironment.a )

#message(STATUS "CMAKE_THREAD_LIBS_INIT: ${CMAKE_THREAD_LIBS_INIT}")

set_property(TARGET t31-rtspd PROPERTY C_STANDARD 99)

install(TARGETS t31-rtspd DESTINATION bin)


