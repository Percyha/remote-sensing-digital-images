QT       += core gui
QT       += core gui charts
QMAKE_LFLAGS_DEBUG += /INCREMENTAL:NO


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    design.cpp \
    dialog_wait.cpp \
    image_fusion.cpp \
    image_registration.cpp \
    main.cpp \
    mainwindow.cpp \
    mygraphicscene.cpp \
    opencv.cpp \
    operation.cpp \
    saveimage.cpp

HEADERS += \
    design.h \
    dialog_wait.h \
    image_fusion.h \
    image_registration.h \
    mainwindow.h \
    mygraphicscene.h \
    opencv.h \
    operation.h \
    saveimage.h

FORMS += \
    design.ui \
    dialog_wait.ui \
    image_fusion.ui \
    image_registration.ui \
    mainwindow.ui \
    saveimage.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# GDAL库
win32: LIBS += -L$$PWD/../../../VS2022/GDAL/QDAL_Qt/lib/ -lgdal_i

INCLUDEPATH += $$PWD/../../../VS2022/GDAL/QDAL_Qt/include
DEPENDPATH += $$PWD/../../../VS2022/GDAL/QDAL_Qt/include

# qwt库

DEFINES += QT_DLL QWT_DLL

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../Software/Qt/6.7.0/msvc2019_64/lib/ -lqwt
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../Software/Qt/6.7.0/msvc2019_64/lib/ -lqwtd

INCLUDEPATH += $$PWD/../../../Software/Qt/6.7.0/msvc2019_64/include/Qwt
DEPENDPATH += $$PWD/../../../Software/Qt/6.7.0/msvc2019_64/include/Qwt

# opencv库
INCLUDEPATH += F:\Software\opencv\build\include
               F:\Software\opencv\build\include\opencv2

LIBS        += F:\Software\opencv\build\x64\vc16\lib\opencv_world480d.lib
               F:\Software\opencv\build\x64\vc16\bin\*.dll

# Eigen库
INCLUDEPATH += F:\VS2022\Eigen

