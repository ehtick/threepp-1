
#include "threepp/threepp.hpp"
#include "threepp/controls/TransformControls.hpp"

using namespace threepp;

int main() {

    Canvas canvas("Transform controls");
    GLRenderer renderer(canvas.size());
    renderer.shadowMap().enabled = true;
    renderer.shadowMap().type = threepp::ShadowMap::PFC;

    PerspectiveCamera camera(60, canvas.aspect());
    camera.position.z = 10;

    Scene scene;
//    scene.background = Color(0xf0f0f0);

    scene.add(AmbientLight::create(0xaaaaaa));

    auto light = SpotLight::create(0xffffff, 1.f);
    light->position.set(0, 25, 50);
    light->angle = math::PI / 9;

    light->castShadow = true;
    light->shadow->camera->near = 10;
    light->shadow->camera->far = 100;
    light->shadow->mapSize.x = 1024;
    light->shadow->mapSize.y = 1024;

    scene.add(light);

    auto material = MeshBasicMaterial::create();
    auto object = Mesh::create(BoxGeometry::create(), material);
    scene.add(object);

    TransformControls controls(camera, canvas);
    controls.object = object.get();

    scene.add(controls);

    canvas.onWindowResize([&](WindowSize size) {
        camera.aspect = size.aspect();
        camera.updateProjectionMatrix();

        renderer.setSize(size);
    });

    canvas.animate([&] {
        renderer.render(scene, camera);
    });
}
