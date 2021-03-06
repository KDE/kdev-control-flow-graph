project(controlflowgraph)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                      ${controlflowgraph_SOURCE_DIR}/cmake/modules
)

find_package(KDE4 REQUIRED)
find_package(KDevPlatform REQUIRED)
find_package(GraphViz REQUIRED)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

include_directories(
    ${KDE4_INCLUDES}
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${GraphViz_INCLUDE_DIRECTORIES}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=9528)

set(kdevcontrolflowgraphview_PART_SRCS
    kdevcontrolflowgraphviewplugin.cpp
    controlflowgraphview.cpp
    duchaincontrolflow.cpp
    dotcontrolflowgraph.cpp
    duchaincontrolflowjob.cpp
    duchaincontrolflowinternaljob.cpp
    controlflowgraphnavigationcontext.cpp
    controlflowgraphnavigationwidget.cpp
    controlflowgraphusescollector.cpp
    controlflowgraphfiledialog.cpp
)

set(kdevcontrolflowgraphview_PART_UI
    controlflowgraphview.ui
    controlflowgraphexportconfiguration.ui
)

kde4_add_ui_files(kdevcontrolflowgraphview_PART_SRCS ${kdevcontrolflowgraphview_PART_UI})
kde4_add_plugin(kdevcontrolflowgraphview ${kdevcontrolflowgraphview_PART_SRCS})
target_link_libraries(kdevcontrolflowgraphview ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KTEXTEDITOR_LIBS} ${KDE4_THREADWEAVER_LIBRARIES} ${KDEVPLATFORM_INTERFACES_LIBRARIES} ${KDEVPLATFORM_LANGUAGE_LIBRARIES} ${KDEVPLATFORM_PROJECT_LIBRARIES} ${KDEVPLATFORM_UTIL_LIBRARIES} gvc cgraph cdt)

install(TARGETS kdevcontrolflowgraphview DESTINATION ${PLUGIN_INSTALL_DIR})

configure_file(kdevcontrolflowgraphview.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevcontrolflowgraphview.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevcontrolflowgraphview.desktop DESTINATION ${SERVICES_INSTALL_DIR})
