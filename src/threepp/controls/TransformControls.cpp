
#include "threepp/controls/TransformControls.hpp"

#include "threepp/cameras/Camera.hpp"
#include "threepp/objects/Mesh.hpp"

#include "threepp/materials/LineBasicMaterial.hpp"
#include "threepp/materials/MeshBasicMaterial.hpp"

#include "threepp/geometries/CylinderGeometry.hpp"

#include "threepp/geometries/BoxGeometry.hpp"
#include "threepp/geometries/PlaneGeometry.hpp"

#include "threepp/core/Raycaster.hpp"

#include "threepp/input/PeripheralsEventSource.hpp"

#include <cmath>
#include <iostream>

using namespace threepp;

namespace {

    Raycaster _raycaster;

    Vector3 _tmpVector;
    Vector2 _tempVector2;
    Quaternion _tempQuaternion;


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

        const auto matRed = gizmoMaterial->clone()->as<MeshBasicMaterial>();
        matRed->color.setHex(0xff0000);

        const auto matGreen = gizmoMaterial->clone()->as<MeshBasicMaterial>();
        matGreen->color.setHex(0x00ff00);

        const auto matBlue = gizmoMaterial->clone()->as<MeshBasicMaterial>();
        matBlue->color.setHex(0x0000ff);

        const auto matWhiteTransparent = gizmoMaterial->clone()->as<MeshBasicMaterial>();
        matWhiteTransparent->opacity = 0.25f;

        const auto matYellowTransparent = matWhiteTransparent->as<MeshBasicMaterial>();
        matYellowTransparent->color.setHex(0xffff00);

        const auto matCyanTransparent = matWhiteTransparent->as<MeshBasicMaterial>();
        matCyanTransparent->color.setHex(0x00ffff);

        const auto matMagentaTransparent = matWhiteTransparent->as<MeshBasicMaterial>();
        matMagentaTransparent->color.setHex(0xff00ff);

        const auto matYellow = gizmoMaterial->clone()->as<MeshBasicMaterial>();
        matYellow->color.setHex(0xffff00);

        const auto matLineRed = gizmoLineMaterial->clone()->as<LineBasicMaterial>();
        matLineRed->color.setHex(0xff0000);

        const auto matLineGreen = gizmoLineMaterial->clone()->as<LineBasicMaterial>();
        matLineGreen->color.setHex(0x00ff00);

        const auto matLineBlue = gizmoLineMaterial->clone()->as<LineBasicMaterial>();
        matLineBlue->color.setHex(0x0000ff);

        const auto matLineCyan = gizmoLineMaterial->clone()->as<LineBasicMaterial>();
        matLineCyan->color.setHex(0x00ffff);

        const auto matLineMagenta = gizmoLineMaterial->clone()->as<LineBasicMaterial>();
        matLineMagenta->color.setHex(0xff00ff);

        const auto matLineYellow = gizmoLineMaterial->clone()->as<LineBasicMaterial>();
        matLineYellow->color.setHex(0xffff00);

        const auto matLineGray = gizmoLineMaterial->clone()->as<LineBasicMaterial>();
        matLineGray->color.setHex(0x787878);

        const auto matLineYellowTransparent = matLineYellow->clone();
        matLineYellowTransparent->opacity = 0.25f;

        // reusable geometry

        const auto arrowGeometry = CylinderGeometry::create(0, 0.05, 0.2, 12, 1, false);

        const auto scaleHandleGeometry = BoxGeometry::create(0.125, 0.125, 0.125);

        const auto lineGeometry = BufferGeometry::create();
        lineGeometry->setAttribute("position", FloatBufferAttribute::create(std::vector<float>{0, 0, 0, 1, 0, 0}, 3));
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

