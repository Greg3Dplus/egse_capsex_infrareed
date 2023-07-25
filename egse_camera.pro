#-------------------------------------------------
#
# Project created by QtCreator 2018-02-27T10:30:33
#
#-------------------------------------------------

QT       += core gui
CONFIG   += console

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = 3DPLUS_camera_app
TEMPLATE = app

RC_ICONS = camera.ico

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
#        ../lib/QT/SpaceWire/brick.cpp \
#        ../lib/QT/SpaceWire/channel.cpp \
#        ../lib/QT/SpaceWire/device.cpp \
#        ../lib/QT/SpaceWire/link.cpp \
#        ../lib/QT/SpaceWire/message.cpp \
#        ../lib/QT/SpaceWire/mode.cpp \
#        ../lib/QT/SpaceWire/spw_interface.cpp \
    ../lib/QT/INIT_FILE/init_file.cpp \
    CLdialog.cpp \
    aboutwindow.cpp \
    cam_controller.cpp \
    cameralink.cpp \
    customspinbox.cpp \
    imx.cpp \
    imx_read_registers.cpp \
    main.cpp \
    mainwindow.cpp \
    saveBMPthread.cpp \
    saveimagethread.cpp \
    script.cpp \
    streamingthreads.cpp \
    cu_frame.cpp \
    qpixarray.cpp \
#    cmv_read_registers.cpp \
    programwindow.cpp \
    print_data.cpp \
    sendcmddialog.cpp \
    test_mode_configuration.cpp

HEADERS += \
#        ../lib/QT/SpaceWire/spw_interface.h \
        ../lib/QT/SpaceWire/winsdk_compat.h \
    ../lib/QT/INIT_FILE/init_file.h \
    CLdialog.h \
    aboutwindow.h \
    cam_controller.h \
    cameralink.h \
    customspinbox.h \
    imx.h \
    imx_read_registers.h \
    mainwindow.h \
    cmv_registers.h \
    prospect_regs.h \
    rimc_regs.h \
    saveBMPthread.h \
    saveimagethread.h \
    script.h \
    streamingthreads.h \
    mcfmc_registers.h \
    mcsdc_registers.h \
    cu_frame.h \
    qpixarray.h \
#    cmv_read_registers.h \
    programwindow.h \
    print_data.h \
    sendcmddialog.h \
    test_mode_configuration.h

FORMS +=



RESOURCES += \
    res.qrc

#win32: LIBS += -L'C:/Program Files/STAR-Dundee/STAR-System/lib/x86-64/' -lrmap_packet_library
#win32: LIBS += -L'C:/Program Files/STAR-Dundee/STAR-System/lib/x86-64/' -lstar_conf_api_brick_mk2
#win32: LIBS += -L'C:/Program Files/STAR-Dundee/STAR-System/lib/x86-64/' -lstar_conf_api_brick_mk3
#win32: LIBS += -L'C:/Program Files/STAR-Dundee/STAR-System/lib/x86-64/' -lstar_conf_api_mk2
#win32: LIBS += -L'C:/Program Files/STAR-Dundee/STAR-System/lib/x86-64/' -lstar_conf_api_pci_mk2
#win32: LIBS += -L'C:/Program Files/STAR-Dundee/STAR-System/lib/x86-64/' -lstar_conf_api_pxi
#win32: LIBS += -L'C:/Program Files/STAR-Dundee/STAR-System/lib/x86-64/' -lstar_conf_api_router
#win32: LIBS += -L'C:/Program Files/STAR-Dundee/STAR-System/lib/x86-64/' -lstar_conf_api_router_mk2s
#win32: LIBS += -L'C:/Program Files/STAR-Dundee/STAR-System/lib/x86-64/' -lstar_conf_api_spfi_router
#win32: LIBS += -L'C:/Program Files/STAR-Dundee/STAR-System/lib/x86-64/' -lstar_rmap_target
#win32: LIBS += -L'C:/Program Files/STAR-Dundee/STAR-System/lib/x86-64/' -lstar_triggering
#win32: LIBS += -L'C:/Program Files/STAR-Dundee/STAR-System/lib/x86-64/' -lstar-api

win32: LIBS += -L'C:/Program Files (x86)/National Instruments/NI-IMAQ/Lib/MSVC/' -limaq

INCLUDEPATH += 'C:/Program Files (x86)/National Instruments/NI-IMAQ/Include'
DEPENDPATH += 'C:/Program Files (x86)/National Instruments/NI-IMAQ/Include'

#INCLUDEPATH += 'C:/Programmes/STAR-Dundee/STAR-System/apis/cpp_api/inc'
#DEPENDPATH += 'C:/Programmes/STAR-Dundee/STAR-System/apis/cpp_api/inc'

#INCLUDEPATH += 'C:/Program Files/STAR-Dundee/STAR-System/inc/star'
#DEPENDPATH += 'C:/Program Files/STAR-Dundee/STAR-System/inc/star'

#INCLUDEPATH += 'C:/Program Files/STAR-Dundee/STAR-System/lib/x86-64'
#DEPENDPATH += 'C:/Program Files/STAR-Dundee/STAR-System/lib/x86-64'

#INCLUDEPATH += 'C:/Program Files/STAR-Dundee/STAR-System/apis/cpp_api/inc'
#DEPENDPATH += 'C:/Program Files/STAR-Dundee/STAR-System/apis/cpp_api/inc'

#INCLUDEPATH += '../lib/QT/SpaceWire'
#DEPENDPATH += '../lib/QT/SpaceWire'

INCLUDEPATH += '../lib/QT/INIT_FILE'
DEPENDPATH += '../lib/QT/INIT_FILE'
