#include "Window.h"

#ifndef NDEBUG
#include <iostream>
#endif //NDEBUG

int main() {
#ifndef NDEBUG
    std::cerr << "WARNING: Running debug build, expect reduced performance." << std::endl;
#endif //NDEBUG

    Window window;
    window.loop();
}
