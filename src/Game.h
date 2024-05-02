#pragma once

#include <SFML/Graphics.hpp>

class Game
{
public:
    Game();
    void run();

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

    int FPS_Limit = 144; //-1 == None.
    bool vSyncEnabled = false;

    bool mIsMovingUp = false;
    bool mIsMovingDown = false;
    bool mIsMovingLeft = false;
    bool mIsMovingRight = false;
    sf::RenderWindow mWindow;
    sf::View mView;
    sf::RectangleShape mPlayer;
    sf::Text mFPSText;
};
