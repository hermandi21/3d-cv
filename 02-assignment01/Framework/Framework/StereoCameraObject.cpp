#include "StereoCameraObject.h"
#include "RenderCamera.h"
#include "Hexahedron.h"
#include <QColor>
#include <cmath>

// Left camera: cyan, right camera: orange
static const QColor COLOR_LEFT (  0, 200, 255);
static const QColor COLOR_RIGHT(255, 100,   0);

StereoCameraObject::StereoCameraObject(
    QVector4D ori_left,  QMatrix4x4 world_left,
    QVector4D ori_right, QMatrix4x4 world_right,
    float dx, float dy, float dist, float focal, QVector2D principal)
: SceneObject(),
  leftCam (ori_left,  world_left,  dx, dy, dist, focal, principal),
  rightCam(ori_right, world_right, dx, dy, dist, focal, principal),
  m_rightBaseMatrix(world_right)
{
    type = SceneObjectType::ST_STEREO_CAMERA;
}

void StereoCameraObject::setMisalignAngle(float degrees) {
    m_misalignAngle = degrees;
    QMatrix4x4 m = m_rightBaseMatrix;
    m.rotate(degrees, QVector3D(0, 1, 0));
    rightCam.setWorldMatrix(m);
}

void StereoCameraObject::affineMap(const QMatrix4x4& m) {
    leftCam.affineMap(m);
    rightCam.affineMap(m);
}

void StereoCameraObject::draw(const RenderCamera& cam, const QColor& /*color*/, float width) const {
    // Draw each camera frustum in its own colour
    leftCam.draw (cam, COLOR_LEFT,  width);
    rightCam.draw(cam, COLOR_RIGHT, width);

    // Draw the baseline connecting both camera centres
    cam.renderLine(leftCam.getProjectionCenter(),
                   rightCam.getProjectionCenter(),
                   QColor(255, 255, 255), width);
}

// ---- Part 1 ----------------------------------------------------------------

void StereoCameraObject::projectObjects(const std::vector<SceneObject*>& objects,
                                        const RenderCamera& renderer) const {
    for (auto* obj : objects) {
        Hexahedron* hex = dynamic_cast<Hexahedron*>(obj);
        if (hex) {
            leftCam .projectHexahedron(hex, renderer, COLOR_LEFT);
            rightCam.projectHexahedron(hex, renderer, COLOR_RIGHT);
        }
    }
}

// ---- Part 2 ----------------------------------------------------------------

void StereoCameraObject::reconstructObjects(const std::vector<SceneObject*>& objects,
                                            const RenderCamera& renderer) const {
    const QVector3D C1  = leftCam.getProjectionCenter();
    const QVector3D C2  = rightCam.getProjectionCenter();
    const float     f   = leftCam.getFocalLength();
    const QVector2D pp  = leftCam.getPrincipalPoint();

    // Stereo normal case: both cameras look along +Z, baseline along X only.
    // Disparity formula: Z = f * b / d,  where b = baseline,  d = x_L - x_R.
    const float baseline = C2.x() - C1.x();

    for (auto* obj : objects) {
        Hexahedron* hex = dynamic_cast<Hexahedron*>(obj);
        if (!hex) continue;

        std::vector<QVector3D> reconstructed(hex->size());
        std::vector<bool>      valid(hex->size(), false);

        for (size_t i = 0; i < hex->size(); i++) {
            QVector3D worldPt = (*hex)[i];

            // Simulate the image measurements produced by Part 1
            QVector2D img1 = leftCam .worldToImageCoordinates(worldPt);
            QVector2D img2 = rightCam.worldToImageCoordinates(worldPt);

            if (img1.x() < -999.f || img2.x() < -999.f) continue;

            // Normal-case reconstruction via disparity
            float disparity = img1.x() - img2.x();     // d = x_L - x_R
            if (std::abs(disparity) < 1e-8f) continue; // parallel ray → no depth

            float Zdepth = f * baseline / disparity;   // depth along optical axis
            if (Zdepth <= 0.f) continue;               // behind cameras

            float X = C1.x() + (img1.x() - pp.x()) * Zdepth / f;
            float Y = C1.y() + (img1.y() - pp.y()) * Zdepth / f;
            float Z = C1.z() + Zdepth;

            reconstructed[i] = QVector3D(X, Y, Z);
            valid[i] = true;

            renderer.renderPoint(reconstructed[i], m_recoColor, 8.0f);
        }

        // Draw reconstructed edges
        for (unsigned e = 0; e < Hexahedron::edgeCount; e++) {
            unsigned v1 = Hexahedron::edgeList[2*e];
            unsigned v2 = Hexahedron::edgeList[2*e + 1];
            if (valid[v1] && valid[v2])
                renderer.renderLine(reconstructed[v1], reconstructed[v2],
                                    m_recoColor, 2.0f);
        }
    }
}

// ---- triangulation helper --------------------------------------------------

QVector3D StereoCameraObject::triangulate(const QVector3D& o1, const QVector3D& d1,
                                          const QVector3D& o2, const QVector3D& d2) const {
    // Standard closest-approach midpoint for two rays.
    // Minimises |( o1 + t1*d1 ) - ( o2 + t2*d2 )|^2
    QVector3D w0 = o1 - o2;
    float a = QVector3D::dotProduct(d1, d1); // 1 if normalised
    float b = QVector3D::dotProduct(d1, d2);
    float c = QVector3D::dotProduct(d2, d2); // 1 if normalised
    float d = QVector3D::dotProduct(d1, w0);
    float e = QVector3D::dotProduct(d2, w0);

    float denom = a * c - b * b;
    if (std::abs(denom) < 1e-10f)
        return (o1 + o2) * 0.5f; // parallel rays – fall back to midpoint

    float t1 = (b * e - c * d) / denom;
    float t2 = (a * e - b * d) / denom;

    return ((o1 + t1 * d1) + (o2 + t2 * d2)) * 0.5f;
}
