cmake_minimum_required(VERSION 2.8.3)
project(inmoov_speech_recognizer)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(PocketSphinx)
find_package(SphinxBase)
find_package(
  catkin
  REQUIRED
  COMPONENTS
  roslaunch
  roscpp
  std_msgs
)

catkin_package()

roslaunch_add_file_check(launch)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  ${SphinxBase_INCLUDE_DIRS}
  ${PocketSphinx_INCLUDE_DIRS}
)

add_executable(
  recognizer
  src/recognizer.cpp
)
add_dependencies(
  recognizer
  ${${PROJECT_NAME}_EXPORTED_TARGETS}
  ${catkin_EXPORTED_TARGETS}
)
target_link_libraries(
  recognizer
  ${catkin_LIBRARIES}
  ${PocketSphinx_LIBRARIES}
  ${SphinxBase_LIBRARIES}
  ${SphinxAD_LIBRARIES}
)

install(
  DIRECTORY launch
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
)
