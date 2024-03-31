
#include "threepp/controls/TransformControls.hpp"

#include "threepp/cameras/Camera.hpp"
#include "threepp/objects/Line.hpp"
#include "threepp/objects/Mesh.hpp"

#include "threepp/materials/LineBasicMaterial.hpp"
#include "threepp/materials/MeshBasicMaterial.hpp"

#include "threepp/geometries/CylinderGeometry.hpp"
#include "threepp/geometries/SphereGeometry.hpp"
#include "threepp/geometries/TorusGeometry.hpp"

#include "threepp/geometries/BoxGeometry.hpp"
#include "threepp/geometries/PlaneGeometry.hpp"

#include "threepp/core/Raycaster.hpp"

#include "threepp/geometries/OctahedronGeometry.hpp"
#include "threepp/input/PeripheralsEventSource.hpp"

#include <cmath>
#include <iostream>
#include <tuple>

using namespace threepp;

namespace {

    Raycaster _raycaster;

    Vector3 _tempVector;
    Vector3 _tempVector2;
    Vector3 _zeroVector;

    Euler _tempEuler;

    Quaternion _identityQuaternion;
    Quaternion _tempQuaternion;
    Quaternion _tempQuaternion2;

    Matrix4 _tempMatrix;
    Matrix4 _lookAtMatrix;

    std::unordered_map<std::string, Vector3> _unit{
            {"X", Vector3{1, 0, 0}},
            {"Y", Vector3{}},
            {"Z", Vector3{}}};

    Vector3 _v1, _v2, _v3;
    Vector3 _unitX = Vector3(1, 0, 0), _unitY = Vector3(0, 1, 0), _unitZ = Vector3(0, 0, 1);
    Vector3 _dirVector, _alignVector;

    Event _changeEvent{"change"};
    Event _mouseDownEvent{"mouseDown"};
    Event _mouseUpEvent{"mouseUp"};
    Event _objectChangeEvent{"objectChange"};

    using GizmoMap = std::unordered_map<std::string, std::vector<std::tuple<std::shared_ptr<Object3D>, std::optional<Vector3>, std::optional<Euler>, std::optional<Vector3>, std::optional<std::string>>>>;

    std::shared_ptr<BufferGeometry> CircleGeometry(float radius, float arc) {

        auto geometry = BufferGeometry::create();
        std::vector<float> vertices;

        for (unsigned i = 0; i <= 64 * arc; ++i) {

            vertices.emplace_back(0);
            vertices.emplace_back(std::cos(static_cast<float>(i) / 32 * math::PI) * radius);
            vertices.emplace_back(std::sin(static_cast<float>(i) / 32 * math::PI) * radius);
        }

        geometry->setAttribute("position", FloatBufferAttribute::create(vertices, 3));

        return geometry;
    }

    std::shared_ptr<BufferGeometry> TranslateHelperGeometry() {

        auto geometry = BufferGeometry::create();

        geometry->setAttribute("position", FloatBufferAttribute::create(std::vector<float>{0, 0, 0, 1, 1, 1}, 3));

        return geometry;
    }

    struct GizmoObject: Object3D {

        std::optional<float> _opacity;
        std::optional<Color> _color;
    };


    //    template<class T>
    //    struct Property {
    //
    //        Property(const std::string& propName, T defaultValue)
    //            : defaultValue(defaultValue), propName(propName) {}
    //
    //        operator T() const {
    //
    //            return propValue.value_or(defaultValue);
    //        }
    //
    //        void operator[](T value) {
    //
    //            if (propValue != value) {
    //
    //                propValue = value;
    //            }
    //        }
    //
    //    private:
    //        T defaultValue;
    //        std::string propName;
    //        std::optional<T> propValue;
    //    };


}// namespace

struct TransformControlsGizmo: Object3D {

    std::unordered_map<std::string, GizmoObject*> gizmo;
    std::unordered_map<std::string, GizmoObject*> picker;
    std::unordered_map<std::string, GizmoObject*> helper;

