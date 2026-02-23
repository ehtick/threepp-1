
#ifndef THREEPP_TEXTNODE_HPP
#define THREEPP_TEXTNODE_HPP

#include "threepp/math/Color.hpp"
#include "threepp/objects/Sprite.hpp"

#include <filesystem>
#include <memory>
#include <string>

namespace threepp {

    // A class for creating 2D text sprites in a 3D scene.
    // Only works with TrueType fonts.
    class TextSprite: public Sprite {

    public:
        explicit TextSprite(const std::filesystem::path& fontPath);

        void setFont(const std::filesystem::path& fontPath);

        void setColor(const Color& color);

        void setText(const std::string& text, float worldScale = 1);

        static std::shared_ptr<TextSprite> create(const std::filesystem::path& fontPath);

        ~TextSprite() override;

    private:
        struct Impl;
        std::unique_ptr<Impl> pimpl_;
    };

}// namespace threepp

#endif//THREEPP_TEXTNODE_HPP
