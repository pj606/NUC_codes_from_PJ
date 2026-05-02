from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, ExecuteProcess
from launch.substitutions import LaunchConfiguration


def generate_launch_description():
    bag_path_arg = DeclareLaunchArgument(
        'bag_path',
        description='Path to rosbag2 directory',
    )

    play_process = ExecuteProcess(
        cmd=['ros2', 'bag', 'play', LaunchConfiguration('bag_path')],
        output='screen',
    )

    return LaunchDescription([
        bag_path_arg,
        play_process,
    ])
