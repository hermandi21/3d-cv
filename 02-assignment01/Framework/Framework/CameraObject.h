#pragma once

#include "SceneObject.h"
#include <QMatrix4x4>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include "Hexahedron.h"

class RenderCamera;
class QColor;

class CameraObject : public SceneObject {
private:
    QVector3D projectionCenter;
    float eulerAngles[3];
    float focalLength;
    QVector2D principalPoint;
    float dx, dy, dist;
    QMatrix4x4 worldMatrix;
    QVector4D origin;
    bool useCustomPlane = false;
    QVector3D customPlanePoint;
    QVector3D customPlaneNormal;

public:
    CameraObject(QVector4D _ori, 
                 QMatrix4x4 _world, 
                 float _dx, 
                 float _dy,
                 float _dist,
                 float _focal = 1.0f,
                 QVector2D _principal = QVector2D(0, 0));
    
    virtual void affineMap(const QMatrix4x4& m) override;
    virtual void draw(const RenderCamera& cam, const QColor& color, float width) const override;

    //fuer den part 3
    QVector3D projectPoint(const QVector3D& worldPoint) const;
    QVector2D worldToImageCoordinates(const QVector3D& worldPoint) const;
    void projectHexahedron(const Hexahedron* hex, const RenderCamera& renderer,
                            const QColor& col = QColor(0, 255, 0)) const;
    void setProjectionPlane(const QVector3D& point, const QVector3D& normal);
    void setWorldMatrix(const QMatrix4x4& m) {
        worldMatrix = m;
        projectionCenter = QVector3D(m(0,3), m(1,3), m(2,3));
    }

    // Getter-Methoden
    QMatrix4x4 getWorldMatrix() const { return worldMatrix; }
    float getFocalLength() const { return focalLength; }
    float getImageWidth() const { return dx; }
    float getImageHeight() const { return dy; }
    float getDist() const {return dist; }
    QVector2D getPrincipalPoint() const {return principalPoint;}
    QVector3D getProjectionCenter() const {return projectionCenter;}
};