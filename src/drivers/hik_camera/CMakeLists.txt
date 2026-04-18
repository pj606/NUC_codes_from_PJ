cmake_minimum_required(VERSION 3.22)
project(hik_camera)

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
  PATHS ${MVS_ROOT}/include
  REQUIRED
)

find_library(MVS_CORE_LIB
  NAMES MvCameraControl
  PATHS ${MVS_ROOT}/lib ${MVS_ROOT}/lib/64
  REQUIRED
)

add_library(${PROJECT_NAME}_core SHARED
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

set_target_properties(${PROJECT_NAME}_core PROPERTIES
  BUILD_RPATH "/opt/MVS/lib;/opt/MVS/lib/64"
  INSTALL_RPATH "/opt/MVS/lib;/opt/MVS/lib/64"
)

install(
  TARGETS ${PROJECT_NAME}_core
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/ DESTINATION include)

ament_export_targets(export_${PROJECT_NAME} HAS_LIBRARY_TARGET)
ament_export_include_directories(include)
ament_export_dependencies(OpenCV rclcpp sensor_msgs std_msgs cv_bridge)

ament_package()