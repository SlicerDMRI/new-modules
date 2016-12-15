# CMake generated Testfile for 
# Source directory: /home/brain/Sandbox/FiberEndPointFromLabelMap/Testing
# Build directory: /home/brain/Sandbox/FiberEndPointFromLabelMap-build/Testing
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(FiberEndPointFromLabelMapTestVTK "/home/brain/Slicer/Slicer-SuperBuild/Slicer-build/Slicer" "--launcher-no-splash" "--launcher-additional-settings" "/home/brain/Sandbox/FiberEndPointFromLabelMap-build/AdditionalLauncherSettings.ini" "--launch" "/home/brain/Sandbox/FiberEndPointFromLabelMap-build/Testing/FiberEndPointFromLabelMapTest" "ModuleEntryPoint" "/home/brain/Sandbox/FiberEndPointFromLabelMap/Testing/../Data/Input/label_map.nrrd" "/home/brain/Sandbox/FiberEndPointFromLabelMap/Testing/../Data/Input" "/fiberendpointtestingoutput.txt")
set_tests_properties(FiberEndPointFromLabelMapTestVTK PROPERTIES  LABELS "FiberEndPointFromLabelMap")
add_test(FiberEndPointFromLabelMapCompareTxts "/home/brain/Slicer/Slicer-SuperBuild/Slicer-build/Slicer" "--launcher-no-splash" "--launcher-additional-settings" "/home/brain/Sandbox/FiberEndPointFromLabelMap-build/AdditionalLauncherSettings.ini" "--launch" "/home/brain/Sandbox/FiberEndPointFromLabelMap-build/Testing/CompareTxts" "/fiberendpointtestingoutput.txt" "/home/brain/Sandbox/FiberEndPointFromLabelMap/Testing/../Data/Baseline/BaselineResult.txt")
set_tests_properties(FiberEndPointFromLabelMapCompareTxts PROPERTIES  DEPENDS "FiberEndPointFromLabelMapTestVTK" LABELS "FiberEndPointFromLabelMap")
