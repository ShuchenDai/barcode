project(usb_prn_barcode)
cmake_minimum_required(VERSION 2.8)

set(INSTALL_DEFAULT_BINDIR "bin" CACHE STRING "Appended to CMAKE_INSTALL_PREFIX")

# Comment the following out for releases.
set(CMAKE_C_FLAGS "$ENV{CFLAGS} -pthread -Wall")

set(CMAKE_CPP_FLAGS "${CMAKE_CPP_FLAGS} $ENV{CPPFLAGS} -std=gnu++11 -pthread -pedantic -Wall -felide-constructors -fno-exceptions")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)


add_subdirectory(src)