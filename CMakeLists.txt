cmake_minimum_required(VERSION 2.8)

project( LANE_DETECTION )

find_package( OpenCV REQUIRED )

add_executable( lanes lanes.cpp )
target_link_libraries( lanes ${OpenCV_LIBS} )

add_executable(new new.cpp)
target_link_libraries(new ${OpenCV_LIBS})