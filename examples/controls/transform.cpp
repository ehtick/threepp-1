
#include "threepp/controls/TransformControls.hpp"
#include "threepp/threepp.hpp"

#include <iostream>

using namespace threepp;

int main() {

    Canvas canvas(Canvas::Parameters()
                          .title("Transform controls")
                          .exitOnKeyEscape(false));

    GLRenderer renderer(canvas.size());
    renderer.shadowMap().enabled = true;
    renderer.shadowMap().type = ShadowMap::PFC;

    PerspectiveCamera camera(60, canvas.aspect());
    camera.position.set(0, 5, 5);

    Scene scene;
    scene.background = Color::aliceblue;

    scene.add(AmbientLight::create(0xaaaaaa));

    auto light = SpotLight::create(0xffffff, 1.f);
    light->position.set(0, 25, 50);
    light->angle = math::PI / 9;

    light->castShadow = true;
    light->shadow->camera->as<PerspectiveCamera>()->nearPlane = 10;
    light->shadow->camera->as<PerspectiveCamera>()->farPlane = 100;
    light->shadow->mapSize.x = 1024;
    light->shadow->mapSize.y = 1024;

    scene.add(light);

    TextureLoader tl;
    auto tex = tl.load(std::string(DATA_FOLDER) + "/textures/crate.gif");

    auto material = MeshBasicMaterial::create();
    material->transparent = true;
    material->opacity = 0.7f;
    material->map = tex;
    auto object = Mesh::create(BoxGeometry::create(), material);
    scene.add(object);


    auto grid = GridHelper::create(10, 10);
    scene.add(grid);

    OrbitControls orbitControls(camera, canvas);

    TransformControls controls(camera, canvas);
    controls.attach(*object);
    scene.add(controls);

    LambdaEventListener changeListener([&](Event& event) {
        orbitControls.enabled = !std::any_cast<bool>(event.target);
    });

    controls.addEventListener("dragging-changed", changeListener);

    KeyAdapter keyDownListener(KeyAdapter::Mode::KEY_PRESSED, [&](KeyEvent evt) {
        switch (evt.key) {
            case Key::Q: {
                controls.setSpace(controls.getSpace() == "local" ? "world" : "local");
                break;
            }
            case Key::W: {
                controls.setMode("translate");
                break;
            }
            case Key::E: {
                controls.setMode("rotate");
                break;
            }
            case Key::R: {
                controls.setMode("scale");
                break;
            }
            case Key::X: {
                controls.showX = !controls.showX;
                break;
            }
            case Key::Y: {
                controls.showY = !controls.showY;
                break;
            }
            case Key::Z: {
                controls.showZ = !controls.showZ;
                break;
            }
            case Key::SPACE: {
                controls.enabled = !controls.enabled;
                break;
            }
            case Key::LEFT_SHIFT:
                controls.setTranslationSnap(1.f);
                controls.setRotationSnap(math::degToRad(15.f));
                controls.setScaleSnap(0.25f);
            default:
                break;
        }
    });
    KeyAdapter keyUpListener(KeyAdapter::Mode::KEY_RELEASED, [&](KeyEvent evt) {
        if (evt.key == Key::LEFT_SHIFT) {
            controls.setTranslationSnap(std::nullopt);
            controls.setRotationSnap(std::nullopt);
            controls.setScaleSnap(std::nullopt);
        }
    });
    canvas.addKeyListener(keyDownListener);
    canvas.addKeyListener(keyUpListener);


    canvas.onWindowResize([&](WindowSize size) {
        camera.aspect = size.aspect();
        camera.updateProjectionMatrix();

        renderer.setSize(size);
    });

    canvas.animate([&] {
        renderer.render(scene, camera);
    });
}
