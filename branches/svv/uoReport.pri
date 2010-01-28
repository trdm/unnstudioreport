
#для профилирования. Добавить в *.pro - файл. в инклуде не работает.
#QMAKE_CXXFLAGS_DEBUG  *= -pg
#QMAKE_LFLAGS_DEBUG *= -pg


QT += xml
# добавить в uoReport.pro
#include( shared.pri )

UI_DIR  = Report
UI_HEADERS_DIR  = Report
UI_SOURCES_DIR  = Report
!win32 {
	debug {
		DESTDIR = Debug
		MOC_DIR = Debug
		OBJECTS_DIR = Debug

	}
	release {
		DESTDIR = Release
		MOC_DIR = Release
		OBJECTS_DIR = Release
	}
}