    TransformControlsGizmo() {

        auto gizmoMaterial = MeshBasicMaterial::create();
        gizmoMaterial->depthTest = false;
        gizmoMaterial->depthWrite = false;
        gizmoMaterial->transparent = true;
        gizmoMaterial->side = Side::Double;
        gizmoMaterial->fog = false;
        gizmoMaterial->toneMapped = false;

        auto gizmoLineMaterial = LineBasicMaterial::create();
        gizmoLineMaterial->depthTest = false;
        gizmoLineMaterial->depthWrite = false;
        gizmoLineMaterial->transparent = true;
        gizmoMaterial->fog = false;
        gizmoMaterial->toneMapped = false;

        // Make unique material for each axis/color

        const auto matInvisible = gizmoMaterial->clone();
        matInvisible->opacity = 0.15f;

        const auto matHelper = gizmoMaterial->clone();
        matHelper->opacity = 0.33f;

        const auto matRed = gizmoMaterial->clone()->as_shared<MeshBasicMaterial>();
        matRed->color.setHex(0xff0000);

        const auto matGreen = gizmoMaterial->clone()->as_shared<MeshBasicMaterial>();
        matGreen->color.setHex(0x00ff00);

        const auto matBlue = gizmoMaterial->clone()->as_shared<MeshBasicMaterial>();
        matBlue->color.setHex(0x0000ff);

        const auto matWhiteTransparent = gizmoMaterial->clone()->as_shared<MeshBasicMaterial>();
        matWhiteTransparent->opacity = 0.25f;

        const auto matYellowTransparent = matWhiteTransparent->clone()->as_shared<MeshBasicMaterial>();
        matYellowTransparent->color.setHex(0xffff00);

        const auto matCyanTransparent = matWhiteTransparent->clone()->as_shared<MeshBasicMaterial>();
        matCyanTransparent->color.setHex(0x00ffff);

        const auto matMagentaTransparent = matWhiteTransparent->clone()->as_shared<MeshBasicMaterial>();
        matMagentaTransparent->color.setHex(0xff00ff);

        const auto matYellow = gizmoMaterial->clone()->as_shared<MeshBasicMaterial>();
        matYellow->color.setHex(0xffff00);

        const auto matLineRed = gizmoLineMaterial->clone()->as_shared<LineBasicMaterial>();
        matLineRed->color.setHex(0xff0000);

        const auto matLineGreen = gizmoLineMaterial->clone()->as_shared<LineBasicMaterial>();
        matLineGreen->color.setHex(0x00ff00);

        const auto matLineBlue = gizmoLineMaterial->clone()->as_shared<LineBasicMaterial>();
        matLineBlue->color.setHex(0x0000ff);

        const auto matLineCyan = gizmoLineMaterial->clone()->as_shared<LineBasicMaterial>();
        matLineCyan->color.setHex(0x00ffff);

        const auto matLineMagenta = gizmoLineMaterial->clone()->as_shared<LineBasicMaterial>();
        matLineMagenta->color.setHex(0xff00ff);

        const auto matLineYellow = gizmoLineMaterial->clone()->as_shared<LineBasicMaterial>();
        matLineYellow->color.setHex(0xffff00);

        const auto matLineGray = gizmoLineMaterial->clone()->as_shared<LineBasicMaterial>();
        matLineGray->color.setHex(0x787878);

        const auto matLineYellowTransparent = matLineYellow->clone();
        matLineYellowTransparent->opacity = 0.25f;

        // reusable geometry

        const auto arrowGeometry = CylinderGeometry::create(0, 0.05, 0.2, 12, 1, false);

        const auto scaleHandleGeometry = BoxGeometry::create(0.125, 0.125, 0.125);

        const auto lineGeometry = BufferGeometry::create();
        lineGeometry->setAttribute("position", FloatBufferAttribute::create(std::vector<float>{0, 0, 0, 1, 0, 0}, 3));

        // Gizmo definitions - custom hierarchy definitions for setupGizmo() function

        // clang-format off
        GizmoMap gizmoTranslate {
                {"X", {
                              {Mesh::create(arrowGeometry, matRed), Vector3{1,0,0}, Euler{0,0,-math::PI/2}, std::nullopt, "fwd"},
                              {Mesh::create(arrowGeometry, matRed), Vector3{1,0,0}, Euler{0,0,math::PI/2}, std::nullopt, "bwd"},
                              {Line::create(lineGeometry, matLineRed), std::nullopt, std::nullopt, std::nullopt, std::nullopt}
                      }},
                {"Y", {
                              {Mesh::create(arrowGeometry, matGreen), Vector3{0,1,0}, std::nullopt, std::nullopt, "fwd"},
                              {Mesh::create(arrowGeometry, matGreen), Vector3{0,1,0}, Euler{math::PI, 0, 0}, std::nullopt, "bwd"},
                              {Line::create(lineGeometry, matLineGreen), std::nullopt, Euler{0,0,math::PI/2}, std::nullopt, std::nullopt}
                      }},
                {"Z", {
                              {Mesh::create(arrowGeometry, matBlue), Vector3{0,0,1}, Euler{math::PI/2, 0,0}, std::nullopt, "fwd"},
                              {Mesh::create(arrowGeometry, matBlue), Vector3{0,0,1}, Euler{-math::PI/2, 0, 0}, std::nullopt, "bwd"},
                              {Line::create(lineGeometry, matLineBlue), std::nullopt, Euler{0,-math::PI/2,0}, std::nullopt, std::nullopt}
                      }},
                {"XYZ", {
                             {Mesh::create(OctahedronGeometry::create(0.1, 0), matWhiteTransparent->clone()), Vector3{0,0,0}, Euler{0,0,0}, std::nullopt, std::nullopt}
                        }},
                {"XY", {
                             {Mesh::create(PlaneGeometry::create(0.295, 0.295), matYellowTransparent->clone()), Vector3{0.15,0.15,0}, std::nullopt, std::nullopt, std::nullopt},
                             {Line::create(lineGeometry, matLineYellow), Vector3{0.18, 0.3, 0}, std::nullopt, Vector3{0.125, 1, 1}, std::nullopt},
                             {Line::create(lineGeometry, matLineYellow), Vector3{0.3, 0.18, 0}, Euler{0, 0, math::PI/2}, Vector3{0.125, 1, 1}, std::nullopt}
                        }},
                {"YZ", {
                            {Mesh::create(PlaneGeometry::create(0.295, 0.295), matCyanTransparent->clone()), Vector3{0, 0.15,0.15}, Euler{0, math::PI/2, 0}, std::nullopt, std::nullopt},
                            {Line::create(lineGeometry, matLineCyan), Vector3{0, 0.18, 0.3}, std::nullopt, Vector3{0.125, 1, 1}, std::nullopt},
                            {Line::create(lineGeometry, matLineCyan), Vector3{0, 0.3, 0.18}, Euler{0, -math::PI/2, 0}, Vector3{0.125, 1, 1}, std::nullopt}
                       }},
                {"XZ", {
                            {Mesh::create(PlaneGeometry::create(0.295, 0.295), matMagentaTransparent->clone()), Vector3{0.15,0,0.15}, Euler{-math::PI/2, 0, 0}, std::nullopt, std::nullopt},
                            {Line::create(lineGeometry, matLineMagenta), Vector3{0.18, 0, 0.3}, std::nullopt, Vector3{0.125, 1, 1}, std::nullopt},
                            {Line::create(lineGeometry, matLineMagenta), Vector3{0.3, 0, 0.18}, Euler{0, -math::PI/2, 0}, Vector3{0.125, 1, 1}, std::nullopt}
                       }}
        };

        GizmoMap pickerTranslate {
                {"X", {
                            {Mesh::create(CylinderGeometry::create(0.2, 0, 1, 4, 1, false), matInvisible), Vector3{0.6, 0, 0}, Euler{0, 0, -math::PI/2}, std::nullopt, std::nullopt}
                      }},
                {"Y", {
                            {Mesh::create(CylinderGeometry::create(0.2, 0, 1, 4, 1, false), matInvisible), Vector3{0, 0.6, 0}, std::nullopt, std::nullopt, std::nullopt}
                      }},
                {"Z", {
                            {Mesh::create(CylinderGeometry::create(0.2, 0, 1, 4, 1, false), matInvisible), Vector3{0, 0, 0.6}, Euler{math::PI/2, 0, 0}, std::nullopt, std::nullopt}
                      }},
                {"XYZ", {
                            {Mesh::create(OctahedronGeometry::create(0.2, 0), matInvisible), std::nullopt, std::nullopt, std::nullopt, std::nullopt}
                      }},
                {"XY", {
                            {Mesh::create(PlaneGeometry::create(0.4, 0.4), matInvisible), Vector3{0.2, 0.2, 0}, std::nullopt, std::nullopt, std::nullopt}
                      }},
                {"YZ", {
                           {Mesh::create(PlaneGeometry::create(0.4, 0.4), matInvisible), Vector3{0, 0.2, 0.2}, Euler{0, math::PI/2, 0}, std::nullopt, std::nullopt}
                     }},
                {"XZ", {
                           {Mesh::create(PlaneGeometry::create(0.4, 0.4), matInvisible), Vector3{0.2, 0, 0.2}, Euler{-math::PI/2, 0, 0}, std::nullopt, std::nullopt}
                     }}
        };

        GizmoMap helperTranslate {
                {"START", {
                            {Mesh::create(OctahedronGeometry::create(0.01, 2), matHelper), std::nullopt, std::nullopt, std::nullopt, "helper"}
                      }},
                {"END", {
                            {Mesh::create(OctahedronGeometry::create(0.01, 2), matHelper), std::nullopt, std::nullopt, std::nullopt, "helper"}
                      }},
                {"DELTA", {
                            {Line::create(TranslateHelperGeometry(), matHelper), std::nullopt, std::nullopt, std::nullopt, "helper"}
                      }},
                {"X", {
                            {Line::create(lineGeometry, matHelper->clone()), Vector3{-1e3, 0, 0}, std::nullopt, Vector3{1e6, 1, 1}, "helper"}
                      }},
                {"Y", {
                            {Line::create(lineGeometry, matHelper->clone()), Vector3{0, -1e3, 0}, Euler{0, 0, math::PI/2}, Vector3{1e6, 1, 1}, "helper"}
                      }},
                {"Z", {
                           {Line::create(lineGeometry, matHelper->clone()), Vector3{0, 0, -1e3}, Euler{0, -math::PI/2, 0}, Vector3{1e6, 1, 1}, "helper"}
                     }}
                };

        GizmoMap gizmoRotate {
                {"X", {
                            {Line::create(CircleGeometry(1, 0.5), matLineRed), std::nullopt, std::nullopt, std::nullopt, std::nullopt},
                            {Mesh::create(OctahedronGeometry::create(0.04, 0), matRed), Vector3{0, 0, 0.99}, std::nullopt, Vector3{1, 3, 1}, std::nullopt}
                      }},
                {"Y", {
                            {Line::create(CircleGeometry(1, 0.5), matLineGreen), std::nullopt, Euler{0, 0, -math::PI/2}, std::nullopt, std::nullopt},
                            {Mesh::create(OctahedronGeometry::create(0.04, 0), matGreen), Vector3{0, 0, 0.99}, std::nullopt, Vector3{3, 1, 1}, std::nullopt}
                      }},
                {"Z", {
                            {Line::create(TranslateHelperGeometry(), matLineBlue), std::nullopt, Euler{0, math::PI/2, 0}, std::nullopt, std::nullopt},
                            {Mesh::create(OctahedronGeometry::create(0.04, 0), matBlue), Vector3{0.99, 0, 0}, std::nullopt, Vector3{1, 3, 1}, std::nullopt},
                      }},
                {"E", {
                            {Line::create(CircleGeometry(1.25, 1), matLineYellowTransparent), std::nullopt, Euler{0, math::PI/2, 0}, std::nullopt, std::nullopt},
                            {Mesh::create(CylinderGeometry::create(0.03, 0, 0.15, 4, 1, false), matLineYellowTransparent), Vector3{1.17, 0, 0}, Euler{0, 0, -math::PI/2}, Vector3{1, 1, 0.001}, std::nullopt},
                            {Mesh::create(CylinderGeometry::create(0.03, 0, 0.15, 4, 1, false), matLineYellowTransparent), Vector3{-1.17, 0, 0}, Euler{0, 0, math::PI/2}, Vector3{1, 1, 0.001}, std::nullopt},
                            {Mesh::create(CylinderGeometry::create(0.03, 0, 0.15, 4, 1, false), matLineYellowTransparent), Vector3{0, -1.17, 0}, Euler{math::PI, 0, 0}, Vector3{1, 1, 0.001}, std::nullopt},
                            {Mesh::create(CylinderGeometry::create(0.03, 0, 0.15, 4, 1, false), matLineYellowTransparent), Vector3{0, 1.17, 0}, Euler{0, 0, 0}, Vector3{1, 1, 0.001}, std::nullopt},
                      }},
                {"XYZE", {
                            {Line::create(CircleGeometry(1, 1), matLineGray), std::nullopt, Euler{0, math::PI/2, 0}, std::nullopt, std::nullopt}
                      }}
                };

        GizmoMap helperRotate {
                {"AXIS", {
                            {Line::create(lineGeometry, matHelper->clone()), Vector3{-1e3, 0, 0}, std::nullopt, Vector3{1e6, 1, 1}, std::nullopt}
                     }}
                };

        GizmoMap pickerRotate {
                {"X", {
                            {Mesh::create(TorusGeometry::create(1, 0.1, 4, 24), matInvisible), Vector3{0, 0, 0}, Euler{0, -math::PI/2, -math::PI/2}, std::nullopt, std::nullopt}
                      }},
                {"Y", {
                            {Mesh::create(TorusGeometry::create(1, 0.1, 4, 24), matInvisible), Vector3{0, 0, 0}, Euler{math::PI/2, 0, 0}, std::nullopt, std::nullopt}
                      }},
                {"Z", {
                            {Mesh::create(TorusGeometry::create(1, 0.1, 4, 24), matInvisible), Vector3{0, 0, 0}, Euler{0, 0, -math::PI/2}, std::nullopt, std::nullopt},
                      }},
                {"E", {
                            {Mesh::create(TorusGeometry::create(1.25, 0.1, 2, 24), matInvisible), std::nullopt, std::nullopt, std::nullopt, std::nullopt},
                      }},
                {"XYZE", {
                            {Mesh::create(SphereGeometry::create(0.7, 10, 8), matInvisible), std::nullopt, std::nullopt, std::nullopt, std::nullopt}
                      }}
                };

        GizmoMap gizmoScale {
                {"X", {
                            {Mesh::create(scaleHandleGeometry, matRed), Vector3{0.8, 0, 0}, Euler{0, 0, -math::PI/2}, std::nullopt, std::nullopt},
                            {Line::create(lineGeometry, matLineRed), std::nullopt, std::nullopt, Vector3{0.8, 1, 1}, std::nullopt}
                      }},
                {"Y", {
                            {Mesh::create(scaleHandleGeometry, matGreen), Vector3{0, 0.6, 0}, std::nullopt, std::nullopt, std::nullopt},
                            {Line::create(lineGeometry, matLineGreen), std::nullopt, Euler{0, 0, math::PI/2}, Vector3{0.8, 1, 1}, std::nullopt}
                      }},
                {"Z", {
                            {Mesh::create(scaleHandleGeometry, matBlue), Vector3{0, 0, 0.6}, Euler{math::PI/2, 0, 0}, std::nullopt, std::nullopt},
                            {Line::create(lineGeometry, matLineGreen), std::nullopt, Euler{0, -math::PI/2, 0}, Vector3{0.8, 1, 1}, std::nullopt}
                      }},
                {"XY", {
                            {Mesh::create(scaleHandleGeometry, matYellowTransparent), Vector3{0.85, 0.85, 0}, std::nullopt, Vector3{2, 2, 0.2}, std::nullopt},
                            {Line::create(lineGeometry, matLineYellow), Vector3{0.855, 0.98, 0}, std::nullopt, Vector3{0.125, 1, 1}, std::nullopt},
                            {Line::create(lineGeometry, matLineYellow), Vector3{0.98, 0.855, 0}, Euler{0, 0, math::PI/2}, Vector3{0.125, 1, 1}, std::nullopt}
                      }},
                {"YZ", {
                            {Mesh::create(scaleHandleGeometry, matMagentaTransparent), Vector3{0, 0.85, 0.85}, std::nullopt, Vector3{0.2, 2, 2}, std::nullopt},
                            {Line::create(lineGeometry, matLineMagenta), Vector3{0, 0.855, 0.98}, Euler{0, 0, math::PI/2}, Vector3{0.125, 1, 1}, std::nullopt},
                            {Line::create(lineGeometry, matLineMagenta), Vector3{0, 0.98, 0.855}, Euler{0, -math::PI/2, 0}, Vector3{0.125, 1, 1}, std::nullopt}
                      }},
                {"XZ", {
                           {Mesh::create(scaleHandleGeometry, matMagentaTransparent), Vector3{0.85, 0, 0.85}, std::nullopt, Vector3{2, 0.2, 2}, std::nullopt},
                           {Line::create(lineGeometry, matLineMagenta), Vector3{0.855, 0, 0.98}, Euler{0, 0, math::PI/2}, Vector3{0.125, 1, 1}, std::nullopt},
                           {Line::create(lineGeometry, matLineMagenta), Vector3{0.98, 0, 0.855}, Euler{0, -math::PI/2, 0}, Vector3{0.125, 1, 1}, std::nullopt}
                     }},
                {"XYZX", {
                           {Mesh::create(BoxGeometry::create(0.125, 0.125, 0.125), matWhiteTransparent->clone()), Vector3{1.1, 0, 0}, std::nullopt, std::nullopt, std::nullopt}
                     }},
                {"XYZY", {
                           {Mesh::create(BoxGeometry::create(0.125, 0.125, 0.125), matWhiteTransparent->clone()), Vector3{0, 1.1, 0}, std::nullopt, std::nullopt, std::nullopt}
                     }},
                {"XYZZ", {
                           {Mesh::create(BoxGeometry::create(0.125, 0.125, 0.125), matWhiteTransparent->clone()), Vector3{0, 0, 1.1}, std::nullopt, std::nullopt, std::nullopt}
                     }}
        };

        GizmoMap pickerScale {
                {"X", {
                            {Mesh::create(CylinderGeometry::create(0.2, 0, 0.8, 4, 1, false), matInvisible), Vector3{0.5, 0, 0}, Euler{0, 0, -math::PI/2}, std::nullopt, std::nullopt}
                      }},
                {"Y", {
                            {Mesh::create(CylinderGeometry::create(0.2, 0, 0.8, 4, 1, false), matInvisible), Vector3{0, 0.5, 0}, std::nullopt, std::nullopt, std::nullopt}
                      }},
                {"Z", {
                            {Mesh::create(CylinderGeometry::create(0.2, 0, 0.8, 4, 1, false), matInvisible), Vector3{0, 0, 0.5}, Euler{math::PI/2, 0, 0}, std::nullopt, std::nullopt}
                      }},
                {"XY", {
                            {Mesh::create(scaleHandleGeometry, matInvisible), Vector3{0.85, 0.85, 0}, std::nullopt, Vector3{3, 3, 0.2}, std::nullopt}
                      }},
                {"YZ", {
                            {Mesh::create(scaleHandleGeometry, matInvisible), Vector3{0, 0.85, 0.85}, std::nullopt, Vector3{0.2, 3, 3}, std::nullopt}
                      }},
                {"XZ", {
                           {Mesh::create(scaleHandleGeometry, matInvisible), Vector3{0.85, 0, 0.85}, std::nullopt, Vector3{3, 0.2, 3}, std::nullopt}
                     }},
                {"XYZX", {
                           {Mesh::create(BoxGeometry::create(0.2, 0.2, 0.2), matInvisible), Vector3{1.1, 0, 0}, std::nullopt, std::nullopt, std::nullopt}
                     }},
                {"XYZY", {
                           {Mesh::create(BoxGeometry::create(0.2, 0.2, 0.2), matInvisible), Vector3{0, 1.1, 0}, std::nullopt, std::nullopt, std::nullopt}
                     }},
                {"XYZZ", {
                           {Mesh::create(BoxGeometry::create(0.2, 0.2, 0.2), matInvisible), Vector3{0, 0, 1.1}, std::nullopt, std::nullopt, std::nullopt}
                     }}
        };

        GizmoMap helperScale {
                {"X", {
                            {Line::create(lineGeometry, matHelper->clone()), Vector3{-1e3, 0, 0}, std::nullopt, Vector3{1e6, 1, 1}, "helper"}
                     }},
                {"Y", {
                            {Line::create(lineGeometry, matHelper->clone()), Vector3{0, -1e3, 0}, Euler{0, 0, math::PI/2}, Vector3{1e6, 1, 1}, "helper"}
                     }},
                {"Z", {
                            {Line::create(lineGeometry, matHelper->clone()), Vector3{0, 0, -1e3}, Euler{0, -math::PI/2, 0}, Vector3{1e6, 1, 1}, "helper"}
                     }}
                };

        // clang-format on

        {
            auto translate = setupGizmo(gizmoTranslate);
            this->gizmo["translate"] = translate.get();
            add(translate);

            auto rotate = setupGizmo(gizmoRotate);
            this->gizmo["rotate"] = rotate.get();
            add(rotate);

            auto scale = setupGizmo(gizmoScale);
            this->gizmo["scale"] = scale.get();
            add(scale);
        }

        {
            auto translate = setupGizmo(pickerTranslate);
            this->picker["translate"] = translate.get();
            add(translate);

            auto rotate = setupGizmo(pickerRotate);
            this->picker["rotate"] = rotate.get();
            add(rotate);

            auto scale = setupGizmo(pickerScale);
            this->picker["scale"] = scale.get();
            add(scale);
        }

        {
            auto translate = setupGizmo(helperTranslate);
            this->helper["translate"] = translate.get();
            add(translate);

            auto rotate = setupGizmo(helperRotate);
            this->helper["rotate"] = rotate.get();
            add(rotate);

            auto scale = setupGizmo(helperScale);
            this->helper["scale"] = scale.get();
            add(scale);
        }

        this->picker["translate"]->visible = false;
        this->picker["rotate"]->visible = false;
        this->picker["scale"]->visible = false;
    }

