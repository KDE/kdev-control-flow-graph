find_package(KDE4 REQUIRED)
find_package(KDevPlatform REQUIRED)

include_directories(
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${KDE4_INCLUDES}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=9518)

set(kdevcontrolflowgraphview_PART_SRCS
    kdevcontrolflowgraphviewplugin.cpp
    controlflowgraphview.cpp
    duchaincontrolflow.cpp
    dotcontrolflowgraph.cpp
)

set(kdevcontrolflowgraphview_PART_UI
    controlflowgraphview.ui
)

kde4_add_ui_files(kdevcontrolflowgraphview_PART_SRCS ${kdevcontrolflowgraphview_PART_UI})
kde4_add_plugin(kdevcontrolflowgraphview ${kdevcontrolflowgraphview_PART_SRCS})
target_link_libraries(kdevcontrolflowgraphview ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} kdevplatforminterfaces kdevplatformlanguage gvc graph cdt)

install(TARGETS kdevcontrolflowgraphview DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES kdevcontrolflowgraphview.desktop DESTINATION ${SERVICES_INSTALL_DIR})
