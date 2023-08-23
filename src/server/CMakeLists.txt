project(server C)

set(VERSION "0.1.0")

set(SOURCES 
  main.c
)

set(THREAD_POOL 4 CACHE STRING "the amount of threads in the pool")

# do some ghetto capability detection
execute_process(COMMAND which apt
                RESULT_VARIABLE HAS_APT)

execute_process(COMMAND which rpm
                RESULT_VARIABLE HAS_RPM)

execute_process(COMMAND which apk
                RESULT_VARIABLE HAS_APK)

# make thread pool count a configuration option
add_compile_definitions(THREAD_POOL=${THREAD_POOL})

include_directories(
  "${http-epoll-server_SOURCE_DIR}/src/lib"
  "${http-epoll-server_SOURCE_DIR}/src/include"
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} pthread http-epoll)

set(CMAKE_INSTALL_MODE "REL_SYMLINK")
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)



message("-- CMAKE_SYSTEM_INFO_FILE: ${CMAKE_SYSTEM_INFO_FILE}")
message("-- CMAKE_SYSTEM_NAME:      ${CMAKE_SYSTEM_NAME}")
message("-- CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
message("-- CMAKE_SYSTEM:           ${CMAKE_SYSTEM}")
message("-- CMAKE_OS_NAME:          ${CMAKE_OS_NAME}")
message("-- HAS_APT:                ${HAS_APT}")
message("-- HAS_RPM:                ${HAS_RPM}")
message("-- HAS_APK:                ${HAS_APK}")

set(PKG_GENERATORS 
  "TGZ"
  "STGZ"
)

if(${HAS_APT} EQUAL 0)
  list(APPEND PKG_GENERATORS "DEB")
endif()

if(${HAS_RPM} EQUAL 0)
  list(APPEND PKG_GENERATORS "RPM")
endif()

# create install packages
set(CPACK_PACKAGE_VERSION ${VERSION})
set(CPACK_GENERATOR ${PKG_GENERATORS})
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "John Hardy")
set(CPACK_PACKAGE_VENDOR "johnchardy.com")
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_PACKAGE_DIRECTORY "${http-epoll-server_SOURCE_DIR}/dist/")
include(CPack)




# get_cmake_property(_variableNames VARIABLES)
# list (SORT _variableNames)
# foreach (_variableName ${_variableNames})
#     message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()
