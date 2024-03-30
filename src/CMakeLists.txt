# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2021 Mathis Brüchert <mbb-mail@gmx.de>

qt_add_qml_module(marknote_static STATIC
    URI org.kde.marknote
    QML_FILES
        qml/Main.qml
        qml/NotesPage.qml
        qml/EditPage.qml
        qml/NotebookMetadataDialog.qml
        qml/WelcomePage.qml
        qml/LinkDialog.qml
        qml/ImageDialog.qml
        qml/components/ActionButton.qml
        qml/components/BottomDrawer.qml
        qml/components/DoubleActionButton.qml
    SOURCES
        documenthandler.cpp
        notesmodel.cpp
        notebooksmodel.cpp
        nestedlisthelper.cpp
        nestedlisthelper_p.h
)

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
)


add_executable(marknote main.cpp)
target_link_libraries(marknote PRIVATE marknote_static marknote_staticplugin)
if (NOT ANDROID)
    target_link_libraries(marknote PRIVATE Qt::Widgets)
endif()

install(TARGETS marknote ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
