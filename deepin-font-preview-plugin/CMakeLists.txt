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

find_package(dfm${DTK_VERSION}-base)
PKG_SEARCH_MODULE(DeepinFileManager dde-file-manager IMPORTED_TARGET)
if (dfm${DTK_VERSION}-base_FOUND)
    message("dfm-base founded.")
    add_definitions(-DDFM_BASE)
elseif(DeepinFileManager_FOUND)
    message("dde-file-manager founded.")
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

if (dfm-base_FOUND)
    target_include_directories(${APP_PLUGIN} PRIVATE ${dfm-base_INCLUDE_DIRS})
elseif(DeepinFileManager_FOUND)
    target_include_directories(${APP_PLUGIN} PRIVATE ${DeepinFileManager_INCLUDE_DIRS})
endif()

# 这里直接复用顶层 CMakeLists.txt 里根据 Qt 版本计算出的 DTK_MODULE_PREFIX，
# Qt5:  Dtk::Widget
# Qt6:  Dtk6::Widget
set(LIBS
    ${FREETYPE_LIBRARIES}
    PkgConfig::FontConfig
    ${DTK_MODULE_PREFIX}::Widget
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Sql
)
if (dfm-base_FOUND)
    list(APPEND LIBS dfm-base)
elseif(dde-file-manager_FOUND)
    list(APPEND LIBS PkgConfig::DeepinFileManager)
endif()

target_link_libraries(${APP_PLUGIN} PRIVATE ${LIBS})

set(PLUGIN_INSTALL_DIR lib/${CMAKE_LIBRARY_ARCHITECTURE}/dde-file-manager/plugins/previews)
install(TARGETS ${APP_PLUGIN} LIBRARY DESTINATION ${PLUGIN_INSTALL_DIR})
