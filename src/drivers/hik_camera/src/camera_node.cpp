#include <cv_bridge/cv_bridge.h>

#include "hik_camera/camera_node.hpp"

namespace hik_camera
{

CameraNode::CameraNode(const rclcpp::NodeOptions& options)
: Node("hik_camera_node", options),
  state_(CameraState::DISCONNECTED),
  fail_count_(0),
  frame_seq_(0)
{
    this->declareParams();

    image_pub_ = this->create_publisher<sensor_msgs::msg::Image>(
        param_image_topic_, 
        10
    );
    camera_info_pub_ = this->create_publisher<sensor_msgs::msg::CameraInfo>(
        param_camera_info_topic_,
        10
    );

    reconnect_srv_ = this->create_service<std_srvs::srv::Trigger>(
        "reconnect",
        std::bind(&CameraNode::onReconnectService, this, std::placeholders::_1, std::placeholders::_2)
    );

    capture_timer_ = this->create_wall_timer(
        std::chrono::milliseconds(param_capture_period_ms_),
        std::bind(&CameraNode::onCaptureTimer, this)
    );

    reconnect_timer_ = this->create_wall_timer(
        std::chrono::milliseconds(param_reconnect_period_ms_),
        std::bind(&CameraNode::onReconnectTimer, this)
    );
}

CameraNode::~CameraNode() noexcept
{
    std::lock_guard<std::mutex> lock(state_mtx_);
    this->stopCamera();
}

void CameraNode::declareParams()
{
    this->declare_parameter<std::string>("frame_id",            "camera");
    this->declare_parameter<std::string>("image_topic",         "image_raw");
    this->declare_parameter<std::string>("camera_info_topic",   "camera_info");
    this->declare_parameter<int>("capture_period_ms",       30);
    this->declare_parameter<int>("reconnect_period_ms",     5000);
    this->declare_parameter<int>("fail_threshold",          5);
    this->declare_parameter<int>("width",                   1280);
    this->declare_parameter<int>("height",                  720);

    this->get_parameter("frame_id",             param_frame_id_);
    this->get_parameter("image_topic",          param_image_topic_);
    this->get_parameter("camera_info_topic",    param_camera_info_topic_);
    this->get_parameter("capture_period_ms",    param_capture_period_ms_);
    this->get_parameter("reconnect_period_ms",  param_reconnect_period_ms_);
    this->get_parameter("fail_threshold",       param_fail_threshold_);
    this->get_parameter("width",                param_width_);
    this->get_parameter("height",               param_height_);
}

void 




} // namespace hik_camera