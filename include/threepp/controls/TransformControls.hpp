
#ifndef THREEPP_TRANSFORMCONTROLS_HPP
#define THREEPP_TRANSFORMCONTROLS_HPP

#include "threepp/core/Object3D.hpp"

#include <memory>

namespace threepp {

    class Camera;
    class Canvas;

    class TransformControls: public Object3D {

    public:

        bool visible = true;

        TransformControls(Camera& camera, Canvas& canvas);


    private:
        struct Impl;
        std::unique_ptr<Impl> pimpl_;

    };

}

#endif//THREEPP_TRANSFORMCONTROLS_HPP
