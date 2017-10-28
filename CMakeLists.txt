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

add_executable(warehouse_ui src/warehouse_ui.cpp)
target_link_libraries(warehouse_ui Threads::Threads)

set(SERVER_HEADERS src/Order.h src/CircularOrderQueue.h src/robot.h src/server.h)
add_executable(server src/server.cpp ${SERVER_HEADERS})
target_link_libraries(server Threads::Threads)

set(CLIENT_HEADERS src/Order.h src/comm.h)
add_executable(customer src/customer.cpp ${CUSTOMER_HEADERS})
target_link_libraries(customer Threads::Threads)

add_executable(manager src/manager.cpp ${CUSTOMER_HEADERS})
target_link_libraries(manager Threads::Threads)

# link with unix realtime library
if(UNIX AND NOT APPLE)
    target_link_libraries(server rt)
    target_link_libraries(client rt)
endif()

if(WIN32)
    target_link_libraries(customer wsock32 ws2_32)
    target_link_libraries(manager wsock32 ws2_32)
    target_link_libraries(server wsock32 ws2_32)
endif()