#include( shared.pri )
QMAKE_CXXFLAGS_DEBUG  += -pg

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
