// window.cpp
// Window creation code
// Copyright Matthew Chandler 2013

#include <SFML/Window.hpp>

#include <SFML/OpenGL.hpp>

int main(int argc, char ** argv)
{
    sf::Window win(sf::VideoMode(800, 600), "Graph 3", sf::Style::Default);

    glClearColor(0.25f, 0.25f, 0.25f, 0.0f);

    // main SFML event loop
    bool running = true;
    while(running)
    {
        sf::Event e;
        while(win.pollEvent(e))
        {
            switch(e.type)
            {
            case sf::Event::Closed:
                running = false;
                win.close();
                break;
            default:
                break;
            }
        }

        // display code
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        win.display();
    }


    win.close();

    return EXIT_SUCCESS;
}
