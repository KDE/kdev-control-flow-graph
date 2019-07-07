cmake_minimum_required(VERSION 3.0)

project(controlflowgraph)

find_package(ECM 1.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH
    ${ECM_MODULE_PATH}
    ${ECM_KDE_MODULE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
)

include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(FeatureSummary)

set(QT_MIN_VERSION "5.5.0")
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Core Widgets)
set(KF5_DEP_VERSION "5.18.0")
find_package(KF5 ${KF5_DEP_VERSION} REQUIRED COMPONENTS
    I18n
    TextEditor
    ThreadWeaver
    ConfigWidgets
    ItemModels
)
find_package(KDevPlatform 5.1.40 )
set_package_properties(KDevPlatform PROPERTIES
    TYPE REQUIRED
)
find_package(GraphViz)
set_package_properties(GraphViz PROPERTIES
    TYPE REQUIRED
)

add_definitions(
    -DQT_DEPRECATED_WARNINGS
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050500
    -DQT_NO_SIGNALS_SLOTS_KEYWORDS
    -DQT_STRICT_ITERATORS
)


include_directories(
    ${GraphViz_INCLUDE_DIRECTORIES}
)

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

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
