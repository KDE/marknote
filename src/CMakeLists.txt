# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2021 Mathis Brüchert <mbb-mail@gmx.de>

qt_add_qml_module(marknote_static STATIC
    URI org.kde.marknote
    QML_FILES
        qml/Main.qml
        qml/MainEditor.qml
        qml/NotesPage.qml
        qml/NoteMetadataDialog.qml
        qml/WelcomePage.qml
        qml/ContextMenu.qml
        qml/SettingsDialog.qml
        qml/KQuickCommandBarPage.qml
        qml/KActionFromAction.qml

        # Notebook
        qml/NotebookContextMenu.qml
        qml/NotebookDelegate.qml
        qml/NotebookDeleteAction.qml
        qml/NotebookMetadataDialog.qml
        qml/NotebookDeleteDialog.qml

        # Editor
        qml/EditPage.qml
        qml/LinkDialog.qml
        qml/ImageDialog.qml
        qml/TableDialog.qml
        qml/SketchDialog.qml

        qml/components/ActionButton.qml
        qml/components/BottomDrawer.qml
        qml/components/DoubleActionButton.qml
        qml/components/RadioSelector.qml
        qml/components/ToolbarBackground.qml
    SOURCES
        documenthandler.cpp
        notesmodel.cpp
        notebooksmodel.cpp
        nestedlisthelper.cpp
        nestedlisthelper_p.h
        navigationcontroller.cpp
        application.cpp
        actionsmodel.cpp
        commandbarfiltermodel.cpp
        config.cpp
)


kconfig_add_kcfg_files(marknote_static GENERATE_MOC marknotesettings.kcfgc)

target_link_libraries(marknote_static PUBLIC
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
    KF6::ColorScheme
    KF6::XmlGui
)

if (TARGET md4c::md4c-html)
    target_link_libraries(marknote_static PUBLIC md4c::md4c-html)
endif()


add_executable(marknote main.cpp)

if(APPLE)
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
if (NOT ANDROID)
    target_link_libraries(marknote PRIVATE Qt::Widgets)
endif()

if (BUILD_TESTING)
    add_subdirectory(autotests)
endif()

install(TARGETS marknote ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
