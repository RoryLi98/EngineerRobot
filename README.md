# EngineerRobot

<div align=center><img src="https://github.com/LinkLiar/ImageStorage/blob/master/SchoolBadge.png" width="200" height="265"/><img src="https://github.com/LinkLiar/ImageStorage/blob/master/CollegeBadge.png" width="200" height="265"/></div>

### Development Environment

PC: ASUS S400C  
OS: Ubuntu 18.04  
IDE: Qt Creator 5.8  
RGB-D Camera: Orbbec Astra Pro  

### Qmake Setting：

    INCLUDEPATH += /usr/local/include \
                   /usr/local/include/opencv \
                   /usr/local/include/opencv2 \
                   /usr/include/openni2
                 
    LIBS += `pkg-config opencv --cflags --libs`
    LIBS += /home/link/NewDepthTest/DepthTest/libOpenNI2.so

<div align=center><img src="https://github.com/LinkLiar/ImageStorage/blob/master/%E5%B7%A5%E7%A8%8B%E6%9C%BA%E5%99%A8%E4%BA%BA%E8%B0%83%E8%AF%95.png"/></div>

深度摄像头视角效果演示：https://github.com/LinkLiar/ImageStorage/blob/master/SimulationForEngineer.gif  