    std::shared_ptr<GizmoObject> setupGizmo(const GizmoMap& gizmoMap) {

        const auto gizmo = std::make_shared<GizmoObject>();

        for (const auto& [name, value] : gizmoMap) {

            for (unsigned i = value.size(); i--;) {

                auto object = std::get<0>(value[i])->clone();
                const auto position = std::get<1>(value[i]);
                const auto rotation = std::get<2>(value[i]);
                const auto scale = std::get<3>(value[i]);
                const auto tag = std::get<4>(value[i]);

                // name and tag properties are essential for picking and updating logic.
                object->name = name;
                object->userData["tag"] = tag;

                if (position) {

                    object->position.copy(*position);
                }

                if (rotation) {

                    object->rotation.copy(*rotation);
                }

                if (scale) {

                    object->scale.copy(*scale);
                }

                object->updateMatrix();

                const auto tempGeometry = object->geometry()->clone();
                tempGeometry->applyMatrix4(*object->matrix);
                object->setGeometry(tempGeometry);
                object->renderOrder = std::numeric_limits<int>::infinity();

                object->position.set(0, 0, 0);
                object->rotation.set(0, 0, 0);
                object->scale.set(1, 1, 1);

                gizmo->add(object);
            }
        }

        return gizmo;
    }

    void updateMatrixWorld(bool force) override {

        const auto space = (this->mode == "scale") ? "local" : this->space;// scale always oriented to local rotation

        const auto quaternion = (space == "local") ? this->worldQuaternion : _identityQuaternion;

        // Show only gizmos for current transform mode

        this->gizmo["translate"]->visible = this->mode == "translate";
        this->gizmo["rotate"]->visible = this->mode == "rotate";
        this->gizmo["scale"]->visible = this->mode == "scale";

        this->helper["translate"]->visible = this->mode == "translate";
        this->helper["rotate"]->visible = this->mode == "rotate";
        this->helper["scale"]->visible = this->mode == "scale";


        std::vector<GizmoObject*> handles;
        handles = handles.concat(this->picker[this->mode].children);
        handles = handles.concat(this->gizmo[this->mode].children);
        handles = handles.concat(this->helper[this->mode].children);

        for (unsigned i = 0; i < handles.size(); i++) {

            const auto handle = handles[i];

            // hide aligned to camera

            handle->visible = true;
            handle->rotation.set(0, 0, 0);
            handle->position.copy(this->worldPosition);

            float factor;

            if (this->camera.isOrthographicCamera) {

                factor = (this->camera.top - this->camera.bottom) / this->camera.zoom;

            } else {

                factor = this->worldPosition.distanceTo(this->cameraPosition) * std::min(1.9 * std::tan(math::PI * this->camera.fov / 360) / this->camera.zoom, 7);
            }

            handle->scale.set(1, 1, 1).multiplyScalar(factor * this->size / 7);

            // TODO: simplify helpers and consider decoupling from gizmo

            if (std::any_cast<std::string>(handle->userData["tag"]) == "helper") {

                handle->visible = false;

                if (handle->name == "AXIS") {

                    handle->position.copy(this->worldPositionStart);
                    handle->visible = !!this->axis;

                    if (this->axis == "X") {

                        _tempQuaternion.setFromEuler(_tempEuler.set(0, 0, 0));
                        handle->quaternion.copy(quaternion).multiply(_tempQuaternion);

                        if (std::abs(_alignVector.copy(_unitX).applyQuaternion(quaternion).dot(this->eye)) > 0.9) {

                            handle->visible = false;
                        }
                    }

                    if (this->axis == "Y") {

                        _tempQuaternion.setFromEuler(_tempEuler.set(0, 0, math::PI / 2));
                        handle->quaternion.copy(quaternion).multiply(_tempQuaternion);

                        if (std::abs(_alignVector.copy(_unitY).applyQuaternion(quaternion).dot(this->eye)) > 0.9) {

                            handle->visible = false;
                        }
                    }

                    if (this->axis == "Z") {

                        _tempQuaternion.setFromEuler(_tempEuler.set(0, math::PI / 2, 0));
                        handle->quaternion.copy(quaternion).multiply(_tempQuaternion);

                        if (std::abs(_alignVector.copy(_unitZ).applyQuaternion(quaternion).dot(this->eye)) > 0.9) {

                            handle->visible = false;
                        }
                    }

                    if (this->axis == "XYZE") {

                        _tempQuaternion.setFromEuler(_tempEuler.set(0, math::PI / 2, 0));
                        _alignVector.copy(this->rotationAxis);
                        handle->quaternion.setFromRotationMatrix(_lookAtMatrix.lookAt(_zeroVector, _alignVector, _unitY));
                        handle->quaternion.multiply(_tempQuaternion);
                        handle->visible = this->dragging;
                    }

                    if (this->axis == "E") {

                        handle->visible = false;
                    }


                } else if (handle->name == "START") {

                    handle->position.copy(this->worldPositionStart);
                    handle->visible = this->dragging;

                } else if (handle->name == "END") {

                    handle->position.copy(this->worldPosition);
                    handle->visible = this->dragging;

                } else if (handle->name == "DELTA") {

                    handle->position.copy(this->worldPositionStart);
                    handle->quaternion.copy(this->worldQuaternionStart);
                    _tempVector.set(1e-10, 1e-10, 1e-10).add(this->worldPositionStart).sub(this->worldPosition).multiplyScalar(-1);
                    _tempVector.applyQuaternion(this->worldQuaternionStart.clone().invert());
                    handle->scale.copy(_tempVector);
                    handle->visible = this->dragging;

                } else {

                    handle->quaternion.copy(quaternion);

                    if (this->dragging) {

                        handle->position.copy(this->worldPositionStart);

                    } else {

                        handle->position.copy(this->worldPosition);
                    }

                    if (this->axis) {

                        handle->visible = this->axis.search(handle->name) != std::string::npos;
                    }
                }

                // If updating helper, skip rest of the loop
                continue;
            }

            // Align handles to current local or world rotation

            handle->quaternion.copy(quaternion);

            if (this->mode == "translate" || this->mode == "scale") {

                // Hide translate and scale axis facing the camera

                const auto AXIS_HIDE_TRESHOLD = 0.99;
                const auto PLANE_HIDE_TRESHOLD = 0.2;
                const auto AXIS_FLIP_TRESHOLD = 0.0;

                if (handle->name == "X" || handle->name == "XYZX") {

                    if (std::abs(_alignVector.copy(_unitX).applyQuaternion(quaternion).dot(this->eye)) > AXIS_HIDE_TRESHOLD) {

                        handle->scale.set(1e-10, 1e-10, 1e-10);
                        handle->visible = false;
                    }
                }

                if (handle->name == "Y" || handle->name == "XYZY") {

                    if (std::abs(_alignVector.copy(_unitY).applyQuaternion(quaternion).dot(this->eye)) > AXIS_HIDE_TRESHOLD) {

                        handle->scale.set(1e-10, 1e-10, 1e-10);
                        handle->visible = false;
                    }
                }

                if (handle->name == "Z" || handle->name == "XYZZ") {

                    if (std::abs(_alignVector.copy(_unitZ).applyQuaternion(quaternion).dot(this->eye)) > AXIS_HIDE_TRESHOLD) {

                        handle->scale.set(1e-10, 1e-10, 1e-10);
                        handle->visible = false;
                    }
                }

                if (handle->name == "XY") {

                    if (std::abs(_alignVector.copy(_unitZ).applyQuaternion(quaternion).dot(this->eye)) < PLANE_HIDE_TRESHOLD) {

                        handle->scale.set(1e-10, 1e-10, 1e-10);
                        handle->visible = false;
                    }
                }

                if (handle->name == "YZ") {

                    if (std::abs(_alignVector.copy(_unitX).applyQuaternion(quaternion).dot(this->eye)) < PLANE_HIDE_TRESHOLD) {

                        handle->scale.set(1e-10, 1e-10, 1e-10);
                        handle->visible = false;
                    }
                }

                if (handle->name == "XZ") {

                    if (std::abs(_alignVector.copy(_unitY).applyQuaternion(quaternion).dot(this->eye)) < PLANE_HIDE_TRESHOLD) {

                        handle->scale.set(1e-10, 1e-10, 1e-10);
                        handle->visible = false;
                    }
                }

                // Flip translate and scale axis ocluded behind another axis

                if (handle->name.find('X') != std::string::npos) {

                    if (_alignVector.copy(_unitX).applyQuaternion(quaternion).dot(this->eye) < AXIS_FLIP_TRESHOLD) {

                        if (std::any_cast<std::string>(handle->userData["tag"]) == "fwd") {

                            handle->visible = false;

                        } else {

                            handle->scale.x *= -1;
                        }

                    } else if (std::any_cast<std::string>(handle->userData["tag"]) == "bwd") {

                        handle->visible = false;
                    }
                }

                if (handle->name.find('Y') != std::string::npos) {

                    if (_alignVector.copy(_unitY).applyQuaternion(quaternion).dot(this->eye) < AXIS_FLIP_TRESHOLD) {

                        if (std::any_cast<std::string>(handle->userData["tag"]) == "fwd") {

                            handle->visible = false;

                        } else {

                            handle->scale.y *= -1;
                        }

                    } else if (std::any_cast<std::string>(handle->userData["tag"]) == "bwd") {

                        handle->visible = false;
                    }
                }

                if (handle->name.find('Z') != std::string::npos) {

                    if (_alignVector.copy(_unitZ).applyQuaternion(quaternion).dot(this->eye) < AXIS_FLIP_TRESHOLD) {

                        if (std::any_cast<std::string>(handle->userData["tag"]) == "fwd") {

                            handle->visible = false;

                        } else {

                            handle->scale.z *= -1;
                        }

                    } else if (std::any_cast<std::string>(handle->userData["tag"]) == "bwd") {

                        handle->visible = false;
                    }
                }

            } else if (this->mode == "rotate") {

                // Align handles to current local or world rotation

                _tempQuaternion2.copy(quaternion);
                _alignVector.copy(this->eye).applyQuaternion(_tempQuaternion.copy(quaternion).invert());

                if (handle->name.find('E') != std::string::npos) {

                    handle->quaternion.setFromRotationMatrix(_lookAtMatrix.lookAt(this->eye, _zeroVector, _unitY));
                }

                if (handle->name == "X") {

                    _tempQuaternion.setFromAxisAngle(_unitX, std::atan2(-_alignVector.y, _alignVector.z));
                    _tempQuaternion.multiplyQuaternions(_tempQuaternion2, _tempQuaternion);
                    handle->quaternion.copy(_tempQuaternion);
                }

                if (handle->name == "Y") {

                    _tempQuaternion.setFromAxisAngle(_unitY, std::atan2(_alignVector.x, _alignVector.z));
                    _tempQuaternion.multiplyQuaternions(_tempQuaternion2, _tempQuaternion);
                    handle->quaternion.copy(_tempQuaternion);
                }

                if (handle->name == "Z") {

                    _tempQuaternion.setFromAxisAngle(_unitZ, std::atan2(_alignVector.y, _alignVector.x));
                    _tempQuaternion.multiplyQuaternions(_tempQuaternion2, _tempQuaternion);
                    handle->quaternion.copy(_tempQuaternion);
                }
            }

            // Hide disabled axes
            handle->visible = handle->visible && (handle->name.find('X') == std::string::npos || this->showX);
            handle->visible = handle->visible && (handle->name.find('Y') == std::string::npos || this->showY);
            handle->visible = handle->visible && (handle->name.find('Z') == std::string::npos || this->showZ);
            handle->visible = handle->visible && (handle->name.find('E') == std::string::npos || (this->showX && this->showY && this->showZ));

            // highlight selected axis

            if (!handle->_opacity) handle->_opacity = handle->material()->opacity;
            if (!handle->_color) handle->_color = handle->material()->as<MaterialWithColor>()->color;

            handle->material()->as<MaterialWithColor>()->color.copy(*handle->_color);
            handle->material()->opacity = *handle->_opacity;

            if (!this->enabled) {

                handle->material()->opacity *= 0.5;
                handle->material()->as<MaterialWithColor>()->color.lerp(Color(1, 1, 1), 0.5);

            } else if (this->axis) {

                if (handle->name == this->axis) {

                    handle->material()->opacity = 1.0;
                    handle->material()->as<MaterialWithColor>()->color.lerp(Color(1, 1, 1), 0.5);

                } else if (this->axis.split( '' ).some(function(a) {
                               return handle->name == = a;
                           })) {

                    handle->material()->opacity = 1.0;
                    handle->material()->as<MaterialWithColor>()->color.lerp(Color(1, 1, 1), 0.5);

                } else {

                    handle->material()->opacity *= 0.25;
                    handle->material()->as<MaterialWithColor>()->color.lerp(Color(1, 1, 1), 0.5);
                }
            }
        }


        Object3D::updateMatrixWorld(force);
    }
};

