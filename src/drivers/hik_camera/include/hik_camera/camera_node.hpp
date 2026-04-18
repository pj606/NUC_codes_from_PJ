#pragma once 

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
#include <std_srvs/srv/trigger.hpp>

#include <opencv2/opencv.hpp>

#include <mutex>
#include <string>

#include "hik_camera.hpp"

namespace hik_camera {

class CameraNode : public rclcpp::Node 
{
public:
    explicit CameraNode (const rclcpp::NodeOptions& options = rclcpp::NodeOptions());
    ~CameraNode() noexcept;

private:
    enum class CameraState {
        DISCONNECTED,
        STREAMING
    };

    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr       image_pub_;
    rclcpp::Publisher<sensor_msgs::msg::CameraInfo>::SharedPtr  camera_info_pub_;
    rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr          reconnect_srv_;
    rclcpp::TimerBase::SharedPtr                                capture_timer_;   
    rclcpp::TimerBase::SharedPtr                                reconnect_timer_;

    std::string     param_frame_id_;
    std::string     param_image_topic_;
    std::string     param_camera_info_topic_;
    int             param_capture_period_ms_;
    int             param_reconnect_period_ms_;
    int             param_fail_threshold_;
    int             param_width_;
    int             param_height_;

    HikCamera       camera_;
    CameraState     state_{CameraState::DISCONNECTED};
    std::mutex      state_mtx_;
    uint64_t        frame_seq_{0};
    int             fail_count_{0};

    void declareParams();
    
    void stopCamera();
    bool openCamera();

    void onCaptureTimer();
    void onReconnectTimer();
    void onReconnectService(
        const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
        std::shared_ptr<std_srvs::srv::Trigger::Response> response);
    
    sensor_msgs::msg::CameraInfo initCameraInfo(
        const rclcpp::Time& stamp) const;
};


} // namespace hik_camera