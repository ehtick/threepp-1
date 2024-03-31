
#ifndef THREEPP_TRANSFORMCONTROLS_HPP
#define THREEPP_TRANSFORMCONTROLS_HPP

#include "threepp/core/Object3D.hpp"

#include <memory>

namespace threepp {

    class Camera;
    class PeripheralsEventSource;

    class TransformControls: public Object3D {

    public:
        bool enabled = true;
        std::string mode{"translate"};
        std::string space{"world"};
        float size = 1;
        bool dragging = false;
        bool showX = true;
        bool showY = true;
        bool showZ = true;

        std::optional<float> rotationSnap;
        std::optional<float> translationSnap;

        TransformControls(Camera& camera, PeripheralsEventSource& canvas);

        TransformControls& attach(Object3D& object);

        TransformControls& detach();

        void updateMatrixWorld(bool force) override;

        ~TransformControls();

    private:
        struct Impl;
        std::unique_ptr<Impl> pimpl_;
    };

}// namespace threepp

#endif//THREEPP_TRANSFORMCONTROLS_HPP
