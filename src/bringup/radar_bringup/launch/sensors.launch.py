from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, LogInfo
from launch.conditions import IfCondition
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    params_file_arg = DeclareLaunchArgument(
        'params_file',
        default_value=PathJoinSubstitution([
            FindPackageShare('radar_bringup'),
            'config',
            'radar_system.yaml',
        ]),
        description='YAML file for radar sensor parameters',
    )

    container_name_arg = DeclareLaunchArgument(
        'container_name',
        default_value='radar_sensor_container',
        description='Component container name for radar sensors',
    )

    enable_lidar_arg = DeclareLaunchArgument(
        'enable_lidar',
        default_value='false',
        description='Enable lidar driver (placeholder)',
    )

    camera_component = ComposableNode(
        package='hik_camera',
        plugin='hik_camera::CameraNode',
        name='hik_camera_node',
        parameters=[LaunchConfiguration('params_file')],
    )

    container = ComposableNodeContainer(
        name=LaunchConfiguration('container_name'),
        namespace='',
        package='rclcpp_components',
        executable='component_container_mt',
        output='screen',
        composable_node_descriptions=[
            camera_component,
        ],
    )

    lidar_placeholder = LogInfo(
        msg='Lidar driver is not launched in this scaffold. Provide a lidar node before enabling ray_tracing.',
        condition=IfCondition(LaunchConfiguration('enable_lidar')),
    )

    return LaunchDescription([
        params_file_arg,
        container_name_arg,
        enable_lidar_arg,
        container,
        lidar_placeholder,
    ])
