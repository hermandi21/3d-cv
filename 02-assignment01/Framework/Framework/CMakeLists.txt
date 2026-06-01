cmake_minimum_required(VERSION 3.16)

project(Framework VERSION 1.0 LANGUAGES CXX)

# Standard C++ Version setzen
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt Automatisierung einschalten
set(CMAKE_AUTOMOC ON) # Für Q_OBJECT Makros
set(CMAKE_AUTORCC ON) # Für .qrc Ressourcen
set(CMAKE_AUTOUIC ON) # Für .ui Dateien

# Benötigte Qt-Pakete finden
find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets OpenGL)
# Falls du noch Qt5 nutzt, ändere "Qt6" oben zu "Qt5"

# Alle Quelldateien auflisten
set(SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    glwidget.cpp
    glwidget.h
    Axes.cpp
    Axes.h
    Cube.cpp
    Cube.h
    GLConvenience.cpp
    GLConvenience.h
    Hexahedron.cpp
    Hexahedron.h
    Plane.cpp
    Plane.h
    PointCloud.cpp
    PointCloud.h
    QtConvenience.cpp
    QtConvenience.h
    RenderCamera.cpp
    RenderCamera.h
    SceneManager.cpp
    SceneManager.h
    SceneObject.cpp
    SceneObject.h
    resources.qrc
    CameraObject.cpp
    CameraObject.h
    StereoCameraObject.cpp
    StereoCameraObject.h
)

# Executable erstellen
add_executable(Framework ${SOURCES})

# Bibliotheken verlinken
target_link_libraries(Framework PRIVATE 
    Qt6::Widgets 
    Qt6::OpenGLWidgets 
    Qt6::OpenGL
)

# Include-Verzeichnisse für 'external' hinzufügen (falls dort Header liegen)
target_include_directories(Framework PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/external
)