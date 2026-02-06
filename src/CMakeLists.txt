# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2021 Mathis Brüchert <mbb-mail@gmx.de>

add_library(marknote_static STATIC)

ecm_add_qml_module(marknote_static
    URI org.kde.marknote
    GENERATE_PLUGIN_SOURCE
    QML_FILES
        qml/Main.qml
        qml/MainEditor.qml
        qml/NotesPage.qml
        qml/NoteMetadataDialog.qml
        qml/WelcomePage.qml
        # Notebook
        qml/NotebookContextMenu.qml
        qml/NotebookDelegate.qml
        qml/NotebookDeleteAction.qml
        qml/NotebookMetadataDialog.qml
        qml/NotebookDeleteDialog.qml
        qml/ImportMaildirDialog.qml
        # Editor
        qml/EditPage.qml
        qml/LinkDialog.qml
        qml/ImageDialog.qml
        qml/TableDialog.qml
        qml/SketchDialog.qml
        qml/TextFieldContextMenu.qml
        # Components
        qml/components/ActionButton.qml
        qml/components/DoubleActionButton.qml
        qml/FormIconDelegate.qml
        # Native menu
        qml/NativeHelpMenu.qml
        qml/NativeWindowMenu.qml
        qml/NativeFileMenu.qml
        qml/NativeEditMenu.qml
        qml/GlobalMenuBar.qml
    SOURCES
        documenthandler.cpp documenthandler.h
        notesmodel.cpp notesmodel.h
        notebooksmodel.cpp notebooksmodel.h
        nestedlisthelper.cpp nestedlisthelper_p.h
        navigationcontroller.cpp navigationcontroller.h
        application.cpp application.h
        tableactionhelper.cpp tableactionhelper.h
        colorschemer.cpp colorschemer.h
        maildirimport.cpp maildirimport.h
        sketchhistory.cpp sketchhistory.h
        tocmodel.cpp tocmodel.h
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/marknote_staticplugin_autogen/include)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/marknote_static_autogen/include)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/settings/settings_autogen/include)

kconfig_target_kcfg_file(marknote_static
    FILE marknotesettings.kcfg
    CLASS_NAME Config
    MUTATORS
    GENERATE_PROPERTIES
    GENERATE_MOC
    DEFAULT_VALUE_GETTERS
    PARENT_IN_CONSTRUCTOR
    QML_REGISTRATION
    SINGLETON
)

add_subdirectory(settings)

target_link_libraries(marknote_static
    PUBLIC
        Qt::Core
        Qt::Gui
        Qt::Qml
        Qt::Quick
        Qt::QuickControls2
        Qt::Svg
        KF6::I18n
        KF6::ConfigCore
        KF6::ConfigGui
        KF6::CoreAddons
        KF6::IconThemes
        KF6::ColorScheme
        KPim6::Mime
        KirigamiAddonsStatefulApp
        settingsplugin
)

if (TARGET KF6::I18nQml)
    target_link_libraries(marknote_static PUBLIC KF6::I18nQml)
endif()

if (TARGET KF6::WindowSystem)
    target_link_libraries(marknote_static PUBLIC KF6::WindowSystem)
    target_compile_definitions(marknote_static PRIVATE HAVE_KWINDOWSYSTEM)
endif()

if(TARGET KF6::Runner)
    target_sources(marknote_static PRIVATE runner.cpp runner.h)
    target_link_libraries(marknote_static PUBLIC KF6::Runner)
    target_compile_definitions(marknote_static PUBLIC HAVE_KRUNNER)
endif()

if (TARGET Qt6::DBus)
    target_link_libraries(marknote_static PUBLIC Qt6::DBus)
endif()

if (TARGET md4c::md4c-html)
    target_link_libraries(marknote_static PUBLIC md4c::md4c-html)
endif()

add_executable(marknote main.cpp)

target_link_libraries(marknote
    PRIVATE
        marknote_static
        marknote_staticplugin
)

if (NOT ANDROID)
    target_link_libraries(marknote PRIVATE Qt::Widgets)
endif()

# Icons
file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../icons/*-apps-org.kde.marknote.png")
ecm_add_app_icon(marknote ICONS
    "${CMAKE_CURRENT_SOURCE_DIR}/../icons/sc-apps-org.kde.marknote.svg"
    ${ICONS_SRCS}
)

# Platform Specific
if (APPLE)
    set_target_properties(marknote PROPERTIES
        MACOSX_BUNDLE_DISPLAY_NAME "Marknote"
        MACOSX_BUNDLE_BUNDLE_NAME "Marknote"
        MACOSX_BUNDLE_LONG_VERSION_STRING "Marknote ${RELEASE_SERVICE_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${RELEASE_SERVICE_VERSION}"
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.marknote"
        MACOSX_BUNDLE_COPYRIGHT "2002-2024 The Marknote Developers")
endif()

target_link_libraries(marknote PRIVATE marknote_static marknote_staticplugin)
if (TARGET KF6::Crash)
   target_link_libraries(marknote PRIVATE KF6::Crash)
endif()
if (NOT ANDROID)
    target_link_libraries(marknote PRIVATE Qt::Widgets)
endif()

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif()

install(TARGETS marknote ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
