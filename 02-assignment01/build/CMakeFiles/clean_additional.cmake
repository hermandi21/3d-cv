# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/Framework_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/Framework_autogen.dir/ParseCache.txt"
  "Framework_autogen"
  )
endif()
