cmake_minimum_required(VERSION 3.21)
project(weather_station)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(weather_station
        weather_station.ino)
