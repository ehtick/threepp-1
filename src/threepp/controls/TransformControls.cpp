
#include "threepp/controls/TransformControls.hpp"

#include "threepp/cameras/Camera.hpp"
#include "threepp/objects/Line.hpp"
#include "threepp/objects/Mesh.hpp"

#include "threepp/materials/LineBasicMaterial.hpp"
#include "threepp/materials/MeshBasicMaterial.hpp"

#include "threepp/geometries/CylinderGeometry.hpp"

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

    Vector3 _tmpVector;
    Vector2 _tempVector2;
    Quaternion _tempQuaternion;

    using GizmoMap = std::unordered_map<std::string, std::vector<std::tuple<std::shared_ptr<Object3D>, std::optional<Vector3>, std::optional<Euler>, std::optional<Vector3>, std::optional<std::string>>>>;

    std::shared_ptr<BufferGeometry> CircleGeometry(float radius, int arc) {

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


}// namespace

struct TransformControlsGizmo: Object3D {

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
                        }}
        };
        // clang-format on
    }
};

struct TransformControlsPlane: Mesh {

    TransformControlsPlane()
        : Mesh(PlaneGeometry::create(100000, 100000, 2, 2),
               MeshBasicMaterial::create({{"visible", false},
                                          {"wireframe", true},
                                          {"side", Side::Double},
                                          {"transparent", true},
                                          {"opacity", 0.1f},
                                          {"toneMapped", false}})) {}

    void updateMatrixWorld(bool force) override {

        auto space = this.space;

        this.position.copy( this.worldPosition );

        if ( this.mode === 'scale' ) space = 'local'; // scale always oriented to local rotation

        _v1.copy( _unitX ).applyQuaternion( space === 'local' ? this.worldQuaternion : _identityQuaternion );
        _v2.copy( _unitY ).applyQuaternion( space === 'local' ? this.worldQuaternion : _identityQuaternion );
        _v3.copy( _unitZ ).applyQuaternion( space === 'local' ? this.worldQuaternion : _identityQuaternion );

        // Align the plane for current transform mode, axis and space.

        _alignVector.copy( _v2 );

        switch ( this.mode ) {

            case 'translate':
            case 'scale':
                switch ( this.axis ) {

                    case 'X':
                        _alignVector.copy( this.eye ).cross( _v1 );
                        _dirVector.copy( _v1 ).cross( _alignVector );
                        break;
                    case 'Y':
                        _alignVector.copy( this.eye ).cross( _v2 );
                        _dirVector.copy( _v2 ).cross( _alignVector );
                        break;
                    case 'Z':
                        _alignVector.copy( this.eye ).cross( _v3 );
                        _dirVector.copy( _v3 ).cross( _alignVector );
                        break;
                    case 'XY':
                        _dirVector.copy( _v3 );
                        break;
                    case 'YZ':
                        _dirVector.copy( _v1 );
                        break;
                    case 'XZ':
                        _alignVector.copy( _v3 );
                        _dirVector.copy( _v2 );
                        break;
                    case 'XYZ':
                    case 'E':
                        _dirVector.set( 0, 0, 0 );
                        break;

                }

                break;
            case 'rotate':
            default:
                // special case for rotate
                _dirVector.set( 0, 0, 0 );

        }

        if ( _dirVector.length() === 0 ) {

            // If in rotate mode, make the plane parallel to camera
            this.quaternion.copy( this.cameraQuaternion );

        } else {

            _tempMatrix.lookAt( _tempVector.set( 0, 0, 0 ), _dirVector, _alignVector );

            this.quaternion.setFromRotationMatrix( _tempMatrix );

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

    std::shared_ptr<TransformControlsGizmo> _gizmo;
    std::shared_ptr<TransformControlsPlane> _plane;

    TransformControls& scope;
    PeripheralsEventSource& canvas;
    Camera& camera;

    Object3D* object = nullptr;

    Vector3 worldPosition;
    Quaternion worldQuaternion;
    Vector3 cameraPosition;
    Quaternion cameraQuaternion;
    Vector3 eye;


    //    template <class T>
    //    struct Property {
    //
    //        Property(Impl& scope, const std::string& propName, T defaultValue)
    //            : scope(scope), defaultValue(defaultValue), propName(propName) {}
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
    //
    //        }
    //
    //    private:
    //        Impl& scope;
    //        T defaultValue;
    //        std::string propName;
    //        std::optional<T> propValue;
    //
    //    };

    Impl(TransformControls& scope, Camera& camera, PeripheralsEventSource& canvas)
        : scope(scope), camera(camera), canvas(canvas),
          _gizmo(std::make_shared<TransformControlsGizmo>()),
          _plane(std::make_shared<TransformControlsPlane>()) {

        this->camera.updateMatrixWorld();
        this->camera.matrixWorld->decompose(this->cameraPosition, this->cameraQuaternion, this->_cameraScale);

        this->eye.copy(this->cameraPosition).sub(this->worldPosition).normalize();
    }

    static std::optional<Intersection> intersectObjectWithRay(Object3D& object, Raycaster& raycaster, bool includeInvisible) {

        const auto allIntersections = raycaster.intersectObject(object, true);

        for (const auto& allIntersection : allIntersections) {

            if (allIntersection.object->visible || includeInvisible) {

                return allIntersection;
            }
        }

        return std::nullopt;
    }

    void pointerHover(Vector2 pointer) {

        if (!this->object || scope.dragging) return;

        _raycaster.setFromCamera(pointer, this->camera);

        const auto intersect = intersectObjectWithRay(this->_gizmo.picker[scope.mode], _raycaster);

        if (intersect) {

            this->axis = intersect.object.name;

        } else {

            this->axis = nullptr;
        }
    }

    void pointerDown(int button, Vector2 pointer) {

        if (!this->object || scope.dragging || button != 0) return;

        if (this->axis != nullptr) {

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

                    const auto snap = scope.rotationSnap;

                    if (this->axis == "X" && snap) this->object->rotation.x = std::round(this->object->rotation.x / snap) * snap;
                    if (this->axis == "Y" && snap) this->object->rotation.y = std::round(this->object->rotation.y / snap) * snap;
                    if (this->axis == "Z" && snap) this->object->rotation.z = std::round(this->object->rotation.z / snap) * snap;
                }

                this->object->updateMatrixWorld();
                this->object->parent->updateMatrixWorld();

                this->_positionStart.copy(this->object->position);
                this->_quaternionStart.copy(this->object->quaternion);
                this->_scaleStart.copy(this->object->scale);

                this->object->matrixWorld->decompose(this->worldPositionStart, this->worldQuaternionStart, this._worldScaleStart);

                this->pointStart.copy(planeIntersect.point).sub(this.worldPositionStart);
            }

            scope.dragging = true;
            _mouseDownEvent.mode = this.mode;
            scope.dispatchEvent(_mouseDownEvent);
        }
    }

