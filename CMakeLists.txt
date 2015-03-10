project(controlflowgraph)
cmake_minimum_required(VERSION 2.8.12)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH
    ${ECM_MODULE_PATH}
    ${ECM_KDE_MODULE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
)

include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
include(FeatureSummary)

find_package(KDevPlatform REQUIRED)
find_package(GraphViz REQUIRED)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Core Widgets)
find_package(KF5 "5.6.0" REQUIRED COMPONENTS
    I18n
    TextEditor
    ThreadWeaver
    ConfigWidgets
    ItemModels
)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

include_directories(
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

ki18n_wrap_ui(kdevcontrolflowgraphview_PART_SRCS ${kdevcontrolflowgraphview_PART_UI})

kdevplatform_add_plugin(kdevcontrolflowgraphview JSON kdevcontrolflowgraphview.json SOURCES ${kdevcontrolflowgraphview_PART_SRCS})
target_link_libraries(kdevcontrolflowgraphview
    KF5::Parts
    KF5::TextEditor
    KF5::ThreadWeaver
    KDev::Interfaces
    KDev::Language
    KDev::Project
    KDev::Util
    gvc cgraph cdt)

