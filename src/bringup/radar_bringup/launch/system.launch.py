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
    camera_frame_arg = DeclareLaunchArgument(
        'camera_frame',
        default_value='radar_camera',
        description='Radar camera frame',
    )
    lidar_frame_arg = DeclareLaunchArgument(
        'lidar_frame',
        default_value='radar_lidar',
        description='Radar lidar frame',
    )

    tf_x_arg = DeclareLaunchArgument(
        'tf_x',
        default_value='0',
        description='Radar frame translation x (meters)',
    )
    tf_y_arg = DeclareLaunchArgument(
        'tf_y',
        default_value='0',
        description='Radar frame translation y (meters)',
    )
    tf_z_arg = DeclareLaunchArgument(
        'tf_z',
        default_value='0',
        description='Radar frame translation z (meters)',
    )
    tf_yaw_arg = DeclareLaunchArgument(
        'tf_yaw',
        default_value='0',
        description='Radar frame rotation yaw (radians)',
    )
    tf_pitch_arg = DeclareLaunchArgument(
        'tf_pitch',
        default_value='0',
        description='Radar frame rotation pitch (radians)',
    )
    tf_roll_arg = DeclareLaunchArgument(
        'tf_roll',
        default_value='0',
        description='Radar frame rotation roll (radians)',
    )
    camera_tf_x_arg = DeclareLaunchArgument(
        'camera_tf_x',
        default_value='0',
        description='Camera frame translation x (meters)',
    )
    camera_tf_y_arg = DeclareLaunchArgument(
        'camera_tf_y',
        default_value='0',
        description='Camera frame translation y (meters)',
    )
    camera_tf_z_arg = DeclareLaunchArgument(
        'camera_tf_z',
        default_value='0',
        description='Camera frame translation z (meters)',
    )
    camera_tf_yaw_arg = DeclareLaunchArgument(
        'camera_tf_yaw',
        default_value='0',
        description='Camera frame rotation yaw (radians)',
    )
    camera_tf_pitch_arg = DeclareLaunchArgument(
        'camera_tf_pitch',
        default_value='0',
        description='Camera frame rotation pitch (radians)',
    )
    camera_tf_roll_arg = DeclareLaunchArgument(
        'camera_tf_roll',
        default_value='0',
        description='Camera frame rotation roll (radians)',
    )
    lidar_tf_x_arg = DeclareLaunchArgument(
        'lidar_tf_x',
        default_value='0',
        description='Lidar frame translation x (meters)',
    )
    lidar_tf_y_arg = DeclareLaunchArgument(
        'lidar_tf_y',
        default_value='0',
        description='Lidar frame translation y (meters)',
    )
    lidar_tf_z_arg = DeclareLaunchArgument(
        'lidar_tf_z',
        default_value='0',
        description='Lidar frame translation z (meters)',
    )
    lidar_tf_yaw_arg = DeclareLaunchArgument(
        'lidar_tf_yaw',
        default_value='0',
        description='Lidar frame rotation yaw (radians)',
    )
    lidar_tf_pitch_arg = DeclareLaunchArgument(
        'lidar_tf_pitch',
        default_value='0',
        description='Lidar frame rotation pitch (radians)',
    )
    lidar_tf_roll_arg = DeclareLaunchArgument(
        'lidar_tf_roll',
        default_value='0',
        description='Lidar frame rotation roll (radians)',
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
        arguments=[
            '--x',
            LaunchConfiguration('tf_x'),
            '--y',
            LaunchConfiguration('tf_y'),
            '--z',
            LaunchConfiguration('tf_z'),
            '--yaw',
            LaunchConfiguration('tf_yaw'),
            '--pitch',
            LaunchConfiguration('tf_pitch'),
            '--roll',
            LaunchConfiguration('tf_roll'),
            '--frame-id',
            LaunchConfiguration('base_frame'),
            '--child-frame-id',
            LaunchConfiguration('radar_frame'),
        ],
    )
    camera_tf = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='radar_camera_tf',
        arguments=[
            '--x',
            LaunchConfiguration('camera_tf_x'),
            '--y',
            LaunchConfiguration('camera_tf_y'),
            '--z',
            LaunchConfiguration('camera_tf_z'),
            '--yaw',
            LaunchConfiguration('camera_tf_yaw'),
            '--pitch',
            LaunchConfiguration('camera_tf_pitch'),
            '--roll',
            LaunchConfiguration('camera_tf_roll'),
            '--frame-id',
            LaunchConfiguration('radar_frame'),
            '--child-frame-id',
            LaunchConfiguration('camera_frame'),
        ],
    )
    lidar_tf = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        name='radar_lidar_tf',
        arguments=[
            '--x',
            LaunchConfiguration('lidar_tf_x'),
            '--y',
            LaunchConfiguration('lidar_tf_y'),
            '--z',
            LaunchConfiguration('lidar_tf_z'),
            '--yaw',
            LaunchConfiguration('lidar_tf_yaw'),
            '--pitch',
            LaunchConfiguration('lidar_tf_pitch'),
            '--roll',
            LaunchConfiguration('lidar_tf_roll'),
            '--frame-id',
            LaunchConfiguration('radar_frame'),
            '--child-frame-id',
            LaunchConfiguration('lidar_frame'),
        ],
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
        camera_frame_arg,
        lidar_frame_arg,
        tf_x_arg,
        tf_y_arg,
        tf_z_arg,
        tf_yaw_arg,
        tf_pitch_arg,
        tf_roll_arg,
        camera_tf_x_arg,
        camera_tf_y_arg,
        camera_tf_z_arg,
        camera_tf_yaw_arg,
        camera_tf_pitch_arg,
        camera_tf_roll_arg,
        lidar_tf_x_arg,
        lidar_tf_y_arg,
        lidar_tf_z_arg,
        lidar_tf_yaw_arg,
        lidar_tf_pitch_arg,
        lidar_tf_roll_arg,
        static_tf,
        camera_tf,
        lidar_tf,
        sensors_launch,
        perception_placeholder,
        decision_placeholder,
    ])
