# Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

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

FIND_PACKAGE(Qt5 COMPONENTS Core Gui Svg Sql Xml DBus Network REQUIRED)
FIND_PACKAGE(DtkWidget REQUIRED)
FIND_PACKAGE(DtkGui REQUIRED)
FIND_PACKAGE(Freetype REQUIRED)
FIND_PACKAGE(PkgConfig REQUIRED)

#社区版上 MOC 时，会缺少 include 路径 /usr/include，
#暂未找到原因，先手动添加 /usr/include 规避
INCLUDE_DIRECTORIES(/usr/include/)

PKG_SEARCH_MODULE(FontConfig REQUIRED fontconfig IMPORTED_TARGET)
#PKG_SEARCH_MODULE(QGsettings REQUIRED gsettings-qt IMPORTED_TARGET)

#CMAKE 3.14版本才有的模块
#INCLUDE(FindFontconfig)
SET(CMAKE_INSTALL_PREFIX /usr)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall ")

#安全编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong -D_FORTITY_SOURCE=1 -z noexecstack -pie -fPIC -z lazy")

IF (${CMAKE_SYSTEM_PROCESSOR} MATCHES "sw_64")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
ENDIF()
IF (CMAKE_BUILD_TYPE MATCHES "Debug")
    # Enable Qt builtin debug mode
    ADD_DEFINITIONS("-DQT_MESSAGELOGCONTEXT")
ENDIF()
ADD_SUBDIRECTORY(deepin-font-manager)
ADD_SUBDIRECTORY(deepin-font-preview-plugin)

#安装帮助手册需要文件
install(DIRECTORY ${CMAKE_SOURCE_DIR}/deepin-font-manager-assets/deepin-font-manager
   DESTINATION /usr/share/deepin-manual/manual-assets/application/)

#代码覆盖率开关
if(CMAKE_COVERAGE_ARG STREQUAL "CMAKE_COVERAGE_ARG_ON")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -fprofile-arcs -ftest-coverage")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -fprofile-arcs -ftest-coverage")
endif()

#ADD_SUBDIRECTORY(tests)

