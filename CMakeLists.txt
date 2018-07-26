cmake_minimum_required(VERSION 2.8.3)
project(rtt-core-scheduler)

set(CMAKE_BUILD_TYPE Debug)#Release

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-omit-frame-pointer -g -finstrument-functions")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(OROCOS-RTT REQUIRED)
if (NOT OROCOS-RTT_FOUND)
  message (FATAL_ERROR "\nCould not find Orocos-RTT. Please use the shell command\n 'source orocos_toolchain/env.sh' and then run cmake again.")
endif()

include(${OROCOS-RTT_USE_FILE_PATH}/UseOROCOS-RTT.cmake )

# set (Eigen_INCLUDE_DIRS "/usr/include/eigen3")

# find_package(Boost COMPONENTS thread REQUIRED)

find_package(RST-RT REQUIRED)
message(STATUS "RST-RT version: ${RST-RT_VERSION}")
include_directories(BEFORE SYSTEM ${RST-RT_INCLUDE_DIRS})
add_definitions(${RST-RT_CFLAGS})
link_directories(${RST-RT_LIBRARY_DIR} ${RST-RT_LIBRARY_DIRS})

option(DISABLE_INTROSPECTION "Disable introspection features" FALSE)

if (DISABLE_INTROSPECTION)
    message(STATUS "###################################################")
    message(STATUS "### RTT-CORE-EXTENSIONS Introspection disabled! ###")
    message(STATUS "###################################################")
else()
    find_package(RTT-CORE-EXTENSIONS 0.2)
    if (RTT-CORE-EXTENSIONS_FOUND)
        message(STATUS "RTT-CORE-EXTENSIONS_LIBRARIES: ${RTT-CORE-EXTENSIONS_LIBRARIES}")
        message(STATUS "RTT-CORE-EXTENSIONS_LIBRARY_DIRS: ${RTT-CORE-EXTENSIONS_LIBRARY_DIRS}")
        message(STATUS "RTT-CORE-EXTENSIONS_INCLUDE_DIRS: ${RTT-CORE-EXTENSIONS_INCLUDE_DIRS}")
        message(STATUS "##################################################")
        message(STATUS "### RTT-CORE-EXTENSIONS Introspection enabled! ###")
        message(STATUS "##################################################")
        set_property(DIRECTORY ${PROJECT_SOURCE_DIR} APPEND PROPERTY COMPILE_DEFINITIONS USE_INTROSPECTION)
        link_directories(${RTT-CORE-EXTENSIONS_LIBRARY_DIRS})
    else()
        message(STATUS "###################################################")
        message(STATUS "### RTT-CORE-EXTENSIONS Introspection disabled! ###")
        message(STATUS "###################################################")
    endif()
endif()

include_directories(
    include
    src
    # ${Boost_INCLUDE_DIR}
    # ${Eigen_INCLUDE_DIRS}
    ${USE_OROCOS_INCLUDE_DIRS}
    ${RTT-CORE-EXTENSIONS_INCLUDE_DIRS}
    ${RST-RT_INCLUDE_DIRS}
)

orocos_component(${PROJECT_NAME}
  src/CoreScheduler.cpp
  src/BarrierCondition.cpp
  src/BarrierData.cpp
  src/Sender.cpp
  src/Receiver.cpp
)
set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY COMPILE_DEFINITIONS RTT_COMPONENT)

target_link_libraries(${PROJECT_NAME}
   ${USE_OROCOS_LIBRARIES}
   ${OROCOS-RTT_LIBRARIES}
   ${RTT-CORE-EXTENSIONS_LIBRARIES}
  #  ${Boost_LIBRARIES}
   ${RST-RT_LIBRARIES}
)

orocos_service(${PROJECT_NAME}-service
    src/service/CoreSchedulerService.cpp
)

target_link_libraries(${PROJECT_NAME}-service
    ${USE_OROCOS_LIBRARIES}
    ${OROCOS-RTT_LIBRARIES}
    #${RTT-CORE-EXTENSIONS_LIBRARIES}
    #${RST-RT_LIBRARIES}
    # NOT SURE IF THIS IS A GOOD IDEA :D
    #${PROJECT_NAME}
)

orocos_generate_package()