struct State {
    std::string mode;
    std::string space;
    std::string axis;
    Vector3 eye;
    Vector3 worldPosition;
    Quaternion worldQuaternion;
    Quaternion cameraQuaternion;
};

struct TransformControlsPlane: Mesh {

    State& state;

    explicit TransformControlsPlane(State& state)
        : state(state), Mesh(PlaneGeometry::create(100000, 100000, 2, 2),
                             MeshBasicMaterial::create({{"visible", false},
                                                        {"wireframe", true},
                                                        {"side", Side::Double},
                                                        {"transparent", true},
                                                        {"opacity", 0.1f},
                                                        {"toneMapped", false}})) {}

    void updateMatrixWorld(bool force) override {

        auto space = state.space;

        this->position.copy(state.worldPosition);

        if (state.mode == "scale") space = "local";// scale always oriented to local rotation

        _v1.copy(_unitX).applyQuaternion(space == "local" ? state.worldQuaternion : _identityQuaternion);
        _v2.copy(_unitY).applyQuaternion(space == "local" ? state.worldQuaternion : _identityQuaternion);
        _v3.copy(_unitZ).applyQuaternion(space == "local" ? state.worldQuaternion : _identityQuaternion);

        // Align the plane for current transform mode, axis and space.

        _alignVector.copy(_v2);

        if (state.mode == "translate" || state.mode == "scale") {

            if (state.axis == "X") {

                _alignVector.copy(state.eye).cross(_v1);
                _dirVector.copy(_v1).cross(_alignVector);
            } else if (state.axis == "Y") {
                _alignVector.copy(state.eye).cross(_v2);
                _dirVector.copy(_v2).cross(_alignVector);
            } else if (state.axis == "Z") {
                _alignVector.copy(state.eye).cross(_v3);
                _dirVector.copy(_v3).cross(_alignVector);
            } else if (state.axis == "XY") {
                _dirVector.copy(_v3);
            } else if (state.axis == "YZ") {
                _dirVector.copy(_v1);
            } else if (state.axis == "XZ") {
                _alignVector.copy(_v3);
                _dirVector.copy(_v2);
            } else if (state.axis == "XYZ" || state.axis == "E") {

                _dirVector.set(0, 0, 0);
            }
        } else {

            _dirVector.set(0, 0, 0);
        }

        if (_dirVector.length() == 0) {

            // If in rotate mode, make the plane parallel to camera
            this->quaternion.copy(state.cameraQuaternion);

        } else {

            _tempMatrix.lookAt(_tempVector.set(0, 0, 0), _dirVector, _alignVector);

            this->quaternion.setFromRotationMatrix(_tempMatrix);
        }

        Object3D::updateMatrixWorld(force);
    }
};

