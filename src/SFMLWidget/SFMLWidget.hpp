// SFMLWidget.h
// GTK widget containing an SFML OpenGL drawing context
// Based on example from SFML github wiki: https://github.com/LaurentGomila/SFML/wiki/Source%3A-GTK-SFMLWidget
// - changed to use SFML window instead of RenderWindow, cleaned up, and tweaked substantially
// Code is public domain

#ifndef SFMLWIDGET_H_INCLUDED
#define SFMLWIDGET_H_INCLUDED

#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>

#include <gtkmm/window.h>
#include <gdkmm/general.h>

#include <SFML/Graphics.hpp>
#include <gtkmm/widget.h>

class SFMLWidget : public Gtk::Widget
{
public:

    SFMLWidget(const sf::VideoMode & mode, const int size_request=-1, const sf::ContextSettings & context_settings = sf::ContextSettings());

    void invalidate();
    void display();

    sf::Window glWindow;

protected:
    virtual void on_size_allocate(Gtk::Allocation& allocation);
    virtual void on_realize();
    virtual void on_unrealize();

    Glib::RefPtr<Gdk::Window> _gdk_window;

    sf::ContextSettings _gl_context_settings;

};

#endif
