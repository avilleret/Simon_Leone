project(Simon_Leone)

cmake_minimum_required(VERSION 2.8)

set(OF_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../of/" CACHE PATH "The root directory of ofnode/of project.")
INCLUDE_DIRECTORIES("/usr/include/gstreamer-1.0")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/ofxIO/libs/snappy/src/")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/ofxIO/libs/lz4/src/")

include(${OF_ROOT}/openFrameworks.cmake)

ofxaddon(ofxXmlSettings)
ofxaddon(ofxSerial)

set(SOURCES
    src/main.cpp
    src/ofApp.cpp
    src/simon.h
)

if(APPLE)
    message(STATUS "copy library dependency")
    list(APPEND DEPENDENCIES /usr/local/opt/mpg123/lib/libmpg123.0.dylib)
    list(APPEND DEPENDENCIES /usr/local/opt/libsndfile/lib/libsndfile.1.dylib)
    list(APPEND DEPENDENCIES /usr/local/opt/boost/lib/libboost_system-mt.dylib)
    list(APPEND DEPENDENCIES /usr/local/opt/boost/lib/libboost_filesystem-mt.dylib)

    set_source_files_properties(${DEPENDENCIES} PROPERTIES MACOSX_PACKAGE_LOCATION Ressources )

    set(MACOSX_RPATH TRUE)
    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    set(CMAKE_INSTAL_RPATH "@loader_path/")

endif()

add_executable(
    ${PROJECT_NAME}
    MACOSX_BUNDLE
    ${SOURCES}
    ${OFXADDONS_SOURCES}
    ${DEPENDENCIES}
)

target_link_libraries(
    ${PROJECT_NAME}
    ${OPENFRAMEWORKS_LIBRARIES}
)

ofSetTargetProperties()

if(APPLE)
  get_property(loc TARGET ${PROJECT_NAME} PROPERTY LOCATION)
  get_property(dir TARGET ${PROJECT_NAME} PROPERTY OUTPUT_DIRECTORY)
  message(STATUS " -----> ${loc}")
  message(STATUS " -----> ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE}")
  set(BIN_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin/${PROJECT_NAME}.app/Contents/MacOS/${PROJECT_NAME}")
  foreach(lib ${DEPENDENCIES} )
      get_filename_component(name ${lib} NAME)
      message("libname: ${name}")
      add_custom_target(${name} ALL install_name_tool -change ${lib} "@loader_path/../Ressources/${name}" ${BIN_PATH})
      add_dependencies(${name} ${PROJECT_NAME})
  endforeach()
endif()


