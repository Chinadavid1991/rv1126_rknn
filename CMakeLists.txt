cmake_minimum_required(VERSION 3.8.2)
PROJECT (PROJECT)

set(CMAKE_CXX_STANDARD 11)
add_definitions(-g -O0 -ggdb -gdwarf -funwind-tables -rdynamic)
add_definitions(-DDEBUG -DRKAIQ)



aux_source_directory(${PROJECT_SOURCE_DIR}/common COMMON_SRCS)
set(LIBS easymedia rkaiq pthread rknn_api rtsp)
set(OpenCV_LIBS 
	opencv_calib3d
	opencv_core
	opencv_imgcodecs
	opencv_imgproc)
#rkmedia
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/easymedia)
include_directories(${PROJECT_SOURCE_DIR}/include/rkmedia)
# rkaiq
include_directories(${CMAKE_SYSROOT}/usr/include/rkaiq/algos)
include_directories(${CMAKE_SYSROOT}/usr/include/rkaiq/common)
include_directories(${CMAKE_SYSROOT}/usr/include/rkaiq/iq_parser)
include_directories(${CMAKE_SYSROOT}/usr/include/rkaiq/uAPI)
include_directories(${CMAKE_SYSROOT}/usr/include/rkaiq/xcore)

link_directories(${PROJECT_SOURCE_DIR}/lib
				 ${PROJECT_SOURCE_DIR}/lib/opencv)

add_executable(main main.cpp RV_VideoCapture.cpp RV_VideoCapture.h RV_RKNN.cpp RV_RKNN.h ${COMMON_SRCS})
target_link_libraries(main ${LIBS} ${OpenCV_LIBS})

add_executable(write_jpeg write_jpeg.c  ${COMMON_SRCS})
target_link_libraries(write_jpeg ${LIBS})

add_executable(vi_rtsp vi_rtsp.c  ${COMMON_SRCS})
target_link_libraries(vi_rtsp ${LIBS})

install(TARGETS main DESTINATION bin)
install(TARGETS write_jpeg DESTINATION bin)
install(TARGETS vi_rtsp DESTINATION bin)
