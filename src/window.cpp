// window.cpp
// Copyright 2013 Matthew Chandler
// windowing code. Using GTK to create the window, SFML to do openGL graphics.

#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <gtkmm.h>

#include <gdk/gdk.h>
#include <gdkmm/general.h>
#include <gdk/gdkx.h>


class SFMLWidget : public Gtk::Widget, public sf::Window
{
    protected:
        sf::VideoMode m_vMode;

        virtual void on_size_request(Gtk::Requisition* requisition);
        virtual void on_size_allocate(Gtk::Allocation& allocation);
        virtual void on_map();
        virtual void on_unmap();
        virtual void on_realize();
        virtual void on_unrealize();
        virtual bool on_idle();
        virtual bool on_expose_event(GdkEventExpose* event);

        Glib::RefPtr<Gdk::Window> m_refGdkWindow;
    public:
        SFMLWidget(sf::VideoMode Mode);
        virtual ~SFMLWidget();
};

bool SFMLWidget::on_idle()
{
    if(m_refGdkWindow)
    {
        this->display();
    }

    return true;
}

SFMLWidget::SFMLWidget(sf::VideoMode Mode)
    : sf::Window(Mode, "")
{
    set_flags(Gtk::NO_WINDOW); //Makes this behave like an interal object rather then a parent window.
    Glib::signal_idle().connect( sigc::mem_fun(*this, &SFMLWidget::on_idle) );
}

SFMLWidget::~SFMLWidget()
{
}

void SFMLWidget::on_size_request(Gtk::Requisition* requisition)
{
    *requisition = Gtk::Requisition();

    sf::Vector2u v = this->getSize();

    requisition->width = v.x;
    requisition->height = v.y;
}

void SFMLWidget::on_size_allocate(Gtk::Allocation& allocation)
{
    //Do something with the space that we have actually been given:
    //(We will not be given heights or widths less than we have requested, though
    //we might get more)

    this->set_allocation(allocation);

    if(m_refGdkWindow)
    {
        m_refGdkWindow->move_resize(allocation.get_x(), allocation.get_y(), allocation.get_width(), allocation.get_height() );
        this->setSize(sf::Vector2u(allocation.get_width(), allocation.get_height()));
    }
}

void SFMLWidget::on_map()
{
    Gtk::Widget::on_map();
}

void SFMLWidget::on_unmap()
{
    Gtk::Widget::on_unmap();
}

void SFMLWidget::on_realize()
{
    Gtk::Widget::on_realize();

    if(!m_refGdkWindow)
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


        m_refGdkWindow = Gdk::Window::create(get_window(), &attributes,
                GDK_WA_X | GDK_WA_Y);
        unset_flags(Gtk::NO_WINDOW);
        set_window(m_refGdkWindow);

        //set colors
        modify_bg(Gtk::STATE_NORMAL , Gdk::Color("red"));
        modify_fg(Gtk::STATE_NORMAL , Gdk::Color("blue"));

        //make the widget receive expose events
        m_refGdkWindow->set_user_data(gobj());

        this->sf::Window::create((GDK_WINDOW_XID(m_refGdkWindow->gobj())));
    }
}

void SFMLWidget::on_unrealize()
{
  m_refGdkWindow.clear();

  //Call base class:
  Gtk::Widget::on_unrealize();
}

bool SFMLWidget::on_expose_event(GdkEventExpose* event)
{
    if(m_refGdkWindow)
    {
        glClearColor(0.25f, 0.25f, 0.25f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        this->display();
    }

    return true;
}

int main(int argc, char* argv[])
{
    Gtk::Main kit(argc, argv); //Initialize Gtk

    Gtk::Window window; //The GTK window will be our top level Window

    //Our RenderWindow will never be below  640x480 (unless we explicitly change it)
    //but it may be more then that
    SFMLWidget ourRenderWindow(sf::VideoMode(640, 480));

    // Doesn't draw the renderWindow but makes it so it will draw when we add it to the window
    ourRenderWindow.show();

    //VBox is a vertical box, we're going to pack our render window and a button in here
    Gtk::VBox ourVBox;

    Gtk::Button ourButton("Hello I do nothing"); //Just a clickable button, it won't be doing anything
    ourButton.show();

    ourVBox.pack_start(ourRenderWindow); //Add ourRenderWindow to the top of the VBox

    //PACK_SHRINK makes the VBox only allocate enough space to show the button and nothing more
    ourVBox.pack_start(ourButton, Gtk::PACK_SHRINK);
    ourVBox.show();

    window.add(ourVBox); //Adds ourVBox to the window so it (and it's children) can be drawn

    Gtk::Main::run(window); //Draw the window
    return 0;
}
