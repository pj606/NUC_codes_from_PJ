#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include <opencv2/core.hpp>

#include "MvCameraControl.h"

namespace hik_camera {

struct HikImage {
  cv::Mat bgr;
  uint64_t device_ts_ns{0};
};

class HikCamera {
public:
  HikCamera() = default;
  ~HikCamera();

  bool openBySerial(const std::string& serial);
  void close();

  bool start();
  void stop();

  bool setExposure(double us);
  bool setGain(double gain);
  bool setFps(double fps);

  bool getLatestFrame(HikImage& out);

  std::string lastError() const;

private:
  struct BufferSlot {
    std::vector<uint8_t> data;
    uint32_t w{0}, h{0};
    uint64_t ts_ns{0};
    bool valid{false};
  };

  void grabLoop();
  bool applyExposureLocked(double us);
  bool applyGainLocked(double gain);
  bool applyFpsLocked(double fps);
  void setErrorLocked(const std::string& e);

private:
  mutable std::mutex mtx_;
  std::condition_variable cv_;
  std::atomic<bool> running_{false};
  std::thread th_;

  void* handle_{nullptr};
  bool is_open_{false};

  BufferSlot slots_[2];
  int write_idx_{0};
  int read_idx_{1};

  std::vector<uint8_t> raw_buf_;
  std::string last_error_;
};

} // namespace hik_camera_driver