# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2021 Mathis Brüchert <mbb-mail@gmx.de>

add_executable(marknote main.cpp documenthandler.cpp notesmodel.cpp notebooksmodel.cpp)

qt_add_qml_module(marknote URI org.kde.marknote NO_RESOURCE_TARGET_PATH
    QML_FILES
        contents/ui/main.qml
        contents/ui/NotesPage.qml
        contents/ui/EditPage.qml
        contents/ui/AddNotebookDialog.qml
        contents/ui/WelcomePage.qml
        contents/ui/components/ActionButton.qml
        contents/ui/components/BottomDrawer.qml
        contents/ui/components/DoubleActionButton.qml
)

target_link_libraries(marknote PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::Quick
    Qt::QuickControls2
    Qt::Svg
    KF6::I18n
    KF6::ConfigCore
    KF6::CoreAddons
)

if (NOT ANDROID)
    target_link_libraries(marknote PRIVATE Qt::Widgets)
endif()
install(TARGETS marknote ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
