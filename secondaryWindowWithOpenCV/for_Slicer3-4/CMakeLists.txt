PROJECT(SecondaryWindowWithOpenCV)

cmake_minimum_required(VERSION 2.4)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# --------------------------------------------------------------------------
# Find Slicer3

if(NOT Slicer3_SOURCE_DIR)
  find_package(Slicer3 REQUIRED)
  include(${Slicer3_USE_FILE})
  slicer3_set_default_install_prefix_for_external_projects()
endif(NOT Slicer3_SOURCE_DIR)

# --------------------------------------------------------------------------
# OpenCV stuff

# Chinzei; Next 1 line is to include here to find FindXX.cmake files.
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})

# Chinzei; This does eerything you want!
FIND_PACKAGE(OpenCV REQUIRED)

# --------------------------------------------------------------------------

# --------------------------------------------------------------------------
# OpenIGTLink stuff   A.Yamada 2010-4-23;

#FIND_PACKAGE(OpenIGTLink REQUIRED)
## K.Chinzei 2009-10-13; add 3 lines
#IF(OpenIGTLink_FOUND)
#  INCLUDE("${OpenIGTLink_USE_FILE}")
#ENDIF(OpenIGTLink_FOUND)

# --------------------------------------------------------------------------
# Include dirs

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
#  ${OpenIGTLink_INCLUDE_DIRS} # A.Yamada 2010-4-23;
  )

#
# Configure include file
#
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkSecondaryWindowWithOpenCVConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkSecondaryWindowWithOpenCVConfigure.h
  )

file(GLOB headers 
  "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
  )

install(FILES 
  ${headers} 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkSecondaryWindowWithOpenCVConfigure.h"
  DESTINATION ${Slicer3_INSTALL_MODULES_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(SecondaryWindowWithOpenCV_SRCS 
  vtkSecondaryWindowWithOpenCVLogic.cxx
  vtkSecondaryWindowWithOpenCVGUI.cxx
  vtkSlicerSecondaryViewerWindow.cxx
  vtkSlicerSecondaryViewerWidget.cxx
)


# --------------------------------------------------------------------------
# Wrapping

include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
vtk_wrap_tcl3(SecondaryWindowWithOpenCV SecondaryWindowWithOpenCV_TCL_SRCS "${SecondaryWindowWithOpenCV_SRCS}" "")



#---------------------------------------------------------------------------
# Add Loadable Module support

generatelm(SecondaryWindowWithOpenCV_SRCS SecondaryWindowWithOpenCV.txt)

# --------------------------------------------------------------------------
# Build and install the library

set(lib_name SecondaryWindowWithOpenCV)
add_library(${lib_name}
  ${SecondaryWindowWithOpenCV_SRCS} 
  ${SecondaryWindowWithOpenCV_TCL_SRCS}
  )

slicer3_set_modules_output_path(${lib_name})

target_link_libraries(${lib_name}
  ${Slicer3_Libs_LIBRARIES}
  ${Slicer3_Base_LIBRARIES}
  ${KWWidgets_LIBRARIES}
  ${OpenCV_LIBRARIES}
#  ${OpenIGTLink_LIBRARIES} # A.Yamada 2010-4-23;
  )

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${Slicer3_INSTALL_MODULES_BIN_DIR} COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION ${Slicer3_INSTALL_MODULES_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer3_INSTALL_MODULES_LIB_DIR} COMPONENT Development
  )

slicer3_install_modules(${lib_name})
