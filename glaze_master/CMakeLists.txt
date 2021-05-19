SET (CMAKE_CXX_FLAGS "-Wall -Wpedantic -Wextra -g")

cmake_minimum_required(VERSION 3.0.0)
project(glaze_master VERSION 0.1.0)

find_package(Boost COMPONENTS system REQUIRED)
find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 17)

file(GLOB server_SRC
  "*.cpp"
)

add_executable(glaze_master
  ${server_SRC}
)

target_link_libraries(glaze_master
  ${Boost_SYSTEM_LIBRARY}
  Threads::Threads
)
