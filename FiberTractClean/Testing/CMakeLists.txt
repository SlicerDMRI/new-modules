
#-----------------------------------------------------------------------------
set(BASELINE ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Baseline)
set(TEST_DATA ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input)

set(CLP ${MODULE_NAME})

add_executable(${CLP}Test ${CLP}Test.cxx)
add_dependencies(${CLP}Test ${CLP})
target_link_libraries(${CLP}Test ${CLP}Lib ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES})
set_target_properties(${CLP}Test PROPERTIES LABELS ${CLP})
set_target_properties(${CLP}Test PROPERTIES FOLDER ${${CLP}_TARGETS_FOLDER})

set(testname ${CLP}TestVTK)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint
    --points 2 
    --kernelsize 4 
    --frac 0.9 
    --verbose 
    ${TEST_DATA}/mask.nrrd 
    ${TEST_DATA}/input_partial.vtk
    ${TEMP}/r_kept_fibers.vtk
    ${TEMP}/r_removed_fibers.vtk
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

set(testname ${CLP}TestVTP)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:${CLP}Test>
  ModuleEntryPoint 
    --points 2 
    --kernelsize 4 
    --frac 0.9 
    --verbose 
    ${TEST_DATA}/mask.nrrd 
    ${TEST_DATA}/input_partial.vtp
    ${TEMP}/r_kept_fibers.vtp
    ${TEMP}/r_removed_fibers.vtp
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

add_executable(CompareTensorMeasureClean CompareTensorMeasure.cxx)
target_link_libraries(CompareTensorMeasureClean ${CLP}Lib ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES})
set_target_properties(CompareTensorMeasureClean PROPERTIES LABELS ${CLP})
set_target_properties(CompareTensorMeasureClean PROPERTIES FOLDER ${${CLP}_TARGETS_FOLDER})

set(testname ${CLP}CompareKeptVTKTest)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:CompareTensorMeasureClean>
  ${TEMP}/r_kept_fibers.vtk  ${BASELINE}/baseline_kept_fibers.vtk
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})
set_property(TEST ${testname} PROPERTY DEPENDS ${CLP}TestVTK)

set(testname ${CLP}CompareRemovedVTKTest)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:CompareTensorMeasureClean>
  ${TEMP}/r_removed_fibers.vtk  ${BASELINE}/baseline_removed_fibers.vtk
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})
set_property(TEST ${testname} PROPERTY DEPENDS ${CLP}TestVTK)

set(testname ${CLP}CompareKeptVTPTest)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:CompareTensorMeasureClean>
  ${TEMP}/r_kept_fibers.vtp  ${BASELINE}/baseline_kept_fibers.vtk
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})
set_property(TEST ${testname} PROPERTY DEPENDS ${CLP}TestVTP)

set(testname ${CLP}CompareRemovedVTPTest)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:CompareTensorMeasureClean>
  ${TEMP}/r_removed_fibers.vtp  ${BASELINE}/baseline_removed_fibers.vtk
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})
set_property(TEST ${testname} PROPERTY DEPENDS ${CLP}TestVTP)
