#include "Window.h"
#include <iostream>
#include <chrono>


// https://github.com/SFML/SFML/wiki/Source%3A-Letterbox-effect-using-a-view#the-function
sf::View getLetterboxView(sf::View view, const int windowWidth, const int windowHeight) {
    // Compares the aspect ratio of the window to the aspect ratio of the view,
    // and sets the view's viewport accordingly in order to achieve a letterbox effect.
    // A new view (with a new viewport set) is returned.

    const float windowRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    const float viewRatio = view.getSize().x / static_cast<float>(view.getSize().y);
    float sizeX = 1;
    float sizeY = 1;
    float posX = 0;
    float posY = 0;

    bool horizontalSpacing = true;
    if (windowRatio < viewRatio)
        horizontalSpacing = false;

    // If horizontalSpacing is true, the black bars will appear on the left and right side.
    // Otherwise, the black bars will appear on the top and bottom.

    if (horizontalSpacing) {
        sizeX = viewRatio / windowRatio;
        posX = (1 - sizeX) / 2.f;
    } else {
        sizeY = windowRatio / viewRatio;
        posY = (1 - sizeY) / 2.f;
    }

    view.setViewport(sf::FloatRect(posX, posY, sizeX, sizeY));

    return view;
}

// ReSharper disable twice CppDFAConstantConditions - vSync
// ReSharper disable once CppDFAUnreachableCode - vSync
Window::Window() :
    mWindow(sf::VideoMode(512, 512, 1), "CHIP8 Emulator", sf::Style::Default)
{
    const auto mode = sf::VideoMode(512, 512, 1); //sf::VideoMode::getDesktopMode();
    std::cout << "Using resolution: " << mode.width << "x" << mode.height << " - " << mode.bitsPerPixel << " bpp" <<
            std::endl;
    std::cout << "FPS Limit: " << FPS_Limit << std::endl;
    mView.setSize(66, 34);
    mView.setCenter(mView.getSize().x / 2, mView.getSize().y / 2);
    mView = getLetterboxView(mView, mode.width, mode.height); // NOLINT(*-narrowing-conversions)

    mWindow.setVerticalSyncEnabled(false);
    //mWindow.setIcon(100, 100, sf::Image()); // Set the window's icon

    cpu.loadROM("assets/test.2.ch8");
}


/// Fixed timestep for Rendering and Update
void Window::loop() {
    int frames = 0;
    int ticks = 0;
    sf::Clock frameClock;

    double t = 0.0; //
    const double dt = 1.0 / 600; // Fixed timestep for game logic (vCPU Speed)
    const double renderDt = 1.0 / static_cast<double>(FPS_Limit); // Fixed timestep for rendering

    auto currentTime = std::chrono::steady_clock::now();
    auto accumulator = std::chrono::duration<double>(0.0);
    auto renderAccumulator = std::chrono::duration<double>(0.0);

    while (mWindow.isOpen()) {
        auto newTime = std::chrono::steady_clock::now();
        const std::chrono::duration<double> frameTime = newTime - currentTime;

        currentTime = newTime;

        accumulator += frameTime;
        renderAccumulator += frameTime;

        while (accumulator.count() >= dt) {
            ticks++;

            processEvents();
            update(t, dt);

            t += dt;
            accumulator -= std::chrono::duration<double>(dt);
        }

        while (renderAccumulator.count() >= renderDt) {
            render(t);
            frames++;

            renderAccumulator -= std::chrono::duration<double>(renderDt);
        }

        if (frameClock.getElapsedTime().asSeconds() >= 1.f) {
            frameClock.restart();
            FPS = frames;
            TPS = ticks;
            std::cout << "FPS: " << frames << " TPS: " << ticks << std::endl;
            frames = 0;
            ticks = 0;
        }
    }
}


