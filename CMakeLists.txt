
cmake_minimum_required(VERSION 2.8)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x -std=c++11")

PROJECT(hackRF_tx)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(hackrf_tx FMModulator.cpp  HackRFDevice.cpp  input.cpp  main.cpp  WavSource.cpp  write_wav.cpp)


target_link_libraries(hackrf_tx -lm -lstdc++ -lhackrf)

install(TARGETS hackrf_tx DESTINATION /usr/local/bin)
