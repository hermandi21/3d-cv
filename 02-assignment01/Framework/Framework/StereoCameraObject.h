#pragma once

#include "SceneObject.h"
#include "CameraObject.h"
#include <vector>

class StereoCameraObject : public SceneObject {
public:
    StereoCameraObject(QVector4D  ori_left,  QMatrix4x4 world_left,
                       QVector4D  ori_right, QMatrix4x4 world_right,
                       float dx, float dy, float dist,
                       float     focal     = 1.0f,
                       QVector2D principal = QVector2D(0, 0));

    void affineMap(const QMatrix4x4& m) override;
    void draw(const RenderCamera& cam, const QColor& color, float width) const override;

    void projectObjects   (const std::vector<SceneObject*>& objects, const RenderCamera& r) const;
    void reconstructObjects(const std::vector<SceneObject*>& objects, const RenderCamera& r) const;

    void setRecoColor(const QColor& c) { m_recoColor = c; }
    void setMisalignAngle(float degrees);
    float getMisalignAngle() const { return m_misalignAngle; }

private:
    CameraObject leftCam, rightCam;
    QColor       m_recoColor      = COLOR_RECONSTRUCTION;
    QMatrix4x4   m_rightBaseMatrix;   // right camera translation without rotation
    float        m_misalignAngle  = 0.0f;

    QVector3D triangulate(const QVector3D& o1, const QVector3D& d1,
                          const QVector3D& o2, const QVector3D& d2) const;
};
