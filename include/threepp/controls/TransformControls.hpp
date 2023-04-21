
#ifndef THREEPP_TRANSFORMCONTROLS_HPP
#define THREEPP_TRANSFORMCONTROLS_HPP

#include "threepp/core/Object3D.hpp"

#include <memory>

namespace threepp {

    class Camera;
    class Canvas;

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

        TransformControls(Object3D& object, Canvas& canvas);

        ~TransformControls();

    private:
        struct Impl;
        std::unique_ptr<Impl> pimpl_;
    };

}// namespace threepp

#endif//THREEPP_TRANSFORMCONTROLS_HPP
