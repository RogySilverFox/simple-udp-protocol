cmake_minimum_required(VERSION 3.27)
project(SimpleUdpProtocol)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_package(Qt6 COMPONENTS
        Core
        Network
        REQUIRED)

add_executable(SimpleUdpProtocol main.cpp
        SimpleProtocol.cpp
        SimpleProtocol.h
        WorkerThread.cpp
        WorkerThread.h)
target_link_libraries(SimpleUdpProtocol
        Qt::Core
        Qt6::Network
)

