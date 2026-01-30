#/*****************************************************************************
#* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
#*
#* Author:     Xiao Zhiguo <xiaozhiguo@uniontech.com>
#* Date:       2020-09-09
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

CMAKE_MINIMUM_REQUIRED(VERSION 3.7)

PROJECT(font-manager)

IF (NOT DEFINED VERSION)
    SET(VERSION 1.0.0)
ENDIF()

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_EXTENSIONS OFF)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTORCC ON)
SET(CMAKE_VERBOSE_MAKEFILE ON)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message("Enable build hardening.")

    set(HARDENING_FLAGS "-Wdate-time -D_FORTIFY_SOURCE=2 -g -O2 -ffile-prefix-map=${CMAKE_SOURCE_DIR}=. -fstack-protector-strong -fstack-clash-protection -Wformat -Werror=format-security")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${HARDENING_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${HARDENING_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,relro -Wl,-z,now")
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)

# 确保使用与 Qt 版本一致的 qmake/moc 等工具
if(NOT DEFINED ENV{QT_SELECT})
    set(ENV{QT_SELECT} "qt${QT_VERSION_MAJOR}")
endif()

# 在确定 QT_VERSION_MAJOR 之后，再查找QT具体模块
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Svg Sql Xml DBus Network LinguistTools)

# 根据 Qt 版本决定 DTK 版本
if (QT_VERSION_MAJOR EQUAL 6)
    set(DTK_VERSION 6)
else()
    set(DTK_VERSION "")
endif()

if(DTK_VERSION STREQUAL "")
    set(DTK_MODULE_PREFIX "Dtk")
else()
    set(DTK_MODULE_PREFIX "Dtk${DTK_VERSION}")
endif()
message(STATUS "Searching for ${DTK_MODULE_PREFIX}Widget in CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")

find_package(${DTK_MODULE_PREFIX}Widget QUIET)
if(NOT ${DTK_MODULE_PREFIX}Widget_FOUND)
    message(WARNING "${DTK_MODULE_PREFIX}Widget not found via find_package. Trying PkgConfig fallback.")
    pkg_check_modules(DTK_WIDGET QUIET dtkwidget)
    if(DTK_WIDGET_FOUND)
        include_directories(${DTK_WIDGET_INCLUDE_DIRS})
        link_directories(${DTK_WIDGET_LIBRARY_DIRS})
    else()
        message(FATAL_ERROR "${DTK_MODULE_PREFIX}Widget not found. Install libdtkwidget-dev and check CMAKE_PREFIX_PATH or PKG_CONFIG_PATH.")
    endif()
endif()

message(STATUS "Searching for ${DTK_MODULE_PREFIX}Gui in CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")

find_package(${DTK_MODULE_PREFIX}Gui QUIET)
if(NOT ${DTK_MODULE_PREFIX}Gui_FOUND)
    message(WARNING "${DTK_MODULE_PREFIX}Gui not found via find_package. Trying PkgConfig fallback.")
    pkg_check_modules(DTK_GUI QUIET dtkgui)
    if(DTK_GUI_FOUND)
        include_directories(${DTK_GUI_INCLUDE_DIRS})
        link_directories(${DTK_GUI_LIBRARY_DIRS})
    else()
        message(FATAL_ERROR "${DTK_MODULE_PREFIX}Gui not found. Install libdtkgui-dev and check CMAKE_PREFIX_PATH or PKG_CONFIG_PATH.")
    endif()
endif()

FIND_PACKAGE(Freetype REQUIRED)
FIND_PACKAGE(PkgConfig REQUIRED)

#社区版上 MOC 时，会缺少 include 路径 /usr/include，
#暂未找到原因，先手动添加 /usr/include 规避
INCLUDE_DIRECTORIES(/usr/include/)

PKG_SEARCH_MODULE(FontConfig REQUIRED fontconfig IMPORTED_TARGET)
#PKG_SEARCH_MODULE(QGsettings REQUIRED gsettings-qt IMPORTED_TARGET)

#CMAKE 3.14版本才有的模块
#INCLUDE(FindFontconfig)
include(GNUInstallDirs)
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall ")

#安全编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy")

IF (${CMAKE_SYSTEM_PROCESSOR} MATCHES "sw_64")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
ENDIF()
IF (CMAKE_BUILD_TYPE MATCHES "Debug")
    # Enable Qt builtin debug mode
    ADD_DEFINITIONS("-DQT_MESSAGELOGCONTEXT")
    ADD_SUBDIRECTORY(tests)
ENDIF()

ADD_SUBDIRECTORY(deepin-font-manager)
ADD_SUBDIRECTORY(deepin-font-preview-plugin)


#安装帮助手册需要文件
install(DIRECTORY ${CMAKE_SOURCE_DIR}/deepin-font-manager-assets/deepin-font-manager
   DESTINATION ${CMAKE_INSTALL_DATADIR}/deepin-manual/manual-assets/application/)

#代码覆盖率开关
if(CMAKE_COVERAGE_ARG STREQUAL "CMAKE_COVERAGE_ARG_ON")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -fprofile-arcs -ftest-coverage")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -fprofile-arcs -ftest-coverage")
endif()


