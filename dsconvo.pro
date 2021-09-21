QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dsconvoclient.cpp \
    dsconvocommon.cpp \
    dsconvostream.cpp \
    dsconvoconnection.cpp \
    dsconvoserver.cpp \
    clientwindow.cpp \
    serverwindow.cpp \
    main.cpp \
    protobuf/*.pb.cc

HEADERS += \
    dsconvoclient.h \
    dsconvocommon.h \
    dsconvostream.h \
    dsconvoconnection.h \
    dsconvoserver.h \
    clientwindow.h \
    serverwindow.h \
    protobuf/*.pb.h

FORMS += \
    clientwindow.ui \
    serverwindow.ui

TRANSLATIONS += \
    dsconvo_es_CO.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

protobufs.commands = @$(MAKE) -C $$PWD/protobuf -f protobuf.mk

protocopy.commands = $(COPY_DIR) $$PWD/protobuf $$OUT_PWD
protocopy.depends = protobufs

QMAKE_EXTRA_TARGETS += protocopy protobufs
QMAKE_CLEAN += protobuf/*.pb.*
PRE_TARGETDEPS += protocopy

# libprotobuf
LIBS += -lprotobuf