struct TransformControls::Impl {

    Vector3 _offset;
    Vector3 _startNorm;
    Vector3 _endNorm;
    Vector3 _cameraScale;

    Vector3 _parentPosition;
    Quaternion _parentQuaternion;
    Quaternion _parentQuaternionInv;
    Vector3 _parentScale;

    Vector3 _worldScaleStart;
    Quaternion _worldQuaternionInv;
    Vector3 _worldScale;

    Vector3 _positionStart;
    Quaternion _quaternionStart;
    Vector3 _scaleStart;

    Vector3 pointStart;
    Vector3 pointEnd;
    Vector3 worldPositionStart;
    Quaternion worldQuaternionStart;

    bool dragging{false};

    Vector3 rotationAxis;

    float rotationAngle{};

    std::optional<std::string> axis;

    std::shared_ptr<TransformControlsGizmo> _gizmo;
    std::shared_ptr<TransformControlsPlane> _plane;

    TransformControls& scope;
    PeripheralsEventSource& canvas;
    Camera& camera;
    Object3D* object = nullptr;

    Vector3 cameraPosition;

    State state;

    struct MyMouseListener: MouseListener {

        Impl& scope;
        bool moveEnabled = false;

        explicit MyMouseListener(Impl& scope): scope(scope) {}

        void onMouseDown(int button, const Vector2& pos) override {

            if (!scope.scope.enabled) return;

            button_ = button;
            moveEnabled = true;

            const auto rect = scope.canvas.size();

            Vector2 _pos;
            _pos.x = (pos.x - rect.width) / rect.width * 2 - 1;
            _pos.y = -(pos.y - rect.height) / rect.height * 2 - 1;

            //            scope.pointerHover(_pos);
            scope.pointerDown(button, _pos);
        }