void Window::handlePlayerInput(const sf::Keyboard::Key key, const bool isPressed) {
    if (key == sf::Keyboard::X) {
        cpu.keypad[0] = isPressed;
    } else if (key == sf::Keyboard::Num1) {
        cpu.keypad[1] = isPressed;
    } else if (key == sf::Keyboard::Num2) {
        cpu.keypad[2] = isPressed;
    } else if (key == sf::Keyboard::Num3) {
        cpu.keypad[3] = isPressed;
    } else if (key == sf::Keyboard::Q) {
        cpu.keypad[4] = isPressed;
    } else if (key == sf::Keyboard::W) {
        cpu.keypad[5] = isPressed;
    } else if (key == sf::Keyboard::E) {
        cpu.keypad[6] = isPressed;
    } else if (key == sf::Keyboard::A) {
        cpu.keypad[7] = isPressed;
    } else if (key == sf::Keyboard::S) {
        cpu.keypad[8] = isPressed;
    } else if (key == sf::Keyboard::D) {
        cpu.keypad[9] = isPressed;
    } else if (key == sf::Keyboard::Z) {
        cpu.keypad[0xA] = isPressed;
    } else if (key == sf::Keyboard::C) {
        cpu.keypad[0xB] = isPressed;
    } else if (key == sf::Keyboard::Num4) {
        cpu.keypad[0xC] = isPressed;
    } else if (key == sf::Keyboard::R) {
        cpu.keypad[0xD] = isPressed;
    } else if (key == sf::Keyboard::F) {
        cpu.keypad[0xE] = isPressed;
    } else if (key == sf::Keyboard::V) {
        cpu.keypad[0xF] = isPressed;
    } else if (key == sf::Keyboard::Escape) {
        exit(0);
    }
}


void Window::processEvents() {
    sf::Event event{};
    while (mWindow.pollEvent(event)) {
        switch (event.type) {
            case sf::Event::Resized:
                if (event.size.width < 256) {
                    //set width to 256
                    mWindow.setSize(sf::Vector2u(256, event.size.height));
                }
                if (event.size.height < 132) {
                    //set height to 132
                    mWindow.setSize(sf::Vector2u(mWindow.getSize().x, 132));
                }
                mView = getLetterboxView(mView, event.size.width, event.size.height); // NOLINT(*-narrowing-conversions)
                break;
            case sf::Event::KeyPressed:
                handlePlayerInput(event.key.code, true);
                break;
            case sf::Event::KeyReleased:
                handlePlayerInput(event.key.code, false);
                break;
            case sf::Event::Closed:
                mWindow.close();
                break;
            default:
                break;
        }
    }
}


void Window::update(const double time, const double deltaTime) {
    cpu.cycle();

#ifndef NDEBUG
    //std::cout << "[Update] t: " << time << " dt: " << deltaTime << std::endl;
#endif
}


void Window::render(double time) {
#ifndef NDEBUG
    //std::cout << "[Render] t: " << time << std::endl;
#endif
    mWindow.clear(sf::Color::Black);
    mWindow.setView(mView);

    //Draw 'background'
    sf::RectangleShape bg;
    bg.setSize(sf::Vector2f(66, 34));
    bg.setFillColor(sf::Color::Cyan);
    bg.setPosition(0.0f, 0.0f);
    mWindow.draw(bg);

    //Draw 'game'
    sf::Texture texture;
    if(texture.create(64, 32)) {
        // Little hacky because SFML texture uses RGBA, but CHIP-8 uses 1-bit per pixel (on/off).
        uint8_t pixels[64*32*4]{20};
        for(int i = 0; i < 64*32; i++) {
            if(cpu.video[i] == 1) {
                // White
                pixels[i*4] = 255;
                pixels[i*4+1] = 255;
                pixels[i*4+2] = 255;
                pixels[i*4+3] = 255;
            } else {
                // Grey
                pixels[i*4] = 40;
                pixels[i*4+1] = 40;
                pixels[i*4+2] = 40;
                pixels[i*4+3] = 255;
            }
        }

        texture.update(pixels, 64, 32, 0, 0);
    } else {
        std::cout << "Error: Failed to create texture." << std::endl;
    }


    sf::Sprite sprite(texture);
    sprite.setPosition(1, 1);
    mWindow.draw(sprite);

    //Draw 'Debug/Admin'

    mWindow.display();
}