    std::shared_ptr<Object3D> setupGizmo(const GizmoMap& gizmoMap) {

        const auto gizmo = Object3D::create();

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
};

TransformControls::TransformControls(Camera& camera, PeripheralsEventSource& canvas): pimpl_(std::make_unique<Impl>(*this, camera, canvas)) {

    this->visible = false;

    this->add(pimpl_->_gizmo);
    this->add(pimpl_->_plane);

    Object3D::updateMatrixWorld();
}

void TransformControls::updateMatrixWorld(bool force) {

    if (pimpl_->object) {

        pimpl_->object->updateMatrixWorld();

        if (!pimpl_->object->parent) {

            std::cerr << "TransformControls: The attached 3D object must be a part of the scene graph." << std::endl;

        } else {

            pimpl_->object->parent->matrixWorld->decompose(pimpl_->_parentPosition, pimpl_->_parentQuaternion, pimpl_->_parentScale);
        }

        pimpl_->object->matrixWorld->decompose(pimpl_->worldPosition, pimpl_->worldQuaternion, pimpl_->_worldScale);

        pimpl_->_parentQuaternionInv.copy(pimpl_->_parentQuaternion).invert();
        pimpl_->_worldQuaternionInv.copy(pimpl_->worldQuaternion).invert();
    }

    pimpl_->camera.updateMatrixWorld();
    pimpl_->camera.matrixWorld->decompose(pimpl_->cameraPosition, pimpl_->cameraQuaternion, pimpl_->_cameraScale);

    pimpl_->eye.copy(pimpl_->cameraPosition).sub(pimpl_->worldPosition).normalize();

    Object3D::updateMatrixWorld(force);
}


TransformControls::~TransformControls() = default;
