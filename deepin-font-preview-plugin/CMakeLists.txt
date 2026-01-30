#/*****************************************************************************
#* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
#*
#* Author:     Xiao Zhiguo <xiaozhiguo@uniontech.com>
#* Date:       2020-09-11
#*
#* Maintainer: Xiao Zhiguo <xiaozhiguo@uniontech.com>
#*
#* This program is free software: you can redistribute it and/or modify
#* it under the terms of the GNU General Public License as published by
#* the Free Software Foundation, either version 3 of the License, or
#* any later version.
#*
#* This program is distributed in the hope that it will be useful,
#* but WITHOUT ANY WARRANTY; without even the implied warranty of
#* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#* GNU General Public License for more details.
#*
#* You should have received a copy of the GNU General Public License
#* along with this program.  If not, see <http://www.gnu.org/licenses/>.
#*
#*****************************************************************************/

# Find dfm-base based on Qt version (prefer matching version)
# Qt6 -> dfm6-base, Qt5 -> dfm-base
set(DFM_BASE_PACKAGE "")
if (QT_VERSION_MAJOR EQUAL 6)
    find_package(dfm6-base QUIET)
    if (dfm6-base_FOUND)
        set(DFM_BASE_PACKAGE "dfm6-base")
        message(STATUS "Found dfm6-base for Qt6")
    else()
        # Fallback to dfm-base for Qt6
        find_package(dfm-base QUIET)
        if (dfm-base_FOUND)
            set(DFM_BASE_PACKAGE "dfm-base")
            message(STATUS "Found dfm-base (fallback) for Qt6")
        endif()
    endif()
else()
    # Qt5
    find_package(dfm-base QUIET)
    if (dfm-base_FOUND)
        set(DFM_BASE_PACKAGE "dfm-base")
        message(STATUS "Found dfm-base for Qt5")
    else()
        # Fallback to dfm5-base for Qt5 (if exists)
        find_package(dfm5-base QUIET)
        if (dfm5-base_FOUND)
            set(DFM_BASE_PACKAGE "dfm5-base")
            message(STATUS "Found dfm5-base for Qt5")
        endif()
    endif()
endif()

# Try legacy dde-file-manager as last resort
PKG_SEARCH_MODULE(DeepinFileManager dde-file-manager IMPORTED_TARGET)

if (DFM_BASE_PACKAGE)
    add_definitions(-DDFM_BASE)
    message(STATUS "Using ${DFM_BASE_PACKAGE}")
elseif(DeepinFileManager_FOUND)
    message(STATUS "Using legacy dde-file-manager")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

# libdeepin-font-manager
set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../libdeepin-font-manager)
file(GLOB LIB_SRC_FILES ${LIB_DIR}/*.cpp ${LIB_DIR}/*.h)

file(GLOB SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}*.h)

set(APP_PLUGIN deepin-font-preview-plugin)
add_library(${APP_PLUGIN} SHARED "")

target_sources(${APP_PLUGIN} PRIVATE ${SRC_FILES} ${LIB_SRC_FILES})
target_include_directories(${APP_PLUGIN} PRIVATE ${LIB_DIR})
target_include_directories(${APP_PLUGIN} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Add dfm-base include directories if found
if (DFM_BASE_PACKAGE)
    # dfm-base typically provides include directories through imported targets
    # Linking will automatically handle include paths
elseif(DeepinFileManager_FOUND)
    target_include_directories(${APP_PLUGIN} PRIVATE ${DeepinFileManager_INCLUDE_DIRS})
endif()

set(LIBS
    ${FREETYPE_LIBRARIES}
    PkgConfig::FontConfig
    Dtk${DTK_VERSION_MAJOR}::Widget
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Sql
)

# Link dfm-base if found
if (DFM_BASE_PACKAGE)
    list(APPEND LIBS ${DFM_BASE_PACKAGE})
elseif(DeepinFileManager_FOUND)
    list(APPEND LIBS PkgConfig::DeepinFileManager)
endif()

target_link_libraries(${APP_PLUGIN} PRIVATE ${LIBS})

set(PLUGIN_INSTALL_DIR lib/${CMAKE_LIBRARY_ARCHITECTURE}/dde-file-manager/plugins/previews)
install(TARGETS ${APP_PLUGIN} LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR})
