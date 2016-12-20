cmake_minimum_required(VERSION 2.8.9)

#-----------------------------------------------------------------------------
set(MODULE_NAME FiberTractClean)

#-----------------------------------------------------------------------------
SEMMacroBuildCLI(
  NAME ${MODULE_NAME}
  LOGO_HEADER ${Slicer_SOURCE_DIR}/Resources/ITKLogo.h
  TARGET_LIBRARIES SlicerBaseLogic vtkTeem ${VTK_LIBRARIES}
  INCLUDE_DIRECTORIES
    ${vtkITK_INCLUDE_DIRS}
    ${vtkTeem_INCLUDE_DIRS}
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
