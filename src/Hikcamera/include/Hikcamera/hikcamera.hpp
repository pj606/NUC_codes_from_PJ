#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <stdexcept>
#include <thread>
#include <vector>


#include <opencv2/core.hpp>

#include "MvCameraControl.h"

namespace hik_camera {

struct HikImage {
  // BGR8
  cv::Mat     img;
  uint64_t    timestamp{0};
};

struct HikStatus {

  uint32_t      code{MV_OK};
  std::string   message;

  explicit operator bool() const { return code == MV_OK; }
};

class HikCamera {
public:
  HikCamera() = default;
  ~HikCamera() noexcept;

  HikCamera(const HikCamera&) = delete;
  HikCamera& operator=(const HikCamera&) = delete;

  HikCamera(HikCamera&&) = delete;
  HikCamera& operator=(HikCamera&&) = delete;

  HikStatus open(const std::string& serial = "");

  HikStatus close();

  HikStatus start_grab();

  HikStatus stop_grab();

  HikStatus setExposure(float us);

  HikStatus setGain(float gain);

  HikStatus setFps(float fps);

  bool getLatestFrame(HikImage& out);

  bool isOpened() const;

  bool isRunning() const;

private:
  struct BufferSlot {
    std::vector<uint8_t>  data;
    uint32_t              w{0};
    uint32_t              h{0};
    uint64_t              ts_ns{0};
    bool                  valid{false};
  };

  void grabLoop();


  void*       handle_{nullptr};

  BufferSlot  buffer_slot_[2];
  int         write_idx_{0};
  int         read_idx_{1};

  std::vector<uint8_t>  raw_buffer_;

  mutable std::mutex    device_mtx_;
  mutable std::mutex    buffer_mtx_;
  std::atomic<bool>     is_opened_{false};
  std::atomic<bool>     is_running_{false};
  std::thread           grab_thread_;
};

} // namespace hik_camera