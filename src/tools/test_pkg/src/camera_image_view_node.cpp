#include <string>

#include <opencv2/opencv.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_components/register_node_macro.hpp>
#include <sensor_msgs/msg/image.hpp>

namespace test_pkg {

class CameraImageViewNode : public rclcpp::Node
{
public:
  explicit CameraImageViewNode(const rclcpp::NodeOptions & options)
  : Node("camera_image_view_node", options)
  {
    image_topic_ = this->declare_parameter<std::string>("image_topic", "image_raw");
    window_name_ = this->declare_parameter<std::string>("window_name", "camera_view");

    cv::namedWindow(window_name_, cv::WINDOW_NORMAL);

    image_sub_ = this->create_subscription<sensor_msgs::msg::Image>(
      image_topic_,
      rclcpp::SensorDataQoS(),
      std::bind(&CameraImageViewNode::onImage, this, std::placeholders::_1));

    RCLCPP_INFO(this->get_logger(), "Subscribed image topic: %s", image_topic_.c_str());
  }

  ~CameraImageViewNode() override
  {
    cv::destroyWindow(window_name_);
  }

private:
  void onImage(const sensor_msgs::msg::Image::SharedPtr msg)
  {
    if (msg->data.empty() || msg->height == 0 || msg->width == 0) {
      return;
    }

    cv::Mat view;

    if (msg->encoding == "bgr8") {
      view = cv::Mat(static_cast<int>(msg->height), static_cast<int>(msg->width), CV_8UC3,
                     const_cast<unsigned char *>(msg->data.data()), static_cast<size_t>(msg->step));
    } else if (msg->encoding == "rgb8") {
      cv::Mat rgb(static_cast<int>(msg->height), static_cast<int>(msg->width), CV_8UC3,
                  const_cast<unsigned char *>(msg->data.data()), static_cast<size_t>(msg->step));
      cv::cvtColor(rgb, view, cv::COLOR_RGB2BGR);
    } else if (msg->encoding == "mono8") {
      cv::Mat gray(static_cast<int>(msg->height), static_cast<int>(msg->width), CV_8UC1,
                   const_cast<unsigned char *>(msg->data.data()), static_cast<size_t>(msg->step));
      cv::cvtColor(gray, view, cv::COLOR_GRAY2BGR);
    } else {
      RCLCPP_WARN_THROTTLE(
        this->get_logger(),
        *this->get_clock(),
        2000,
        "Unsupported image encoding: %s",
        msg->encoding.c_str());
      return;
    }

    if (view.empty()) {
      return;
    }

    cv::imshow(window_name_, view);
    cv::waitKey(1);
  }

  std::string image_topic_;
  std::string window_name_;

  rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr image_sub_;
};

}  // namespace test_pkg

RCLCPP_COMPONENTS_REGISTER_NODE(test_pkg::CameraImageViewNode)
