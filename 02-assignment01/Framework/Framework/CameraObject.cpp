#include "CameraObject.h"
#include "RenderCamera.h"
#include <QColor>
#include <iostream>

#if defined(__APPLE__)
#include <OpenGL/gl.h>
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include "./external/freeglut/include/GL/freeglut.h"
#else
#include <GL/gl.h>
#endif

CameraObject::CameraObject(QVector4D _ori, 
    QMatrix4x4 _world, 
    float _dx, 
    float _dy,
    float _dist,
    float _focal,
    QVector2D _principal)
: SceneObject(), dx(_dx), dy(_dy), dist(_dist), focalLength(_focal),
  principalPoint(_principal), worldMatrix(_world)
  {
    type = SceneObjectType::ST_PERSPECTIVE_CAMERA;
    origin = _ori;
    projectionCenter = QVector3D(worldMatrix(0,3), worldMatrix(1,3), worldMatrix(2,3));
    
    // DEBUG: Überprüfe alle Parameter
    std::cout << "=== CameraObject erstellt ===" << std::endl;
    std::cout << "Origin: (" << _ori.x() << ", " << _ori.y() << ", " << _ori.z() << ")" << std::endl;
    std::cout << "Projection Center: (" << projectionCenter.x() << ", " 
              << projectionCenter.y() << ", " << projectionCenter.z() << ")" << std::endl;
    std::cout << "Bildebnen-Größe: " << dx << " x " << dy << std::endl;
    std::cout << "Fokus-Distanz: " << focalLength << std::endl;
    std::cout << "Principal Point: (" << principalPoint.x() << ", " << principalPoint.y() << ")" << std::endl;
    std::cout << "Type: " << (int)type << std::endl;
    std::cout << "=========================" << std::endl;
}

void CameraObject::affineMap(const QMatrix4x4& m) {
    worldMatrix = m * worldMatrix;
}

void CameraObject::draw(const RenderCamera& cam, const QColor& color, float width) const {
    // Berechne die vier Ecken der Bildebene in Kamera-Koordinaten
    float halfWidth = dx / 2.0f;
    float halfHeight = dy / 2.0f;

    float linksOben_x = -halfWidth + principalPoint.x();
    float rechtsOben_x = halfWidth + principalPoint.x();
    float linksOben_y = -halfHeight + principalPoint.y();
    float rechtsUnten_y = halfHeight + principalPoint.y();
    float z = focalLength;

    // Transformiere alle Punkte in Weltkoordinaten
    QVector4D origin_local(0, 0, 0, 1);
    QVector4D lo_local(linksOben_x, linksOben_y, z, 1);
    QVector4D ro_local(rechtsOben_x, linksOben_y, z, 1);
    QVector4D ru_local(rechtsOben_x, rechtsUnten_y, z, 1);
    QVector4D lu_local(linksOben_x, rechtsUnten_y, z, 1);

    QVector3D origin_world = QVector3D(worldMatrix * origin_local);
    QVector3D lo_world = QVector3D(worldMatrix * lo_local);
    QVector3D ro_world = QVector3D(worldMatrix * ro_local);
    QVector3D ru_world = QVector3D(worldMatrix * ru_local);
    QVector3D lu_world = QVector3D(worldMatrix * lu_local);

    // Zeichne die Bildebene (Rechteck)
    cam.renderLine(lo_world, ro_world, color, width);
    cam.renderLine(ro_world, ru_world, color, width);
    cam.renderLine(ru_world, lu_world, color, width);
    cam.renderLine(lu_world, lo_world, color, width);

    // Zeichne Pyramide vom Ursprung zu den 4 Ecken
    cam.renderLine(origin_world, lo_world, color, width);
    cam.renderLine(origin_world, ro_world, color, width);
    cam.renderLine(origin_world, ru_world, color, width);
    cam.renderLine(origin_world, lu_world, color, width);


    // Zeichne einen gelben Punkt für den URSPRUNG des Frustums (nicht die Bildebene!)
    cam.renderPoint(origin_world, QColor(255, 255, 0), 8.0f);  // Gelb beim Kamera-Ursprung
}

