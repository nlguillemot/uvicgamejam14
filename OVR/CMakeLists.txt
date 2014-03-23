CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(OVR CXX)

SET(COMMON_SOURCES
    src/Kernel/OVR_Alg.cpp
    src/Kernel/OVR_Allocator.cpp
    src/Kernel/OVR_Atomic.cpp
    src/Kernel/OVR_File.cpp
    src/Kernel/OVR_FileFILE.cpp
    src/Kernel/OVR_Log.cpp
    src/Kernel/OVR_Math.cpp
    src/Kernel/OVR_RefCount.cpp
    src/Kernel/OVR_Std.cpp
    src/Kernel/OVR_String.cpp
    src/Kernel/OVR_String_FormatUtil.cpp
    src/Kernel/OVR_String_PathUtil.cpp
    src/Kernel/OVR_SysFile.cpp
    src/Kernel/OVR_System.cpp
    src/Kernel/OVR_Timer.cpp
    src/Kernel/OVR_UTF8Util.cpp
    src/Util/Util_LatencyTest.cpp
    src/Util/Util_Render_Stereo.cpp
    src/OVR_DeviceHandle.cpp
    src/OVR_DeviceImpl.cpp
    src/OVR_JSON.cpp
    src/OVR_LatencyTestImpl.cpp 
    src/OVR_Profile.cpp
    src/OVR_SensorFilter.cpp
    src/OVR_SensorFusion.cpp
    src/OVR_SensorImpl.cpp
    src/OVR_ThreadCommandQueue.cpp)

SET(LINUX_SOURCES
    src/Kernel/OVR_ThreadsPthread.cpp
    src/OVR_Linux_DeviceManager.cpp
    include/OVR_Linux_DeviceManager.h
    src/OVR_Linux_HIDDevice.cpp
    include/OVR_Linux_HIDDevice.h
    src/OVR_Linux_HMDDevice.cpp
    include/OVR_Linux_HMDDevice.h
    src/OVR_Linux_SensorDevice.cpp)

SET(OSX_SOURCES
    src/Kernel/OVR_ThreadsPthread.cpp
    src/OVR_OSX_DeviceManager.cpp
    include/OVR_OSX_DeviceManager.h
    src/OVR_OSX_HIDDevice.cpp
    include/OVR_OSX_HIDDevice.h
    src/OVR_OSX_HMDDevice.cpp
    include/OVR_OSX_HMDDevice.h
    src/OVR_OSX_SensorDevice.cpp)

SET(WIN32_SOURCES
    src/Kernel/OVR_ThreadsWinAPI.cpp
    src/OVR_Win32_DeviceManager.cpp
    include/OVR_Win32_DeviceManager.h
    src/OVR_Win32_DeviceStatus.cpp
    include/OVR_Win32_DeviceStatus.h
    src/OVR_Win32_HIDDevice.cpp
    include/OVR_Win32_HIDDevice.h
    src/OVR_Win32_HMDDevice.cpp
    include/OVR_Win32_HMDDevice.h
    src/OVR_Win32_SensorDevice.cpp
    include/OVR_Win32_SensorDevice.h)
    
SET(HEADERS
    include/OVR.h
    include/OVRVersion.h
    include/OVR_DeviceConstants.h
    include/OVR_Device.h
    include/OVR_DeviceHandle.h
    include/OVR_DeviceMessages.h
    include/OVR_Profile.h
    include/OVR_SensorFusion.h
    include/OVR_DeviceImpl.h
    include/OVR_HIDDeviceBase.h
    include/OVR_HIDDevice.h
    include/OVR_HIDDeviceImpl.h
    include/OVR_JSON.h
    include/OVR_LatencyTestImpl.h
    include/OVR_SensorFilter.h
    include/OVR_SensorImpl.h
    include/OVR_ThreadCommandQueue.h
    include/Kernel/OVR_Alg.h
    include/Kernel/OVR_Allocator.h
    include/Kernel/OVR_Array.h
    include/Kernel/OVR_Atomic.h
    include/Kernel/OVR_Color.h
    include/Kernel/OVR_ContainerAllocator.h
    include/Kernel/OVR_File.h
    include/Kernel/OVR_Hash.h
    include/Kernel/OVR_KeyCodes.h
    include/Kernel/OVR_List.h
    include/Kernel/OVR_Log.h
    include/Kernel/OVR_Math.h
    include/Kernel/OVR_RefCount.h
    include/Kernel/OVR_Std.h
    include/Kernel/OVR_String.h
    include/Kernel/OVR_StringHash.h
    include/Kernel/OVR_SysFile.h
    include/Kernel/OVR_System.h
    include/Kernel/OVR_Threads.h
    include/Kernel/OVR_Timer.h
    include/Kernel/OVR_Types.h
    include/Kernel/OVR_UTF8Util.h
    include/Util/Util_LatencyTest.h
    include/Util/Util_Render_Stereo.h)

IF(WIN32)
    SET(SOURCES ${COMMON_SOURCES} ${WIN32_SOURCES})
ELSEIF(UNIX AND NOT APPLE)
    SET(SOURCES ${COMMON_SOURCES} ${LINUX_SOURCES})
ELSE()
    SET(SOURCES ${COMMON_SOURCES} ${OSX_SOURCES})
ENDIF()

ADD_LIBRARY(ovr ${SOURCES} ${HEADERS})

TARGET_LINK_LIBRARIES(ovr TinyXml2)

INCLUDE_DIRECTORIES(include src include/Kernel include/Util src/Kernel src/Util
    ${TinyXml2_SOURCE_DIR}/include)

IF(WIN32)
    # TODO
ELSEIF(UNIX AND NOT APPLE)
    FIND_PACKAGE(UDev REQUIRED)
    FIND_PACKAGE(Threads REQUIRED)
    FIND_PACKAGE(X11 REQUIRED)
    FIND_PACKAGE(Xinerama REQUIRED)
    
    SET(OVR_LIBRARIES ovr
        ${UDEV_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        ${X11_LIBRARIES}
        ${XINERAMA_LIBRARIES}
        PARENT_SCOPE)
ELSE()
    # TODO
ENDIF()
