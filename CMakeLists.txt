cmake_minimum_required(VERSION 3.5)
project(smartcam)

SET(CMAKE_INSTALL_PREFIX $ENV{OECORE_TARGET_SYSROOT})
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-narrowing -fpermissive -rdynamic -Wl,--no-undefined" )
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++14")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# 1. Find Packages
find_package(GStreamer REQUIRED)
find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)

# 2. Specifically look for XRT using PkgConfig
pkg_check_modules(XRT REQUIRED IMPORTED_TARGET xrt)

SET(INSTALL_PATH "opt/xilinx/kv260-smartcam")

# --- vvas_xpp ---
add_library(vvas_xpp SHARED src/vvas_xpp_pipeline.c)
target_include_directories(vvas_xpp PRIVATE ${GSTREAMER_INCLUDE_DIRS})
target_link_libraries(vvas_xpp jansson vvasutil-3.0 gstvvasinfermeta-3.0)
install(TARGETS vvas_xpp DESTINATION ${INSTALL_PATH}/lib)


# --- vvas_text2overlay (MODIFIED FOR XRT) ---
add_library(vvas_text2overlay SHARED src/vvas_text2overlay.cpp)

# Add XRT and GStreamer include paths
set(XRT_CORRECT_INCLUDE_DIR "${CMAKE_SYSROOT}/usr/include/xrt")

target_include_directories(vvas_text2overlay PRIVATE 
    ${GSTREAMER_INCLUDE_DIRS}
    ${XRT_CORRECT_INCLUDE_DIR}
)


message(STATUS "--- Debugging XRT Paths ---")
message(STATUS "CMAKE root: ${CMAKE_SYSROOT}")
message(STATUS "XRT_INCLUDE: ${XRT_CORRECT_INCLUDE_DIR}")
message(STATUS "XRT_INCLUDE: ${XRT_INCLUDE_DIRS}")
message(STATUS "XRT_LIBRARIES:    ${XRT_LIBRARIES}")
message(STATUS "-----------------------------------------")

# Link XRT and other dependencies
target_link_libraries(vvas_text2overlay
    gstreamer-1.0 
    glib-2.0 
    gstvvasinfermeta-3.0 
    jansson 
    vvasutil-3.0 
    ${OpenCV_LIBS} 
    ${XRT_LIBRARIES}     # Added XRT libraries here
    glog
)
install(TARGETS vvas_text2overlay DESTINATION ${INSTALL_PATH}/lib)

# --- main executable ---
add_executable(${CMAKE_PROJECT_NAME} src/main.cpp)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${GSTREAMER_INCLUDE_DIRS})
target_link_libraries(${CMAKE_PROJECT_NAME}
  gstapp-1.0 gstreamer-1.0 gstbase-1.0 gobject-2.0 glib-2.0 gstvideo-1.0 gstallocators-1.0 gstrtsp-1.0 gstrtspserver-1.0
  glib-2.0 gobject-2.0 gstvvasinfermeta-3.0 vvasutil-3.0)
  
install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION ${INSTALL_PATH}/bin)


install(FILES
    README
    DESTINATION ${INSTALL_PATH}/
    RENAME      README_SMARTCAM
    )

install(FILES
    config/text2overlay.json
    DESTINATION ${INSTALL_PATH}/share/vvas/)

install(DIRECTORY
    config/facedetect
    config/refinedet
    config/ssd
    DESTINATION ${INSTALL_PATH}/share/vvas/)

install(DIRECTORY
    models/kv260/densebox_640_360
	models/kv260/refinedet_pruned_0_96
	models/kv260/ssd_adas_pruned_0_95
    DESTINATION ${INSTALL_PATH}/share/vitis_ai_library/models/)

install(DIRECTORY
    notebook/
    DESTINATION ${INSTALL_PATH}/share/notebooks/)

set(VERSION "1.0.1")
set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_GENERATOR "RPM")
set(CPACK_PACKAGE_NAME "${CMAKE_PROJECT_NAME}")
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "Yuxiz@xilinx.com")
set(CPACK_PACKAGE_VENDOR "xilinx")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")
set(CPACK_RPM_PACKAGE_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
set(CPACK_PACKAGING_INSTALL_PREFIX "/")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_RPM_SPEC_MORE_DEFINE "%define _build_id_links none")
include(CPack)

