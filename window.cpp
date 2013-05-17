// window.cpp
// Window creation code
// Copyright Matthew Chandler 2013

#include <SFML/Window.hpp>

int main(int argc, char ** argv)
{
    sf::Window win(sf::VideoMode(800, 600), "Graph 3", sf::Style::Default);

    // main SFML event loop
    while(win.isOpen())
    {
        sf::Event e;
        while(win.pollEvent(e))
        {
            switch(e.type)
            {
            case sf::Event::Closed:
                win.close();
                break;
            default:
                break;
            }
        }
    }

    return EXIT_SUCCESS;
}
