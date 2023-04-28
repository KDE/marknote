# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2021 Mathis Brüchert <mbb-mail@gmx.de>

add_executable(marknote main.cpp documenthandler.cpp notesmodel.cpp notebooksmodel.cpp contents/ui/resources.qrc)
target_link_libraries(marknote
    Qt5::Core
    Qt5::Gui
    Qt5::Qml
    Qt5::Quick
    Qt5::QuickControls2
    Qt5::Svg
    KF5::I18n
    KF5::ConfigCore
    KF5::CoreAddons
)
install(TARGETS marknote ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
