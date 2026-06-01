//
//  A very simple class for rudimentary scene management
//
//  (c) Georg Umlauf, 2021+2022+2023+2024
//

#include "SceneManager.h"
#include "CameraObject.h"
#include "StereoCameraObject.h"
#include "Hexahedron.h"
#include <iostream>

using enum SceneObjectType;
//
// iterates all objects under its control and has them drawn by the renderer
//
void SceneManager::draw(const RenderCamera& renderer, const QColor& color) const
{

    for (auto obj : *this) if (obj && obj->isVisible()) {
        switch (obj->getType()) {
        case ST_AXES:
            if (m_showAxes) obj->draw(renderer,COLOR_AXES,2.0f);
            break;
        case ST_PLANE:
            obj->draw(renderer,COLOR_PLANE,0.3f);
            break;
        case ST_CUBE:
        case ST_HEXAHEDRON:
            obj->draw(renderer,color,2.0f);
            break;
        case ST_POINT_CLOUD:
            obj->draw(renderer,COLOR_POINT_CLOUD,3.0f);     // last argument unused
            break;
/*         case ST_PERSPECTIVE_CAMERA:
            obj->draw(renderer, COLOR_AXES, 2.0f);
            // Projiziere alle anderen Objekte auf die Bildebene dieser Kamera
            {
                CameraObject* camera = dynamic_cast<CameraObject*>(obj);
                if (camera) {
                    for (auto sceneObj : *this) {
                        if (sceneObj && sceneObj->getType() == ST_PERSPECTIVE_CAMERA){
                            Hexahedron* hex = dynamic_cast<Hexahedron*>(sceneObj);
                            camera->projectHexahedron(hex, renderer);
                        }
                        
                    }
                }
            }
            break; */

        case ST_PERSPECTIVE_CAMERA:
            obj->draw(renderer, COLOR_AXES, 2.0f);
            {
                CameraObject* camera = dynamic_cast<CameraObject*>(obj);
                if (camera) {
                    for (auto sceneObj : *this) {
                        if (sceneObj && sceneObj->getType() != ST_PERSPECTIVE_CAMERA) {
                            Hexahedron* hex = dynamic_cast<Hexahedron*>(sceneObj);
                            if (hex) {
                                camera->projectHexahedron(hex, renderer);
                            }
                        }
                    }
                }
            }
            break;
        case ST_STEREO_CAMERA:
            // TODO: Assignement 2, Part 1 - 3
            // Part 1: This is the place to invoke the stereo camera's projection method and draw the projected objects.
            // Part 2: This is the place to invoke the stereo camera's reconstruction method.
            // Part 3: This is the place to invoke the stereo camera's reconstruction method using misaligned stereo cameras.

            obj->draw(renderer, color, 2.0f); //calls StereoCameraObject::draw()
            //intern wird dann leftCam.draw() und rightCam.draw() aufgerufen
            {
                StereoCameraObject* stereo = dynamic_cast<StereoCameraObject*>(obj);
                if(stereo) {//wenn es dann eine Instanz von der Stereo Camera ist
                    //sammelt alle Hexahedrone in der Szene
                    std::vector<SceneObject*> hexes;
                    for ( auto o : *this)
                        if(o->getType() == ST_HEXAHEDRON) hexes.push_back(o);

                    stereo->projectObjects(hexes, renderer); //erster Teil
                    stereo->reconstructObjects(hexes, renderer); //zweiter Teil

                }
            }
           break;
        default: break;
        }
    }
}
