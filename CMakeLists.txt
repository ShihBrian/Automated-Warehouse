cmake_minimum_required(VERSION 3.2)
project(lab4)

set(CMAKE_CXX_STANDARD 11)
find_package( Threads REQUIRED )  # Multithreading package required

# add library directory
include_directories(../library/include)
include_directories(include)

# copy data files
FILE(COPY data DESTINATION "${CMAKE_BINARY_DIR}")
FILE(COPY data DESTINATION "${CMAKE_BINARY_DIR}/${CONFIGURATION}")

#===================================================
#  Maze Runner
#===================================================

# child
add_executable(warehouse_ui src/warehouse_ui.cpp)
target_link_libraries(warehouse_ui Threads::Threads)

set(SERVER_HEADERS src/Order.h src/CircularOrderQueue.h
        src/client.h src/OrderQueue.h src/robot.h)

#=================================================================
#  Restauranat
#=================================================================
add_executable(server src/server.cpp ${SERVER_HEADERS})
target_link_libraries(server Threads::Threads)