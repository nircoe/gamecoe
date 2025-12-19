#include <gamecoe.hpp>
#include <iostream>
#include <cmath>
#include <optional>

int main()
{
    gamecoe::Game game;

    auto &camera = game.mainCamera();
    camera.owner().transform().translate({3.0f, 3.0f, 5.0f});
    camera.owner().transform().lookAt({0.0f, 0.0f, 0.0f});

    auto &scene = game.createScene("tester");
    auto &shape = scene.createGameObject("shape");
    shape.setRenderer(gamecoe::ShapeRenderer::cube(shape, colorcoe::maroon()));
    // shape.transform().setScale({0.5f, 0.5f, 0.5f});
    // shape.transform().rotate({0.0f, 0.0f, 45.0f});
    // shape.transform().translate({0.25f, 0.3f, 0.0f});


    game.loadScene("tester");
    game.activateScene("tester");

    game.play();

    return 0;
}