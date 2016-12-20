cmake_minimum_required(VERSION 2.8.9)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_HOMEPAGE "http://slicer.org/slicerWiki/index.php/Documentation/Nightly/Extensions/SlicerDMRISandbox")
set(EXTENSION_CATEGORY "Diffusion")
set(EXTENSION_CONTRIBUTORS "Fan Zhang (AnyWare Corp.), Isaiah Norton, Ye Wu, Lauren O'Donnell (Brigham & Women's Hospital)")
set(EXTENSION_DESCRIPTION "New modules created for diffusion MRI tasks in 3D Slicer")
set(EXTENSION_ICONURL "http://www.example.com/Slicer/Extensions/SlicerDMRISandbox.png")
set(EXTENSION_SCREENSHOTURLS "http://www.example.com/Slicer/Extensions/SlicerDMRISandbox/Screenshots/1.png")
set(EXTENSION_DEPENDS "NA") # Specified as a space separated string, a list or 'NA' if any

#-----------------------------------------------------------------------------
# Extension dependencies
find_package(Slicer COMPONENTS ConfigurePrerequisites REQUIRED)
project(SlicerDMRISandbox)
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

#-----------------------------------------------------------------------------

#
# SlicerExecutionModel
#
find_package(SlicerExecutionModel REQUIRED)
include(${SlicerExecutionModel_USE_FILE})

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(FiberTractClean)
add_subdirectory(FiberEndPointFromLabelMap)
## NEXT_MODULE

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_CPACK})
