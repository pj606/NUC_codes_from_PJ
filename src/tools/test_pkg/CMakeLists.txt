cmake_minimum_required(VERSION 3.22)
project(test_pkg)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(class_loader REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)

# 关键：调用你的相机包 + OpenCV
find_package(OpenCV REQUIRED)
find_package(hik_camera REQUIRED)

add_executable(test_camera_noNode src/test_camera_noNode.cpp)
target_link_libraries(test_camera_noNode ${OpenCV_LIBS})
ament_target_dependencies(test_camera_noNode
  rclcpp
  std_msgs
  sensor_msgs
  hik_camera
)

add_library(camera_image_view_component SHARED src/camera_image_view_node.cpp)
target_link_libraries(camera_image_view_component ${OpenCV_LIBS})
ament_target_dependencies(camera_image_view_component
  rclcpp
  rclcpp_components
  class_loader
  sensor_msgs
)

rclcpp_components_register_nodes(camera_image_view_component "test_pkg::CameraImageViewNode")

install(TARGETS
  test_camera_noNode
  DESTINATION lib/${PROJECT_NAME}
)

install(TARGETS
  camera_image_view_component
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib
)

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()