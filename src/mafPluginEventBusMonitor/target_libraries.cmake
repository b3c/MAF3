#
# See CMake/mafMacroGetTargetLibraries.cmake
# 
# This file should list the libraries required to build the current MAF module.
# 

SET(foundation_libraries 
  qxmlrpc 
  QtSOAP
  opencv_core
  fervor
)

SET(ExternalLib_LIBRARIES
  
  )

SET(target_libraries
  ${MAF_BASE_LIBRARIES}
  ${foundation_libraries}
  ${ExternalLib_LIBRARIES}
  mafCore
  mafEventBus
  mafResources
  mafGUI
  )
