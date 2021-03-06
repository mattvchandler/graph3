// SFMLWidget.cpp
// GTK widget containing an SFML OpenGL drawing context
// Based on example from SFML github wiki: https://github.com/LaurentGomila/SFML/wiki/Source%3A-GTK-SFMLWidget
// - changed to use SFML window instead of RenderWindow, cleaned up, and tweaked substantially
// Code is public domain

#include "SFMLWidget.hpp"

// Tested on Linux Mint 12.4 and Windows 7
#if defined(SFML_SYSTEM_WINDOWS)

#include <gdk/gdkwin32.h>
#define GET_WINDOW_HANDLE_FROM_GDK GDK_WINDOW_HWND

#elif defined(SFML_SYSTEM_LINUX) || defined(SFML_SYSTEM_FREEBSD)

#include <gdk/gdkx.h>
#define GET_WINDOW_HANDLE_FROM_GDK GDK_WINDOW_XID

#elif defined(SFML_SYSTEM_MACOS)

#error Note: You have to figure out an analogue way to access the handle of the widget on a Mac-System

#else

#error Unsupported Operating System

#endif

#include <iostream>

SFMLWidget::SFMLWidget(const sf::VideoMode & mode, int size_request, const sf::ContextSettings & context_settings): _gl_context_settings(context_settings)
{
    if(size_request<=0)
        size_request = std::max<int>(1, std::min<int>(mode.width, mode.height) / 2);
    set_size_request(size_request, size_request);

    set_has_window(false); // Makes this behave like an interal object rather then a parent window.
}

void SFMLWidget::invalidate()
{
    if(_gdk_window)
    {
        _gdk_window->invalidate(true);
    }
}

void SFMLWidget::display()
{
    if(_gdk_window)
    {
        glWindow.display();
    }
}

void SFMLWidget::on_size_allocate(Gtk::Allocation& allocation)
{
    // Do something with the space that we have actually been given:
    // (We will not be given heights or widths less than we have requested, though
    // we might get more)

    this->set_allocation(allocation);

    if(_gdk_window)
    {
        _gdk_window->move_resize(allocation.get_x(),
                                    allocation.get_y(),
                                    allocation.get_width(),
                                    allocation.get_height());
        glWindow.setSize(sf::Vector2u(allocation.get_width(),
                                          allocation.get_height()));
    }
}

void SFMLWidget::on_realize()
{
    Gtk::Widget::on_realize();

    if(!_gdk_window)
    {
        //Create the GdkWindow:
        GdkWindowAttr attributes;
        memset(&attributes, 0, sizeof(attributes));

        Gtk::Allocation allocation = get_allocation();

        //Set initial position and size of the Gdk::Window:
        attributes.x = allocation.get_x();
        attributes.y = allocation.get_y();
        attributes.width = allocation.get_width();
        attributes.height = allocation.get_height();

        attributes.event_mask = get_events () | Gdk::EXPOSURE_MASK;
        attributes.window_type = GDK_WINDOW_CHILD;
        attributes.wclass = GDK_INPUT_OUTPUT;


        _gdk_window = Gdk::Window::create(get_window(), &attributes,
                GDK_WA_X | GDK_WA_Y);
        set_has_window(true);
        set_window(_gdk_window);

        // transparent background
        #if GTK_VERSION_GE(3, 0)
            this->unset_background_color();
        #else
            this->get_window()->set_back_pixmap(Glib::RefPtr<Gdk::Pixmap>());
        #endif

        this->set_double_buffered(false);

        //make the widget receive expose events
        _gdk_window->set_user_data(gobj());

        glWindow.create(static_cast<sf::WindowHandle>(GET_WINDOW_HANDLE_FROM_GDK(_gdk_window->gobj())), _gl_context_settings);
    }
}

void SFMLWidget::on_unrealize()
{
  _gdk_window.clear();

  //Call base class:
  Gtk::Widget::on_unrealize();
}

