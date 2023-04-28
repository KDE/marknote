# SPDX-FileCopyrightText: 2021 Mathis Brüchert <mbb-mail@gmx.de>
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.16)

# TODO move to release service
set(RELEASE_SERVICE_VERSION "0.1.0")

project(marknote VERSION ${RELEASE_SERVICE_VERSION})

include(FeatureSummary)

set(QT5_MIN_VERSION 5.12)
set(KF5_MIN_VERSION 5.65)

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(ECMSetupVersion)
include(KDEClangFormat)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(KDEGitCommitHooks)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX MARKNOTE
    VERSION_HEADER ${CMAKE_CURRENT_BINARY_DIR}/marknote-version.h
)

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} REQUIRED COMPONENTS Core Gui Qml QuickControls2 Svg)
find_package(KF${QT_MAJOR_VERSION} ${KF_MIN_VERSION} REQUIRED COMPONENTS Kirigami2 I18n Config CoreAddons)

add_definitions(-DQT_NO_FOREACH -DQT_NO_KEYWORDS)

add_subdirectory(src)

ki18n_install(po)

install(PROGRAMS org.kde.marknote.desktop DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.marknote.metainfo.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES org.kde.marknote.svg DESTINATION ${KDE_INSTALL_FULL_ICONDIR}/hicolor/scalable/apps)

ecm_qt_install_logging_categories(
    EXPORT MARKNOTE
    FILE marknote.categories
    SORT DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES src/*.cpp src/*.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
