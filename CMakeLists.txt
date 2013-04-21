### written by Ian Katz ifreecarve@gmail.com
### adapted from lamss-* written by t. schneider tes@mit.edu
### adapted from moos-ivp-local/CMakeLists.txt written by Christopher Gagner

project(lamss C CXX)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")

## set project policies
cmake_minimum_required(VERSION 2.6)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0005 NEW) # read > cmake --help-policy CMP0005
endif(COMMAND cmake_policy)

## turn on testing (> make test)
enable_testing()
include(CTest)

## enable all warnings when building Debug
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall")

option(enable_omap_arm_neon_coprocessor "Enable the NEON FP coprocessor for certain ARM processors" OFF)

if(enable_omap_arm_neon_coprocessor)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mcpu=cortex-a8 -mfloat-abi=softfp -mfpu=neon -ffast-math -fno-math-errno")
endif()

## fetch all the local directories for generated code
get_filename_component(project_LIB_DIR lib ABSOLUTE)
get_filename_component(project_BIN_DIR bin ABSOLUTE)
get_filename_component(project_INC_DIR include ABSOLUTE)
get_filename_component(project_SRC_DIR src ABSOLUTE)
get_filename_component(project_SHARE_DIR share ABSOLUTE)
get_filename_component(project_BUILD_DIR build ABSOLUTE)

## set the cmake defaults for libraries and binaries
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${project_LIB_DIR} CACHE PATH 
  "Output directory for the libraries" )
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${project_BIN_DIR} CACHE PATH
  "Output directory for the binaries" )

## set instructions for `make install`
install(DIRECTORY ${project_BIN_DIR}/ DESTINATION ${CMAKE_INSTALL_PREFIX}/bin 
  FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE )
install(DIRECTORY ${project_LIB_DIR}/ DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(DIRECTORY ${project_SHARE_DIR}/doc/ DESTINATION ${CMAKE_INSTALL_PREFIX}/share/doc/project FILES_MATCHING PATTERN "*.pdf")
install(DIRECTORY ${project_INC_DIR}/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include)


## find dependencies. This should match those given in lamss/DEPENDENCIES

## these dependencies are assumed to be installed to a standard location (e.g. /usr or /usr/local) that is already in the CMake link and include directories

# lua
find_package(Lua51 REQUIRED)
include_directories(${LUA_INCLUDE_DIR})


## now add subdirectories
add_subdirectory(src)
