QT       += core gui widgets

win32: LIBS += -lgdi32 -luser32 -lpsapi

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ProcessComboBox.cpp \
    donatedialog.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    ProcessComboBox.h \
    donatedialog.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    FileSaver_zh_CN.ts
    FileSaver_en.ts

CONFIG += lrelease
CONFIG += embed_translations

RESOURCES += projectResource.qrc
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
