#include <cstring>
#include <iostream>
#include <memory>


#include "Hikcamera/hik_camera.hpp"


namespace hik_camera {

HikCamera::~HikCamera() noexcept
{
  (void)this->stop_grab();
  (void)this->close();

  std::cout << "[hik_camera] HikCamera destructed" << std::endl;
}


HikStatus HikCamera::open(const std::string& serial)
{
  std::lock_guard<std::mutex> lock(device_mtx_);
  
  if(this->is_opened_.load()) {
    return HikStatus{MV_OK, "[hikcamera.cpp line 25]: Camera already opened"};
  }

  MV_CC_DEVICE_INFO_LIST device_list;
  memset(&device_list, 0, sizeof(MV_CC_DEVICE_INFO_LIST));

  int nRet = MV_CC_EnumDevices(MV_USB_DEVICE, &device_list);

  if(nRet != MV_OK) {
    return HikStatus{static_cast<uint32_t>(nRet), "[hikcamera.cpp line 40]: Failed to enumerate devices"};
  }
  if(device_list.nDeviceNum == 0) {
    return HikStatus{MV_E_NODATA, "[hikcamera.cpp line 43]: No camera found"};
  }

  MV_CC_DEVICE_INFO* selected = nullptr;

  if(serial.empty()) {
    selected = device_list.pDeviceInfo[0];
  }
  else {
    char sn[INFO_MAX_BUFFER_SIZE] = {0};
    for (size_t i = 0; i < device_list.nDeviceNum; ++i) {
      auto* info = device_list.pDeviceInfo[i];

      if(!info) {
        continue;
      }

      std::memcpy(sn, info->SpecialInfo.stUsb3VInfo.chSerialNumber, INFO_MAX_BUFFER_SIZE);

      sn[INFO_MAX_BUFFER_SIZE - 1] = '\0';

      if(serial == sn) {
        selected = info;
        break;
      }
    }
    
    if(!selected) {
      return HikStatus{MV_E_NODATA, "[hikcamera.cpp line 60]: No camera with the specified serial number found"};
    }
  }

  nRet = MV_CC_CreateHandle(&handle_, selected);
  if(nRet != MV_OK || handle_ == nullptr) {
    handle_ = nullptr;
    return HikStatus{static_cast<uint32_t>(nRet), "[hikcamera.cpp line 68]: Failed to create camera handle"};
  }

  nRet = MV_CC_OpenDevice(handle_);
  if(nRet != MV_OK) {
    MV_CC_DestroyHandle(handle_);
    handle_ = nullptr;
    return HikStatus{static_cast<uint32_t>(nRet), "[hikcamera.cpp line 75]: Failed to open camera"};
  }

  // params setting

  is_opened_.store(true);
  return HikStatus{MV_OK, "ok"};
}


HikStatus HikCamera::close()
{
  std::lock_guard<std::mutex> lock(device_mtx_);

  if(!is_opened_.load()) {
    return HikStatus{MV_OK, "[hikcamera.cpp line 94]: Camera already closed"};
  }
  if(is_running_.load()) {
    return HikStatus{MV_E_CALLORDER, "[hikcamera.cpp line 97]: Cannot close camera while grabbing"};
  }

  int nRet = MV_CC_CloseDevice(handle_);
  if(nRet != MV_OK) {
    return HikStatus{static_cast<uint32_t>(nRet), "[hikcamera.cpp line 102]: Failed to close camera"};
  }

  nRet = MV_CC_DestroyHandle(handle_);
  if(nRet != MV_OK) {
    return HikStatus{static_cast<uint32_t>(nRet), "[hikcamera.cpp line 107]: Failed to destroy camera handle"};
  }

  handle_ = nullptr;
  is_opened_.store(false);

  {
    std::lock_guard<std::mutex> buf(this->buffer_mtx_);
    this->buffer_slot_[0].valid = false;
    this->buffer_slot_[1].valid = false;
  }
  return HikStatus{MV_OK, "ok"};
}


HikStatus HikCamera::start_grab()
{
  {
    std::lock_guard<std::mutex> lock(device_mtx_);

    if(!is_opened_.load()) {
      return HikStatus{MV_E_HANDLE, "[hikcamera.cpp line 128]: Cannot start grabbing when camera is not opened"};
    }
    if(is_running_.load()) {
      return HikStatus{MV_OK, "[hikcamera.cpp line 131]: Grabbing already started"};
    }

    int nRet = MV_CC_StartGrabbing(handle_);
    if(nRet != MV_OK) {
      return HikStatus{static_cast<uint32_t>(nRet), "[hikcamera.cpp line 136]: Failed to start grabbing"};
    }

    is_running_.store(true);
  }

  try {
    grab_thread_ = std::thread(&HikCamera::grabLoop, this);
  } catch (...) {
    std::lock_guard<std::mutex> lock(device_mtx_);
    is_running_.store(false);

    (void)MV_CC_StopGrabbing(handle_);

    return HikStatus{MV_E_RESOURCE, "[hikcamera.cpp line 150]: Failed to start grab thread"};
   }

  return HikStatus{MV_OK, "ok"};
}


HikStatus HikCamera::stop_grab()
{
  bool just_running = is_running_.exchange(false);

  if(just_running && grab_thread_.joinable()) {
    grab_thread_.join();
  }

  {
    std::lock_guard<std::mutex> lock(device_mtx_);

    if(!is_opened_.load()) {
      return HikStatus{MV_E_HANDLE, "[hikcamera.cpp line 179]: Cannot stop grabbing when camera is not opened"};
    }
    if(!is_running_.load()) {
      return HikStatus{MV_OK, "[hikcamera.cpp line 172]: Grabbing already stopped"};
    }

    is_running_.store(false);
  }

  int nRet = MV_CC_StopGrabbing(handle_);
  if(nRet != MV_OK) {
    return HikStatus{static_cast<uint32_t>(nRet), "[hikcamera.cpp line 180]: Failed to stop grabbing"};
  }

  return HikStatus{MV_OK, "ok"};
}


HikStatus HikCamera::setExposure(float us)
{
  std::lock_guard<std::mutex> lock(device_mtx_);

  if(!is_opened_.load()) {
    return HikStatus{MV_E_HANDLE, "[hikcamera.cpp line 192]: Cannot set exposure when camera is not opened"};
  }
  if(us <= 0.0f) {
    return HikStatus{MV_E_PARAMETER, "[hikcamera.cpp line 195]: Invalid exposure time"};
  }

  int nRet = MV_CC_SetEnumValue(handle_, "ExposureMode", MV_EXPOSURE_AUTO_MODE_OFF);
  if(nRet != MV_OK) {
    return HikStatus{static_cast<uint32_t>(nRet), "[hikcamera.cpp line 200]: Failed to set exposure time"};
  }

  nRet = MV_CC_SetFloatValue(handle_, "ExposureTime", us);
  if(nRet != MV_OK) {
    return HikStatus{static_cast<uint32_t>(nRet), "[hikcamera.cpp line 205]: Failed to set exposure time"};
  }

  return HikStatus{MV_OK, "ok"};
}


HikStatus HikCamera::setGain(float gain)
{
  std::lock_guard<std::mutex> lock(device_mtx_);

  if(!is_opened_.load()) {
    return HikStatus{MV_E_HANDLE, "[hikcamera.cpp line 217]: Cannot set gain when camera is not opened"};
  }
  if(gain < 0.0f) {
    return HikStatus{MV_E_PARAMETER, "[hikcamera.cpp line 220]: Invalid gain value"};
  }

  int nRet = MV_CC_SetFloatValue(handle_, "Gain", gain);
  if(nRet != MV_OK) {
    return HikStatus{static_cast<uint32_t>(nRet), "[hikcamera.cpp line 225]: Failed to set gain"};
  }

  return HikStatus{MV_OK, "ok"};
}


HikStatus HikCamera::setFps(float fps)
{
  std::lock_guard<std::mutex> lock(device_mtx_);

  if(!is_opened_.load()) {
    return HikStatus{MV_E_HANDLE, "[hikcamera.cpp line 237]: Cannot set FPS when camera is not opened"};
  }
  if(fps <= 0.0f) {
    return HikStatus{MV_E_PARAMETER, "[hikcamera.cpp line 240]: Invalid FPS value"};
  }

  int nRet = MV_CC_SetFloatValue(handle_, "AcquisitionFrameRate", fps);
  if(nRet != MV_OK) {
    return HikStatus{static_cast<uint32_t>(nRet), "[hikcamera.cpp line 245]: Failed to set FPS"};
  }

  return HikStatus{MV_OK, "ok"};
}


bool HikCamera::getLatestFrame(HikImage& out)
{
  std::lock_guard<std::mutex> lock(buffer_mtx_);

  const auto& slot = buffer_slot_[read_idx_];

  if(!slot.valid || slot.w == 0 || slot.h == 0 || slot.data.empty()) {
    return false;
  }

  cv::Mat view(static_cast<int>(slot.h), static_cast<int>(slot.w), CV_8UC3, 
                const_cast<uint8_t*>(slot.data.data()));
  view.copyTo(out.img);
  out.timestamp = slot.ts_ns;
  return true;
}


bool HikCamera::isOpened() const
{
  return is_opened_.load();
}


bool HikCamera::isRunning() const
{
  return is_running_.load();
}


void HikCamera::grabLoop()
{
  while(is_running_.load()) {
    MV_FRAME_OUT out_frame{};

    int nRet = MV_CC_GetImageBuffer(handle_, &out_frame, 1000);
    if(nRet != MV_OK) {
      continue;
    }

    const uint32_t w = out_frame.stFrameInfo.nWidth;
    const uint32_t h = out_frame.stFrameInfo.nHeight;
    const uint32_t data_len = out_frame.stFrameInfo.nFrameLen;

    if(raw_buffer_.size() < data_len) {
      raw_buffer_.resize(data_len);
    }

    std::memcpy(raw_buffer_.data(), out_frame.pBufAddr, data_len);

    MV_CC_PIXEL_CONVERT_PARAM convert_param{};
    convert_param.nWidth = w;
    convert_param.nHeight = h;
    convert_param.pSrcData = raw_buffer_.data();
    convert_param.nSrcDataLen = data_len;
    convert_param.enSrcPixelType = out_frame.stFrameInfo.enPixelType;
    convert_param.enDstPixelType = PixelType_Gvsp_BGR8_Packed;
    convert_param.nDstBufferSize = w * h * 3;

    int wi;
    {
      std::lock_guard<std::mutex> lock(buffer_mtx_);
      wi = write_idx_;
      if(buffer_slot_[wi].data.size() < convert_param.nDstBufferSize) {
        buffer_slot_[wi].data.resize(convert_param.nDstBufferSize);
      }
      convert_param.pDstBuffer = buffer_slot_[wi].data.data();
    }

    nRet = MV_CC_ConvertPixelType(handle_, &convert_param);

    MV_CC_FreeImageBuffer(handle_, &out_frame);

    if(nRet != MV_OK) {
      continue;
    }

    {
      std::lock_guard<std::mutex> lock(buffer_mtx_);
      buffer_slot_[wi].w = w;
      buffer_slot_[wi].h = h;
      buffer_slot_[wi].ts_ns = (static_cast<uint64_t>(out_frame.stFrameInfo.nDevTimeStampHigh) << 32) |
                                static_cast<uint64_t>(out_frame.stFrameInfo.nDevTimeStampLow);
      buffer_slot_[wi].valid = true;

      std::swap(write_idx_, read_idx_);
    }
  }
}

} // namespace hik_camera