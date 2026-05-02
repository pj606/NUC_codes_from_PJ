from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, LogInfo
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    params_file_arg = DeclareLaunchArgument(
        'params_file',
        default_value=PathJoinSubstitution([
            FindPackageShare('radar_bringup'),
            'config',
            'radar_system.yaml',
        ]),
        description='YAML file for radar system parameters',
    )

    base_frame_arg = DeclareLaunchArgument(
        'base_frame',
        default_value='base_link',
        description='Robot base frame',
    )

    radar_frame_arg = DeclareLaunchArgument(
        'radar_frame',
        default_value='radar_link',
        description='Radar reference frame',
    )

    sensors_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            PathJoinSubstitution([
                FindPackageShare('radar_bringup'),
                'launch',
                'sensors.launch.py',
            ])
        ),
        launch_arguments={
            'params_file': LaunchConfiguration('params_file'),
        }.items(),
    )

    static_tf = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='radar_static_tf',
        arguments=['0', '0', '0', '0', '0', '0',
                   LaunchConfiguration('base_frame'),
                   LaunchConfiguration('radar_frame')],
    )

    perception_placeholder = LogInfo(
        msg='Perception nodes (img_detector/ray_tracing) are placeholders in this launch.',
    )

    decision_placeholder = LogInfo(
        msg='Decision node (double_vulner) is a placeholder in this launch.',
    )

    return LaunchDescription([
        params_file_arg,
        base_frame_arg,
        radar_frame_arg,
        static_tf,
        sensors_launch,
        perception_placeholder,
        decision_placeholder,
    ])
