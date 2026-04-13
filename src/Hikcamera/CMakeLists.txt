cmake_minimum_required(VERSION 3.8)
project(hik_camera_driver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(OpenCV REQUIRED)

set(MVS_ROOT "/opt/MVS")

find_path(MVS_INCLUDE_DIR
  NAMES MvCameraControl.h
  PATHS
    ${MVS_ROOT}/include
  REQUIRED
)

find_library(MVS_CORE_LIB
  NAMES MvCameraControl
  PATHS
    ${MVS_ROOT}/lib
    ${MVS_ROOT}/lib/64
  REQUIRED
)

message(STATUS "MVS include dir: ${MVS_INCLUDE_DIR}")
message(STATUS "MVS core lib   : ${MVS_CORE_LIB}")

add_library(${PROJECT_NAME}_core
  src/hik_camera.cpp
)

target_include_directories(${PROJECT_NAME}_core
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${MVS_INCLUDE_DIR}
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}_core
  ${MVS_CORE_LIB}
  ${OpenCV_LIBS}
)

add_executable(${PROJECT_NAME}_node
  src/hik_camera_node.cpp
)

target_link_libraries(${PROJECT_NAME}_node
  ${PROJECT_NAME}_core
)

ament_target_dependencies(${PROJECT_NAME}_node
  rclcpp
  sensor_msgs
  std_msgs
  cv_bridge
)

set_target_properties(${PROJECT_NAME}_core PROPERTIES
  BUILD_RPATH "/opt/MVS/lib;/opt/MVS/lib/64"
  INSTALL_RPATH "/opt/MVS/lib;/opt/MVS/lib/64"
)
set_target_properties(${PROJECT_NAME}_node PROPERTIES
  BUILD_RPATH "/opt/MVS/lib;/opt/MVS/lib/64"
  INSTALL_RPATH "/opt/MVS/lib;/opt/MVS/lib/64"
)

install(
  TARGETS
    ${PROJECT_NAME}_core
    ${PROJECT_NAME}_node
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

install(
  DIRECTORY include/
  DESTINATION include
)

ament_export_targets(export_${PROJECT_NAME} HAS_LIBRARY_TARGET)
ament_export_include_directories(include)
ament_package()