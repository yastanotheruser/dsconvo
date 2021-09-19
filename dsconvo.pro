QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    clientwindow.cpp \
    dsconvoconnection.cpp \
    dsconvoserver.cpp \
    main.cpp \
    serverwindow.cpp

HEADERS += \
    clientwindow.h \
    dsconvoconnection.h \
    dsconvoserver.h \
    serverwindow.h

FORMS += \
    clientwindow.ui \
    serverwindow.ui

TRANSLATIONS += \
    dsconvo_es_CO.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

protocopy.commands = $(COPY_DIR) -u $$PWD/protobuf $$OUT_PWD

protobufs.commands = @$(MAKE) -C protobuf -f protobuf.mk
protobufs.depends = protocopy

QMAKE_EXTRA_TARGETS += protocopy protobufs
QMAKE_CLEAN += protobuf/*.pb.*
PRE_TARGETDEPS += protobufs

DISTFILES += \
    protobuf/dsconvo.proto \
    protobuf/protobuf.mk