//-------PART 3 -------------------

// Transformiere Punkt von Weltkoordinaten zu Kamera-Koordinaten
QVector3D CameraObject::projectPoint(const QVector3D& worldPoint) const {
    // Inverse der worldMatrix um in Kamera-Koordinaten zu transformieren
    QMatrix4x4 inv = worldMatrix.inverted();
    QVector4D camPoint4D = inv * QVector4D(worldPoint, 1.0f);
    return QVector3D(camPoint4D);
}

// Projiziere Punkt von Kamera-Koordinaten auf die Bildebene
QVector2D CameraObject::worldToImageCoordinates(const QVector3D& worldPoint) const {
    // Schritt 1: Transformiere in Kamera-Koordinaten
    QVector3D cameraPoint = projectPoint(worldPoint);
    
    // Schritt 2: Perspektivische Projektion (ähnlich wie Pinhole-Kamera)
    // x_image = f * (X_cam / Z_cam) + H_x
    // y_image = f * (Y_cam / Z_cam) + H_y
    
    if (cameraPoint.z() <= 0) {
        // Punkt liegt hinter der Kamera oder auf der Ebene
        return QVector2D(-1000, -1000);  // Ungültiger Punkt
    }
    
    float x_image = (focalLength * cameraPoint.x() / cameraPoint.z()) + principalPoint.x();
    float y_image = (focalLength * cameraPoint.y() / cameraPoint.z()) + principalPoint.y();
    
    return QVector2D(x_image, y_image);
}

void CameraObject::projectHexahedron(const Hexahedron* hex, const RenderCamera& renderer) const {
    if (!hex) {
        std::cout << "ERROR: Hexahedron ist nullptr!" << std::endl;
        return;
    }
    
    std::cout << "\n=== projectHexahedron START ===" << std::endl;
    std::cout << "Hexahedron hat " << hex->size() << " Punkte" << std::endl;
    
    std::vector<QVector3D> projectedPoints;
    std::vector<bool> pointsValid;
    
    // Projiziere alle 8 Eckpunkte des Hexahedron
    for (size_t i = 0; i < hex->size(); i++) {
        QVector3D worldPoint = (*hex)[i];
        std::cout << "\nPunkt " << i << ": (" << worldPoint.x() << ", " 
                  << worldPoint.y() << ", " << worldPoint.z() << ")" << std::endl;
        
        QVector2D imageCoords = worldToImageCoordinates(worldPoint);
        std::cout << "  Image Coords: (" << imageCoords.x() << ", " 
                  << imageCoords.y() << ")" << std::endl;
        
        float z = 4.0f;  // Z-Position der Plane
        QVector4D projPointLocal(imageCoords.x(), imageCoords.y(), z, 1.0f);
        QVector3D projPointWorld = QVector3D(worldMatrix * projPointLocal);
        
        std::cout << "  Proj World: (" << projPointWorld.x() << ", " 
                  << projPointWorld.y() << ", " << projPointWorld.z() << ")" << std::endl;
        
        projectedPoints.push_back(projPointWorld);
        bool isValid = imageCoords.x() > -999 && imageCoords.y() > -999;
        pointsValid.push_back(isValid);
        std::cout << "  Valid: " << (isValid ? "JA" : "NEIN") << std::endl;
        
        // Zeichne den Punkt in GRÜN
        if (isValid) {
            renderer.renderPoint(projPointWorld, QColor(0, 255, 0), 8.0f);
        }
    }
    
    // Zeichne die Kanten des Hexahedron (grüne Linien)
    for (unsigned e = 0; e < Hexahedron::edgeCount; e++) {
        unsigned v1 = Hexahedron::edgeList[2*e];
        unsigned v2 = Hexahedron::edgeList[2*e + 1];
        
        if (v1 < pointsValid.size() && v2 < pointsValid.size() &&
            pointsValid[v1] && pointsValid[v2]) {
            renderer.renderLine(projectedPoints[v1], projectedPoints[v2], 
                               QColor(0, 255, 0), 2.0f);
        }
    }
    
    std::cout << "=== projectHexahedron END ===" << std::endl;
}