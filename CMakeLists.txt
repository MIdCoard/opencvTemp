cmake_minimum_required(VERSION 3.17)
project(opencv)

set(CMAKE_CXX_STANDARD 14)
find_package(OpenCV REQUIRED)

# If the package has been found, several variables will
# be set, you can find the full list with descriptions
# in the OpenCVConfig.cmake file.
# Print some message showing some of them
message(STATUS "OpenCV library status:")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")

find_library(FFTW_LIBRARY
        NAMES fftw3 fftw)
set(FFTW_LIBRARIES "${FFTW_LIBRARY}")

find_library(FFTW_libm_LIBRARY
        NAMES m)
list(APPEND FFTW_LIBRARIES "${FFTW_libm_LIBRARY}")

include_directories(
        include/
        ${OpenCV_INCLUDE_DIRS}
        /usr/local/include/eigen3
        /usr/local/include
)
# Add OpenCV headers location to your include paths
include_directories(${OpenCV_INCLUDE_DIRS})

# Declare the executable target built from your sources
add_executable(ab main3.cpp image_dft.cpp image_registration.cpp image_transforms.cpp)
add_executable(oo main5.cpp)
add_executable(main4 main4.cpp image_dft.cpp image_registration.cpp image_transforms.cpp)
add_executable(main1 main.cpp)
add_executable(main6 main6.cpp)

# Link your application with OpenCV libraries
target_link_libraries(ab ${OpenCV_LIBS} ${FFTW_LIBRARIES})
target_link_libraries(main1 ${OpenCV_LIBS} ${FFTW_LIBRARIES})
target_link_libraries(oo ${OpenCV_LIBS} ${FFTW_LIBRARIES})
target_link_libraries(main4 ${OpenCV_LIBS} ${FFTW_LIBRARIES})
target_link_libraries(main6 ${OpenCV_LIBS} ${FFTW_LIBRARIES})