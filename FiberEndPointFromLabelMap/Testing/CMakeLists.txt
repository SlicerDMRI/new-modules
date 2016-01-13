
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
    ${TEST_DATA}/label_map.nrrd
    ${TEST_DATA}
    ${TEMP}/fiberendpointtestingoutput.txt
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})

add_executable(CompareTxts CompareTxts.cxx)
target_link_libraries(CompareTxts ${CLP}Lib ${SlicerExecutionModel_EXTRA_EXECUTABLE_TARGET_LIBRARIES})
set_target_properties(CompareTxts PROPERTIES LABELS ${CLP})
set_target_properties(CompareTxts PROPERTIES FOLDER ${${CLP}_TARGETS_FOLDER})

set(testname ${CLP}CompareTxts)
add_test(NAME ${testname} COMMAND ${SEM_LAUNCH_COMMAND} $<TARGET_FILE:CompareTxts>
  ${TEMP}/fiberendpointtestingoutput.txt  ${BASELINE}/BaselineResult.txt
  )
set_property(TEST ${testname} PROPERTY LABELS ${CLP})
set_property(TEST ${testname} PROPERTY DEPENDS ${CLP}TestVTK)
