#include <iostream>
#include <opencv2/opencv.hpp>
#include "hik_camera/hik_camera.hpp"

int main() {
  hik_camera::HikCamera cam;
  auto st = cam.open("");
  if (!st) {
    std::cerr << "open failed: 0x" << std::hex << st.code << std::dec
              << " " << st.message << "\n";
    return 1;
  }

  st = cam.setExposure(8000.0f);

  st = cam.setGain(8.0f);

  st = cam.setFps(60.0f);
  


  st = cam.start_grab();
  if (!st) {
    std::cerr << "start_grab failed: 0x" << std::hex << st.code << std::dec
              << " " << st.message << "\n";
    return 1;
  }

  std::cout << "opened. ESC to quit.\n";
  while (true) {
    hik_camera::HikImage img;
    if (cam.getLatestFrame(img) && !img.img.empty()) {
      cv::imshow("hik_wrapper_test", img.img);
    }
    if (cv::waitKey(1) == 27) break;
  }

  (void)cam.stop_grab();
  (void)cam.close();
  return 0;
}