        void onMouseMove(const Vector2& pos) override {
            if (!moveEnabled) return;
            if (!scope.scope.enabled) return;

            const auto rect = scope.canvas.size();

            Vector2 _pos;
            _pos.x = (pos.x - rect.width) / rect.width * 2 - 1;
            _pos.y = -(pos.y - rect.height) / rect.height * 2 - 1;

            scope.pointerMove(button_, _pos);
        }

        void onMouseUp(int button, const Vector2& pos) override {
            if (!scope.scope.enabled) return;

            button_ = -1;
            moveEnabled = false;

            const auto rect = scope.canvas.size();

            Vector2 _pos;
            _pos.x = (pos.x - rect.width) / rect.width * 2 - 1;
            _pos.y = -(pos.y - rect.height) / rect.height * 2 - 1;

            scope.pointerUp(button, _pos);
        }


    private:
        int button_;
    };


    MyMouseListener myMouseListener;

    Impl(TransformControls& scope, Camera& camera, PeripheralsEventSource& canvas)
        : scope(scope), myMouseListener(*this), camera(camera), canvas(canvas),
          _gizmo(std::make_shared<TransformControlsGizmo>()),
          _plane(std::make_shared<TransformControlsPlane>(state)) {

        this->camera.updateMatrixWorld();
        this->camera.matrixWorld->decompose(this->cameraPosition, state.cameraQuaternion, this->_cameraScale);

        state.eye.copy(this->cameraPosition).sub(state.worldPosition).normalize();

        canvas.addMouseListener(myMouseListener);
    }

