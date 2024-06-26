// https://github.com/mrdoob/three.js/blob/r129/src/materials/MeshMatcapMaterial.js

#ifndef THREEPP_MESHMATCAPMATERIAL_HPP
#define THREEPP_MESHMATCAPMATERIAL_HPP

#include "Material.hpp"
#include "interfaces.hpp"

namespace threepp {

    class MeshMatcapMaterial: public virtual Material,
                              public MaterialWithColor,
                              public MaterialWithMap,
                              public MaterialWithAlphaMap,
                              public MaterialWithBumpMap,
                              public MaterialWithDisplacementMap,
                              public MaterialWithNormalMap,
                              public MaterialWithMatCap,
                              public MaterialWithFlatShading,
                              public MaterialWithDefines {

    public:
        [[nodiscard]] std::string type() const override {

            return "MeshMatcapMaterial";
        }

        void copyInto(Material& material) const override {

            Material::copyInto(material);

            auto m = material.as<MeshMatcapMaterial>();

            m->defines["MATCAP"] = "";

            m->color.copy(color);

            m->matcap = matcap;

            m->map = map;

            m->bumpMap = bumpMap;
            m->bumpScale = bumpScale;

            m->normalMap = normalMap;
            m->normalMapType = normalMapType;
            m->normalScale.copy(normalScale);

            m->displacementMap = displacementMap;
            m->displacementScale = displacementScale;
            m->displacementBias = displacementBias;

            m->alphaMap = alphaMap;

            m->flatShading = flatShading;
        }

        static std::shared_ptr<MeshMatcapMaterial> create() {

            return std::shared_ptr<MeshMatcapMaterial>(new MeshMatcapMaterial());
        }

    protected:
        MeshMatcapMaterial()
            : MaterialWithColor(0xffffff),
              MaterialWithFlatShading(false),
              MaterialWithBumpMap(1),
              MaterialWithDisplacementMap(1, 0),
              MaterialWithNormalMap(NormalMapType::TangentSpace, {1, 1}) {

            this->defines["MATCAP"] = "";
        }

        std::shared_ptr<Material> createDefault() const override {

            return std::shared_ptr<MeshMatcapMaterial>(new MeshMatcapMaterial());
        }
    };

}// namespace threepp

#endif//THREEPP_MESHMATCAPMATERIAL_HPP
