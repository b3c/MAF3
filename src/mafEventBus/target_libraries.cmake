#
# See CMake/mafMacroGetTargetLibraries.cmake
# 
# This file should list the libraries required to build the current MAF module.
# 

SET(foundation_libraries 

)

SET(target_libraries
    ${foundation_libraries}
    ${MAF_BASE_LIBRARIES}       
)