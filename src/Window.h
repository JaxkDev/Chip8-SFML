#pragma once

#include <SFML/Graphics.hpp>

#include "vCPU.h"

class Window {
public:
    Window();

    void loop();

    int getFPS() const { return FPS; }
    int getTPS() const { return TPS; }

private:
    void handlePlayerInput(sf::Keyboard::Key key, bool isPressed);

    void processEvents();

    void update(double time, double deltaTime);

    void render(double time);

    //Vars
    int FPS = 0;
    int TPS = 0;

    int FPS_Limit = 60; // CHIP-8 Ran at 60FPS / 60Hz

    sf::RenderWindow mWindow;
    sf::View mView;

    vCPU cpu;
};
