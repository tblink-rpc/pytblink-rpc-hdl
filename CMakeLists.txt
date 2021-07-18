#****************************************************************************
#* CMakeLists.txt
#*
#* project: tblink-rpc-core
#****************************************************************************

cmake_minimum_required(VERSION 3.11)

project(pytblink-rpc-hdl)

include (ExternalProject)

if (NOT PACKAGES_DIR)
  set(PACKAGES_DIR ${CMAKE_SOURCE_DIR}/packages)
endif()

message("pytblink-rpc-hdl: PACKAGES_DIR=${PACKAGES_DIR}")


if(NOT WIN32)
  add_compile_options(-fPIC)
endif()

set(CMAKE_CXX_STANDARD 11)


foreach(CompilerFlag ${CompilerFlags})
  string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
  string(REPLACE "/MDd" "/MTd" ${CompilerFlag} "${${CompilerFlag}}")
endforeach()

 
 enable_testing()
 
# ExternalProject_Add(tblink-rpc-core
#        PREFIX tblink-rpc-core
#        SOURCE_DIR "${PACKAGES_DIR}/tblink-rpc-core"
#        CMAKE_CACHE_ARGS
#          -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/tblink-rpc-core
#          -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
#          -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
#          -DPACKAGES_DIR:PATH=${PACKAGES_DIR}
#    )
add_subdirectory(${PACKAGES_DIR}/tblink-rpc-core)
   
# ExternalProject_Add(tblink-rpc-hdl
#        PREFIX tblink-rpc-hdl
#        SOURCE_DIR "${PACKAGES_DIR}/tblink-rpc-hdl"
#        CMAKE_CACHE_ARGS
#          -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}/tblink-rpc-hdl
#          -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
#          -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
#          -DPACKAGES_DIR:PATH=${PACKAGES_DIR}
#    )
add_subdirectory(${PACKAGES_DIR}/tblink-rpc-hdl)

file(GLOB pytblink_rpc_hdl_SRC
  src/*.cpp
  src/*.h)
 
add_library(pytblink_rpc_hdl SHARED ${pytblink_rpc_hdl_SRC})

add_dependencies(pytblink_rpc_hdl 
	tblink_rpc_core
	tblink_rpc_hdl
	)

target_include_directories(pytblink_rpc_hdl PRIVATE
	${PACKAGES_DIR}/tblink-rpc-core/cpp/include
	${PACKAGES_DIR}/tblink-rpc-hdl/src
	)

target_link_libraries(pytblink_rpc_hdl 
	-Wl,--whole-archive
	tblink_rpc_hdl
	tblink_rpc_core
	-Wl,--no-whole-archive
	)

target_link_directories(pytblink_rpc_hdl PRIVATE
	${CMAKE_BINARY_DIR}/tblink-rpc-hdl/lib
	${CMAKE_BINARY_DIR}/tblink-rpc-core/lib
	)

install(TARGETS pytblink_rpc_hdl 
	DESTINATION lib
	EXPORT pytblink_rpc_hdl-targets)

