cmake_minimum_required(VERSION 3.16)
project(CupHolderMusic VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6  REQUIRED COMPONENTS Core Widgets Sql)
find_package(PkgConfig REQUIRED)
pkg_check_modules(TAGLIB REQUIRED taglib)
pkg_check_modules(LIBVLC REQUIRED libvlc)

set(SOURCES
		main.cpp
		mainwindow.cpp
		settings.cpp
		database.cpp
		views/libraryview.cpp
		views/albumcard.cpp
		popups/albuminfo.cpp
		popups/tags.cpp
		libraryscanner.cpp
		audiobackends/libvlcbackend.cpp
		audiobackends/queuing.cpp
		views/albumview.cpp

)

set(HEADERS
		mainwindow.h
		settings.h
		database.h
		models/album.h
		views/libraryview.h
		views/albumcard.h
		popups/albuminfo.h
		popups/tags.h
		models/track.h
		libraryscanner.h
		audiobackends/audiobackend.h
		audiobackends/libvlcbackend.h
		audiobackends/queuing.h
		views/albumview.h
)

qt_add_executable(cupholdermusic ${SOURCES} ${HEADERS})

target_link_libraries(cupholdermusic PRIVATE
	Qt6::Core Qt6::Widgets Qt6::Sql
	${TAGLIB_LIBRARIES}
	${LIBVLC_LIBRARIES}
)

target_include_directories(cupholdermusic PRIVATE
	src
	${TAGLIB_INCLUDE_DIRS}
	${LIBVLC_INCLUDE_DIRS}
)

install(TARGETS cupholdermusic
		RUNTIME DESTINATION bin)

install(FILES resources/cupholdermusic.desktop
		DESTINATION share/applications)