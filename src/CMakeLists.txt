set(BUILD_STATIC_OR_SHARED_LIBRARIES "SHARED" CACHE STRING "Build STATIC or SHARED libraries")

add_subdirectory(mcci)
#add_subdirectory(lib)
#add_subdirectory(tools)
#add_subdirectory(sandbox)
#add_subdirectory(thirdparty)
#
#if(BUILD_TESTING)
#  add_subdirectory(test)
#endif()



## latex, for documentation
#find_package(LATEX)
#
#if(LATEX_COMPILER)
#  include(UseLATEX)
#  set(LATEX_OUTPUT_PATH "${project_SHARE_DIR}/doc")
#  add_subdirectory(doc)
#endif()

