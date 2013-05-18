// Window creation code
// Copyright Matthew Chandler 2013

#include <iostream>

#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>

#include <gtkmm/application.h>
#include <gtkmm/box.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/label.h>
#include <gtkmm/window.h>

#include <gtk/gtk.h>
#include <gdk/gdkx.h>

class My_draw: public Gtk::DrawingArea
{
public:
    friend class My_win;
    My_draw()
    {
        set_can_focus(true);
        set_double_buffered(false);
        set_size_request(400, 400);
        s_win.setFramerateLimit(60);
    }
    void on_expose()
    {
        glClearColor(0.25f, 0.25f, 0.25f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        s_win.display();
        queue_draw();
    }
    sf::Window s_win;
};
class My_win: public Gtk::Window
{
public:
    My_win()
    {
        set_default_size(800, 600);
        set_title("Graph3");

        add(main_box);
        main_box.pack_start(draw);
        show_all_children();

        draw.realize();

        draw.s_win.create(GDK_WINDOW_XID(draw.get_window()->gobj()));
    }

    Gtk::VBox main_box;
    My_draw draw;
};

int main(int argc, char ** argv)
{
    // set up and launch a GTK window
    Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, "graph3.graph3");

    My_win win;

    return app->run(win);
}