    static std::optional<Intersection> intersectObjectWithRay( Object3D& object, Raycaster& raycaster, bool includeInvisible ) {

        const auto allIntersections = raycaster.intersectObject( object, true );

        for (const auto & allIntersection : allIntersections) {

            if ( allIntersection.object->visible || includeInvisible ) {

                return allIntersection;

            }

        }

        return std::nullopt;

    }

    void pointerHover( Vector2 pointer ) {

        if ( !this->object || scope.dragging ) return;

        _raycaster.setFromCamera( pointer, this->camera );

        const auto intersect = intersectObjectWithRay( this->_gizmo.picker[ scope.mode ], _raycaster );

        if ( intersect ) {

            this->axis = intersect.object.name;

        } else {

            this->axis = nullptr;

        }

    }

    void pointerDown(int button, Vector2 pointer ) {

        if ( !this->object || scope.dragging || button != 0 ) return;

        if ( this->axis != nullptr ) {

            _raycaster.setFromCamera( pointer, this->camera );

            const auto planeIntersect = intersectObjectWithRay( *this->_plane, _raycaster, true );

            if ( planeIntersect ) {

                auto space = scope.space;

                if ( scope.mode == "scale" ) {

                    space = "local";

                } else if ( this->axis == "E" || this->axis == "XYZE" || this->axis == "XYZ" ) {

                    space = "world";

                }

                if ( space == "local" && scope.mode == "rotate" ) {

                    const auto snap = scope.rotationSnap;

                    if ( this->axis == "X" && snap ) this->object->rotation.x = std::round( this->object->rotation.x / snap ) * snap;
                    if ( this->axis == "Y" && snap ) this->object->rotation.y = std::round( this->object->rotation.y / snap ) * snap;
                    if ( this->axis == "Z" && snap ) this->object->rotation.z = std::round( this->object->rotation.z / snap ) * snap;

                }

                this->object->updateMatrixWorld();
                this->object->parent->updateMatrixWorld();

                this->_positionStart.copy( this->object->position );
                this->_quaternionStart.copy( this->object->quaternion );
                this->_scaleStart.copy( this->object->scale );

                this->object->matrixWorld->decompose( this->worldPositionStart, this->worldQuaternionStart, this._worldScaleStart );

                this->pointStart.copy( planeIntersect.point ).sub( this.worldPositionStart );

            }

            scope.dragging = true;
            _mouseDownEvent.mode = this.mode;
            scope.dispatchEvent( _mouseDownEvent );

        }

    }

};

TransformControls::TransformControls(Camera& camera, PeripheralsEventSource& canvas): pimpl_(std::make_unique<Impl>(*this, camera, canvas)) {

    this->visible = false;

    this->add(pimpl_->_gizmo);
    this->add(pimpl_->_plane);

    Object3D::updateMatrixWorld();
}

void TransformControls::updateMatrixWorld(bool force) {

    if ( pimpl_->object ) {

        pimpl_->object->updateMatrixWorld();

        if ( !pimpl_->object->parent ) {

            std::cerr << "TransformControls: The attached 3D object must be a part of the scene graph." << std::endl;

        } else {

            pimpl_->object->parent->matrixWorld->decompose( pimpl_->_parentPosition, pimpl_->_parentQuaternion, pimpl_->_parentScale );

        }

        pimpl_->object->matrixWorld->decompose( pimpl_->worldPosition, pimpl_->worldQuaternion, pimpl_->_worldScale );

        pimpl_->_parentQuaternionInv.copy( pimpl_->_parentQuaternion ).invert();
        pimpl_->_worldQuaternionInv.copy( pimpl_->worldQuaternion ).invert();

    }

    pimpl_->camera.updateMatrixWorld();
    pimpl_->camera.matrixWorld->decompose( pimpl_->cameraPosition, pimpl_->cameraQuaternion, pimpl_->_cameraScale );

    pimpl_->eye.copy( pimpl_->cameraPosition ).sub( pimpl_->worldPosition ).normalize();

    Object3D::updateMatrixWorld(force);
}


TransformControls::~TransformControls() = default;
