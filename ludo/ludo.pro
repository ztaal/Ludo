#-------------------------------------------------
#
# Project created by QtCreator 2016-03-15T10:40:30
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ludo
TEMPLATE = app

CONFIG   += console
CONFIG   -= app_bundle

unix{
LIBS += -L/usr/lib64
LIBS += -L/usr/local
LIBS += -L/usr
LIBS += -lboost_thread
LIBS += -lboost_system
LIBS += -lboost_iostreams
LIBS += -lboost_serialization
}

SOURCES += main.cpp\
        dialog.cpp \
    game.cpp \
    ludo_player.cpp \
    ludo_player_random.cpp \
    q_player.cpp \
    simple_q_player.cpp \

HEADERS  += dialog.h \
    game.h \
    ludo_player.h \
    q_player.hpp \
    simple_q_player.hpp \
    positions_and_dice.h \
    ludo_player_random.h \

FORMS    += dialog.ui

QMAKE_CXXFLAGS += -std=c++11 -Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic -Wunused