    static std::optional<Intersection> intersectObjectWithRay(Object3D& object, Raycaster& raycaster, bool includeInvisible = false) {

        const auto allIntersections = raycaster.intersectObject(object, true);

        for (const auto& allIntersection : allIntersections) {

            if (allIntersection.object->visible || includeInvisible) {

                return allIntersection;
            }
        }

        return std::nullopt;
    }

    //    void pointerHover(Vector2 pointer) {
    //
    //        if (!this->scope.object || scope.dragging) return;
    //
    //        _raycaster.setFromCamera(pointer, this->camera);
    //
    //        const auto intersect = intersectObjectWithRay(*this->_gizmo->picker[scope.mode], _raycaster);
    //
    //        if (intersect) {
    //
    //            this->axis = intersect->object->name;
    //
    //        } else {
    //
    //            this->axis = std::nullopt;
    //        }
    //    }

    void pointerDown(int button, Vector2 pointer) {

        if (!this->object || scope.dragging || button != 0) return;

        if (this->axis) {

            _raycaster.setFromCamera(pointer, this->camera);

            const auto planeIntersect = intersectObjectWithRay(*this->_plane, _raycaster, true);

            if (planeIntersect) {

                auto space = scope.space;

                if (scope.mode == "scale") {

                    space = "local";

                } else if (this->axis == "E" || this->axis == "XYZE" || this->axis == "XYZ") {

                    space = "world";
                }

                if (space == "local" && scope.mode == "rotate") {

                    //                    const auto snap = scope.rotationSnap;
                    //
                    //                    if (this->axis == "X" && snap) this->object->rotation.x = std::round(this->object->rotation.x / snap) * snap;
                    //                    if (this->axis == "Y" && snap) this->object->rotation.y = std::round(this->object->rotation.y / snap) * snap;
                    //                    if (this->axis == "Z" && snap) this->object->rotation.z = std::round(this->object->rotation.z / snap) * snap;
                }

                this->object->updateMatrixWorld();
                this->object->parent->updateMatrixWorld();

                this->_positionStart.copy(this->object->position);
                this->_quaternionStart.copy(this->object->quaternion);
                this->_scaleStart.copy(this->object->scale);

                this->object->matrixWorld->decompose(this->worldPositionStart, this->worldQuaternionStart, this->_worldScaleStart);

                this->pointStart.copy(planeIntersect->point).sub(this->worldPositionStart);
            }

            scope.dragging = true;
            _mouseDownEvent.target = &this->scope.mode;
            scope.dispatchEvent(_mouseDownEvent);
        }
    }

