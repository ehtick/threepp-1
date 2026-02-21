// https://github.com/mrdoob/three.js/blob/r129/examples/jsm/controls/TransformControls.js

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

        TransformControls(Camera& camera, PeripheralsEventSource& canvas);

        void setSpace(const std::string& space);

        [[nodiscard]] std::string getSpace() const;

        void setMode(const std::string& mode);

        TransformControls& attach(Object3D& object);

        TransformControls& detach();

        void updateMatrixWorld(bool force) override;

        ~TransformControls() override;

    private:
        struct Impl;
        std::unique_ptr<Impl> pimpl_;
    };

}// namespace threepp

#endif//THREEPP_TRANSFORMCONTROLS_HPP
