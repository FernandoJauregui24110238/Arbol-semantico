QT += core gui widgets
CONFIG += c++17
CONFIG -= debug_and_release
TEMPLATE = app
TARGET = Arbolsemantico
DESTDIR = bin
OBJECTS_DIR = bin/obj
MOC_DIR = bin/moc
RCC_DIR = bin/rcc
UI_DIR = bin/ui
INCLUDEPATH += include
PORTABLE_DIR = dist

SOURCES += \
    src/main.cpp \
    src/AnalizadorLexico.cpp \
    src/GestorAtomos.cpp \
    src/NodoArbol.cpp \
    src/ArbolSemantico.cpp \
    src/MotorInferencia.cpp \
    src/VentanaPrincipal.cpp

HEADERS += \
    include/AnalizadorLexico.hpp \
    include/GestorAtomos.hpp \
    include/NodoArbol.hpp \
    include/ArbolSemantico.hpp \
    include/MotorInferencia.hpp \
    include/VentanaPrincipal.hpp

QMAKE_CXXFLAGS += -Wall -Wextra

QMAKE_EXTRA_TARGETS += portable_package first
portable_package.target = portable_package
portable_package.depends = all
portable_package.commands = set CHERE_INVOKING=1 && set MSYSTEM=UCRT64 && C:/msys64/usr/bin/bash.exe -lc ./tools/package.sh
first.depends += portable_package