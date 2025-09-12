########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(kickcat_COMPONENT_NAMES "")
if(DEFINED kickcat_FIND_DEPENDENCY_NAMES)
  list(APPEND kickcat_FIND_DEPENDENCY_NAMES )
  list(REMOVE_DUPLICATES kickcat_FIND_DEPENDENCY_NAMES)
else()
  set(kickcat_FIND_DEPENDENCY_NAMES )
endif()

########### VARIABLES #######################################################################
#############################################################################################
set(kickcat_PACKAGE_FOLDER_RELEASE "/home/makepluscode/EtherCATSimulator/.conan2/p/b/kickc37623b493c11c/p")
set(kickcat_BUILD_MODULES_PATHS_RELEASE )


set(kickcat_INCLUDE_DIRS_RELEASE "${kickcat_PACKAGE_FOLDER_RELEASE}/include")
set(kickcat_RES_DIRS_RELEASE )
set(kickcat_DEFINITIONS_RELEASE )
set(kickcat_SHARED_LINK_FLAGS_RELEASE )
set(kickcat_EXE_LINK_FLAGS_RELEASE )
set(kickcat_OBJECTS_RELEASE )
set(kickcat_COMPILE_DEFINITIONS_RELEASE )
set(kickcat_COMPILE_OPTIONS_C_RELEASE )
set(kickcat_COMPILE_OPTIONS_CXX_RELEASE )
set(kickcat_LIB_DIRS_RELEASE "${kickcat_PACKAGE_FOLDER_RELEASE}/lib")
set(kickcat_BIN_DIRS_RELEASE )
set(kickcat_LIBRARY_TYPE_RELEASE STATIC)
set(kickcat_IS_HOST_WINDOWS_RELEASE 0)
set(kickcat_LIBS_RELEASE kickcat)
set(kickcat_SYSTEM_LIBS_RELEASE )
set(kickcat_FRAMEWORK_DIRS_RELEASE )
set(kickcat_FRAMEWORKS_RELEASE )
set(kickcat_BUILD_DIRS_RELEASE )
set(kickcat_NO_SONAME_MODE_RELEASE FALSE)


# COMPOUND VARIABLES
set(kickcat_COMPILE_OPTIONS_RELEASE
    "$<$<COMPILE_LANGUAGE:CXX>:${kickcat_COMPILE_OPTIONS_CXX_RELEASE}>"
    "$<$<COMPILE_LANGUAGE:C>:${kickcat_COMPILE_OPTIONS_C_RELEASE}>")
set(kickcat_LINKER_FLAGS_RELEASE
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${kickcat_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${kickcat_SHARED_LINK_FLAGS_RELEASE}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${kickcat_EXE_LINK_FLAGS_RELEASE}>")


set(kickcat_COMPONENTS_RELEASE )