from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, ExecuteProcess, LogInfo
from launch.conditions import IfCondition
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    diagnostics_config_arg = DeclareLaunchArgument(
        'diagnostics_config',
        default_value=PathJoinSubstitution([
            FindPackageShare('radar_bringup'),
            'config',
            'diagnostic_aggregator.yaml',
        ]),
        description='Diagnostics aggregator configuration file',
    )

    enable_recording_arg = DeclareLaunchArgument(
        'enable_recording',
        default_value='false',
        description='Enable rosbag2 recording',
    )

    bag_output_arg = DeclareLaunchArgument(
        'bag_output',
        default_value='radar_bag',
        description='Output directory name for rosbag2',
    )

    diagnostics_node = Node(
        package='diagnostic_aggregator',
        executable='aggregator_node',
        name='radar_diagnostics',
        output='screen',
        parameters=[LaunchConfiguration('diagnostics_config')],
    )

    record_process = ExecuteProcess(
        cmd=[
            'ros2',
            'bag',
            'record',
            '-a',
            '-o',
            LaunchConfiguration('bag_output'),
        ],
        condition=IfCondition(LaunchConfiguration('enable_recording')),
        output='screen',
    )

    perf_hint = LogInfo(
        msg='Use ros2 topic hz/bw for runtime performance checks.',
    )

    return LaunchDescription([
        diagnostics_config_arg,
        enable_recording_arg,
        bag_output_arg,
        diagnostics_node,
        record_process,
        perf_hint,
    ])
