# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2021 Mathis Brüchert <mbb-mail@gmx.de>

add_executable(marknote main.cpp documenthandler.cpp notesmodel.cpp notebooksmodel.cpp contents/ui/resources.qrc)
target_link_libraries(marknote PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Qml
    Qt::Quick
    Qt::QuickControls2
    Qt::Svg
    KF${QT_MAJOR_VERSION}::I18n
    KF${QT_MAJOR_VERSION}::ConfigCore
    KF${QT_MAJOR_VERSION}::CoreAddons
)

if (NOT ANDROID)
    target_link_libraries(marknote PRIVATE Qt::Widgets)
endif()
install(TARGETS marknote ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
