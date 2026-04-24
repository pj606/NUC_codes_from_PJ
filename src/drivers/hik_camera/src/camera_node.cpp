#include <cv_bridge/cv_bridge.h>
#include <rclcpp_components/register_node_macro.hpp>

#include "hik_camera/camera_node.hpp"

namespace hik_camera
{

CameraNode::CameraNode(const rclcpp::NodeOptions& options)
: Node("hik_camera_node", options)
{
    this->declareParams();

    camera_info_manager_ = std::make_shared<camera_info_manager::CameraInfoManager>(
        this,
        "hik_camera",
        param_camera_info_url_);

    if(!param_camera_info_url_.empty()) {
        if(!camera_info_manager_->validateURL(param_camera_info_url_)) {
            RCLCPP_ERROR(this->get_logger(), "Camera info URL is invalid: %s", param_camera_info_url_.c_str());
        } else if(!camera_info_manager_->loadCameraInfo(param_camera_info_url_)) {
            RCLCPP_ERROR(this->get_logger(), "Failed to load camera info from URL: %s", param_camera_info_url_.c_str());
        } else {
            RCLCPP_INFO(this->get_logger(), "Loaded camera info from URL: %s", param_camera_info_url_.c_str());

        }
    } else {
        RCLCPP_WARN(this->get_logger(), "Camera info URL is empty!");
    }

    image_pub_ = this->create_publisher<sensor_msgs::msg::Image>(
        param_image_topic_, 
        rclcpp::SensorDataQoS()
    );
    camera_info_pub_ = this->create_publisher<sensor_msgs::msg::CameraInfo>(
        param_camera_info_topic_,
        rclcpp::SensorDataQoS()
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

    if(openCamera()) {
        state_ = CameraState::STREAMING;
        RCLCPP_INFO(this->get_logger(), "Camera opened and streaming");
    } else {
        state_ = CameraState::DISCONNECTED;
        RCLCPP_WARN(this->get_logger(), "Camera open failed, waiting for reconnect timer");
    }

    RCLCPP_INFO(this->get_logger(),"Camera node initialized");
}

CameraNode::~CameraNode() noexcept
{
    std::lock_guard<std::mutex> lock(state_mtx_);
    stopCamera();
}

void CameraNode::declareParams()
{
    this->declare_parameter<std::string>("frame_id",            "camera");
    this->declare_parameter<std::string>("image_topic",         "image_raw");
    this->declare_parameter<std::string>("camera_info_topic",   "camera_info");
    this->declare_parameter<std::string>("camera_info_url",     "config/camera/camera_info.yaml");
    this->declare_parameter<std::string>("serial",              "DA1919491");
    this->declare_parameter<int>("capture_period_ms",       30);
    this->declare_parameter<int>("reconnect_period_ms",     5000);
    this->declare_parameter<int>("fail_threshold",          5);
    this->declare_parameter<int>("width",                   3072);
    this->declare_parameter<int>("height",                  2048);

    this->get_parameter("frame_id",             param_frame_id_);
    this->get_parameter("image_topic",          param_image_topic_);
    this->get_parameter("camera_info_topic",    param_camera_info_topic_);
    this->get_parameter("camera_info_url",      param_camera_info_url_);
    this->get_parameter("serial",               param_serial_);
    this->get_parameter("capture_period_ms",    param_capture_period_ms_);
    this->get_parameter("reconnect_period_ms",  param_reconnect_period_ms_);
    this->get_parameter("fail_threshold",       param_fail_threshold_);
    this->get_parameter("width",                param_width_);
    this->get_parameter("height",               param_height_);
}

bool CameraNode::openCamera()
{
    if(this->camera_.open(param_serial_).code != MV_OK) {
        RCLCPP_ERROR(this->get_logger(), "Failed to open camera, SN code is error");
        return false;
    }
    if(this->camera_.start_grab().code != MV_OK) {
        RCLCPP_ERROR(this->get_logger(), "Failed to start grabbing");
        this->camera_.close();
        return false;
    }
    return true;
}


void CameraNode::stopCamera()
{
    camera_.stop_grab();
    camera_.close();
}

void CameraNode::onCaptureTimer()
{
    std::lock_guard<std::mutex> lock(state_mtx_);
    if(state_ != CameraState::STREAMING) {
        return;
    }

    HikImage frame;

    if(!camera_.getLatestFrame(frame)) {
        ++fail_count_;
        if(fail_count_ >= param_fail_threshold_) {
            RCLCPP_WARN(this->get_logger(), "Capture failed %d times, stopping camera", fail_count_);
            stopCamera();
            state_ = CameraState::DISCONNECTED;
            fail_count_ = 0;
        }
        return;
    }

    fail_count_ = 0;

    rclcpp::Time stamp = this->now();
    if(frame.timestamp != 0) {
        stamp = rclcpp::Time(frame.timestamp);
    }

    std_msgs::msg::Header header;
    header.stamp = stamp;
    header.frame_id = param_frame_id_;

    auto image_msg = cv_bridge::CvImage(header, "bgr8", frame.img).toImageMsg();
    image_pub_->publish(*image_msg);

    auto camera_info = getCameraInfo(stamp);
    camera_info_pub_->publish(camera_info);

    ++frame_seq_;
}

void CameraNode::onReconnectTimer()
{
    std::lock_guard<std::mutex> lock(state_mtx_);

    if(state_ == CameraState::STREAMING) {
        return;
    }

    if(openCamera()) {
        state_ = CameraState::STREAMING;
        fail_count_ = 0;
        RCLCPP_INFO(this->get_logger(), "Camera reconnected");
    }
}

void CameraNode::onReconnectService(
    const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
    std::shared_ptr<std_srvs::srv::Trigger::Response> response)
{
    (void)request;
    std::lock_guard<std::mutex> lock(state_mtx_);

    stopCamera();

    if(openCamera()) {
        state_ = CameraState::STREAMING;
        fail_count_ = 0;
        response->success = true;
        response->message = "camera reconnected";
    } else {
        state_ = CameraState::DISCONNECTED;
        response->success = false;
        response->message = "failed to reconnect camera";
    }
}

sensor_msgs::msg::CameraInfo CameraNode::getCameraInfo(const rclcpp::Time& stamp) const
{
    sensor_msgs::msg::CameraInfo msg;
    if(camera_info_manager_) {
        msg = camera_info_manager_->getCameraInfo();
    }

    msg.header.stamp = stamp;
    msg.header.frame_id = param_frame_id_;

    if(msg.height == 0 || msg.width == 0) {
        msg.height = static_cast<uint32_t>(param_height_);
        msg.width = static_cast<uint32_t>(param_width_);
    }

    return msg;
}

} // namespace hik_camera

RCLCPP_COMPONENTS_REGISTER_NODE(hik_camera::CameraNode)