    void pointerMove(int button, Vector2 pointer) {

        const auto axis = this->axis;
        const auto mode = this->scope.mode;
        const auto object = this->object;
        auto space = this->scope.space;

        if (mode == "scale") {

            space = "local";

        } else if (axis == "E" || axis == "XYZE" || axis == "XYZ") {

            space = "world";
        }

        if (!object || !axis || this->dragging == false || button != -1) return;

        _raycaster.setFromCamera(pointer, this->camera);

        const auto planeIntersect = intersectObjectWithRay(*this->_plane, _raycaster, true);

        if (!planeIntersect) return;

        this->pointEnd.copy(planeIntersect->point).sub(this->worldPositionStart);

        if (mode == "translate") {

            // Apply translate

            this->_offset.copy(this->pointEnd).sub(this->pointStart);

            if (space == "local" && axis != "XYZ") {

                this->_offset.applyQuaternion(this->_worldQuaternionInv);
            }

            if (axis->find('X') == std::string::npos) this->_offset.x = 0;
            if (axis->find('Y') == std::string::npos) this->_offset.y = 0;
            if (axis->find('Z') == std::string::npos) this->_offset.z = 0;

            if (space == "local" && axis != "XYZ") {

                this->_offset.applyQuaternion(this->_quaternionStart).divide(this->_parentScale);

            } else {

                this->_offset.applyQuaternion(this->_parentQuaternionInv).divide(this->_parentScale);
            }

            object->position.copy(this->_offset).add(this->_positionStart);

            // Apply translation snap

            if (this->scope.translationSnap) {

                if (space == "local") {

                    object->position.applyQuaternion(_tempQuaternion.copy(this->_quaternionStart).invert());

                    if (axis->find('X') != std::string::npos) {

                        object->position.x = std::round(object->position.x / *this->scope.translationSnap) * *this->scope.translationSnap;
                    }

                    if (axis->find('Y') != std::string::npos) {

                        object->position.y = std::round(object->position.y / *this->scope.translationSnap) * *this->scope.translationSnap;
                    }

                    if (axis->find('Z') != std::string::npos) {

                        object->position.z = std::round(object->position.z / *this->scope.translationSnap) * *this->scope.translationSnap;
                    }

                    object->position.applyQuaternion(this->_quaternionStart);
                }

                if (space == "world") {

                    if (object->parent) {

                        object->position.add(_tempVector.setFromMatrixPosition(*object->parent->matrixWorld));
                    }

                    if (axis->find('X') != std::string::npos) {

                        object->position.x = std::round(object->position.x / *this->scope.translationSnap) * *this->scope.translationSnap;
                    }

                    if (axis->find('Y') != std::string::npos) {

                        object->position.y = std::round(object->position.y / *this->scope.translationSnap) * *this->scope.translationSnap;
                    }

                    if (axis->find('Z') != std::string::npos) {

                        object->position.z = std::round(object->position.z / *this->scope.translationSnap) * *this->scope.translationSnap;
                    }

                    if (object->parent) {

                        object->position.sub(_tempVector.setFromMatrixPosition(*object->parent->matrixWorld));
                    }
                }
            }

        } else if (mode == "scale") {

            if (axis->find("XYZ") != std::string::npos) {

                auto d = this->pointEnd.length() / this->pointStart.length();

                if (this->pointEnd.dot(this->pointStart) < 0) d *= -1;

                _tempVector2.set(d, d, d);

            } else {

                _tempVector.copy(this->pointStart);
                _tempVector2.copy(this->pointEnd);

                _tempVector.applyQuaternion(this->_worldQuaternionInv);
                _tempVector2.applyQuaternion(this->_worldQuaternionInv);

                _tempVector2.divide(_tempVector);

                if (axis->find('X') == std::string::npos) {

                    _tempVector2.x = 1;
                }

                if (axis->find('Y') == std::string::npos) {

                    _tempVector2.y = 1;
                }

                if (axis->find('Z') == std::string::npos) {

                    _tempVector2.z = 1;
                }
            }

            // Apply scale

            object->scale.copy(this->_scaleStart).multiply(_tempVector2);

            //            if ( this->scaleSnap ) {
            //
            //                if ( axis.search( 'X' ) !== - 1 ) {
            //
            //                    object.scale.x = std::round( object.scale.x / this->scaleSnap ) * this->scaleSnap || this->scaleSnap;
            //
            //                }
            //
            //                if ( axis.search( 'Y' ) !== - 1 ) {
            //
            //                    object.scale.y = std::round( object.scale.y / this->scaleSnap ) * this->scaleSnap || this->scaleSnap;
            //
            //                }
            //
            //                if ( axis.search( 'Z' ) !== - 1 ) {
            //
            //                    object.scale.z = std::round( object.scale.z / this->scaleSnap ) * this->scaleSnap || this->scaleSnap;
            //
            //                }
            //
            //            }

        } else if (mode == "rotate") {

            this->_offset.copy(this->pointEnd).sub(this->pointStart);

            const auto ROTATION_SPEED = 20.f / this->state.worldPosition.distanceTo(_tempVector.setFromMatrixPosition(*this->camera.matrixWorld));

            if (axis == "E") {

                this->rotationAxis.copy(this->state.eye);
                this->rotationAngle = this->pointEnd.angleTo(this->pointStart);

                this->_startNorm.copy(this->pointStart).normalize();
                this->_endNorm.copy(this->pointEnd).normalize();

                this->rotationAngle *= (this->_endNorm.cross(this->_startNorm).dot(state.eye) < 0 ? 1 : -1);

            } else if (axis == "XYZE") {

                this->rotationAxis.copy(this->_offset).cross(state.eye).normalize();
                this->rotationAngle = this->_offset.dot(_tempVector.copy(this->rotationAxis).cross(state.eye)) * ROTATION_SPEED;

            } else if (axis == "X" || axis == "Y" || axis == "Z") {

                this->rotationAxis.copy(_unit[*axis]);

                _tempVector.copy(_unit[*axis]);

                if (space == "local") {

                    _tempVector.applyQuaternion(state.worldQuaternion);
                }

                this->rotationAngle = this->_offset.dot(_tempVector.cross(state.eye).normalize()) * ROTATION_SPEED;
            }

            // Apply rotation snap

            if (this->scope.rotationSnap) this->rotationAngle = std::round(this->rotationAngle / *this->scope.rotationSnap) * *this->scope.rotationSnap;

            // Apply rotate
            if (space == "local" && axis != "E" && axis != "XYZE") {

                object->quaternion.copy(this->_quaternionStart);
                object->quaternion.multiply(_tempQuaternion.setFromAxisAngle(this->rotationAxis, this->rotationAngle)).normalize();

            } else {

                this->rotationAxis.applyQuaternion(this->_parentQuaternionInv);
                object->quaternion.copy(_tempQuaternion.setFromAxisAngle(this->rotationAxis, this->rotationAngle));
                object->quaternion.multiply(this->_quaternionStart).normalize();
            }
        }

        this->scope.dispatchEvent(_changeEvent);
        this->scope.dispatchEvent(_objectChangeEvent);
    }

    void pointerUp(int button, Vector2 pointer) {

        if (button != 0) return;

        if (this->dragging && this->axis) {

            _mouseUpEvent.target = &this->scope.mode;
            this->scope.dispatchEvent(_mouseUpEvent);
        }

        this->dragging = false;
        this->axis = std::nullopt;
    }

    void attach(Object3D& object) {

        scope.visible = true;
        this->object = &object;
    }

    void detach() {

        this->object = nullptr;
        scope.visible = false;
        this->axis = std::nullopt;
    }
};

TransformControls::TransformControls(Camera& camera, PeripheralsEventSource& canvas)
    : pimpl_(std::make_unique<Impl>(*this, camera, canvas)) {

    this->visible = false;

    this->add(pimpl_->_gizmo);
    this->add(pimpl_->_plane);

    //    Object3D::updateMatrixWorld();
}

void TransformControls::updateMatrixWorld(bool force) {

    if (pimpl_->object) {

        pimpl_->object->updateMatrixWorld();

        if (!pimpl_->object->parent) {

            std::cerr << "TransformControls: The attached 3D object must be a part of the scene graph." << std::endl;

        } else {

            pimpl_->object->parent->matrixWorld->decompose(pimpl_->_parentPosition, pimpl_->_parentQuaternion, pimpl_->_parentScale);
        }

        pimpl_->object->matrixWorld->decompose(pimpl_->state.worldPosition, pimpl_->state.worldQuaternion, pimpl_->_worldScale);

        pimpl_->_parentQuaternionInv.copy(pimpl_->_parentQuaternion).invert();
        pimpl_->_worldQuaternionInv.copy(pimpl_->state.worldQuaternion).invert();
    }

    pimpl_->camera.updateMatrixWorld();
    pimpl_->camera.matrixWorld->decompose(pimpl_->cameraPosition, pimpl_->state.cameraQuaternion, pimpl_->_cameraScale);

    pimpl_->state.eye.copy(pimpl_->cameraPosition).sub(pimpl_->state.worldPosition).normalize();

    Object3D::updateMatrixWorld(force);
}

TransformControls& TransformControls::attach(Object3D& object) {

    pimpl_->attach(object);

    return *this;
}

TransformControls& TransformControls::detach() {

    pimpl_->detach();

    return *this;
}

TransformControls::~TransformControls() = default;
