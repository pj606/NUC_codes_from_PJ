from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode


def generate_launch_description():
    params_file_arg = DeclareLaunchArgument(
        'params_file',
        default_value='src/config/camera/camera_components.yaml',
        description='YAML file for camera and viewer component parameters'
    )

    image_topic_arg = DeclareLaunchArgument(
        'image_topic',
        default_value='image_raw',
        description='Image topic to subscribe'
    )

    window_name_arg = DeclareLaunchArgument(
        'window_name',
        default_value='camera_view',
        description='OpenCV imshow window name'
    )

    camera_component = ComposableNode(
        package='hik_camera',
        plugin='hik_camera::CameraNode',
        name='hik_camera_node',
        parameters=[
            LaunchConfiguration('params_file'),
            {
                'image_topic': LaunchConfiguration('image_topic'),
            }
        ]
    )

    viewer_component = ComposableNode(
        package='test_pkg',
        plugin='test_pkg::CameraImageViewNode',
        name='camera_image_view_node',
        parameters=[
            LaunchConfiguration('params_file'),
            {
                'image_topic': LaunchConfiguration('image_topic'),
                'window_name': LaunchConfiguration('window_name'),
            }
        ]
    )

    container = ComposableNodeContainer(
        name='camera_component_container',
        namespace='',
        package='rclcpp_components',
        executable='component_container_mt',
        output='screen',
        composable_node_descriptions=[
            camera_component,
            viewer_component,
        ]
    )

    return LaunchDescription([
        params_file_arg,
        image_topic_arg,
        window_name_arg,
        container,
    ])
