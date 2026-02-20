
#include "threepp/controls/TransformControls.hpp"
#include "threepp/threepp.hpp"

using namespace threepp;

int main() {

    Canvas canvas("Transform controls");
    GLRenderer renderer(canvas.size());
    renderer.shadowMap().enabled = true;
    renderer.shadowMap().type = ShadowMap::PFC;

    PerspectiveCamera camera(60, canvas.aspect());
    camera.position.z = 10;

    Scene scene;
    scene.background = Color(0xf0f0f0);

    scene.add(AmbientLight::create(0xaaaaaa));

    auto light = SpotLight::create(0xffffff, 1.f);
    light->position.set(0, 25, 50);
    light->angle = math::PI / 9;
    //
    // light->castShadow = true;
    // light->shadow->camera->as<OrthographicCamera>()->nearPlane = 10;
    // light->shadow->camera->as<OrthographicCamera>()->farPlane = 100;
    // light->shadow->mapSize.x = 1024;
    // light->shadow->mapSize.y = 1024;

    scene.add(light);

    auto material = MeshBasicMaterial::create();
    material->transparent = true;
    material->opacity = 0.7;
    auto object = Mesh::create(BoxGeometry::create(), material);
    scene.add(object);

    TransformControls controls(camera, canvas);
    controls.attach(*object);

    scene.add(controls);

    OrbitControls orbitControls(camera, canvas);
    orbitControls.enabled = false;

    KeyAdapter adapter(KeyAdapter::Mode::KEY_PRESSED, [&](KeyEvent evt) {
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
            case Key::SPACE: {
                orbitControls.enabled = !orbitControls.enabled;
            }
        }
    });
    canvas.addKeyListener(adapter);

    canvas.onWindowResize([&](WindowSize size) {
        camera.aspect = size.aspect();
        camera.updateProjectionMatrix();

        renderer.setSize(size);
    });

    canvas.animate([&] {
        renderer.render(scene, camera);
    });
}
