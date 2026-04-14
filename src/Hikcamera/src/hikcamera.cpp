#include <cstring>
#include <iostream>

#include "Hikcamera/hikcamera.hpp"


namespace hik_camera {

HikCamera::~HikCamera() 
{
  this->stop_grab();
  this->close();

  std::cout << "[hik_camera] HikCamera destructed" << std::endl;
}






}