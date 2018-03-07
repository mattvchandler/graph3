// graph_disp_input.cpp
// Graphics display mouse & keyboard input

// Copyright 2018 Matthew Chandler

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "graph_disp.hpp"

// main input processing
bool Graph_disp::input()
{
    // state vars
    static std::unordered_map<sf::Keyboard::Key, bool, std::hash<int>> key_lock;
    static sf::Vector2i old_mouse_pos = sf::Mouse::getPosition(glWindow);
    static sf::Clock cursor_delay;
    static sf::Clock zoom_delay;

    // the neat thing about having this in a timeout func is that we
    // don't need to calculate dt for movement controls.
    // it is always (almost) exactly 10ms

    // only process when the window is active and display is focused
    if(dynamic_cast<Gtk::Window *>(get_toplevel())->is_active())
    {
        sf::Vector2i new_mouse_pos = sf::Mouse::getPosition(glWindow);

        if(!has_focus() && new_mouse_pos.x >= 0 && new_mouse_pos.y >= 0 &&
            new_mouse_pos.x < get_allocated_width() && new_mouse_pos.y < get_allocated_height())
        {
            grab_focus();
        }

        if(has_focus())
        {
            // Camera controls

            float mov_scale = 0.1f;

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
            {
                mov_scale *= 2.0f;
            }

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt))
            {
                mov_scale *= 0.1f;
            }

            // orbiting rotational cam
            if(use_orbit_cam)
            {
                // reset
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::R) && !key_lock[sf::Keyboard::R])
                {
                    key_lock[sf::Keyboard::R] = true;
                    reset_cam();
                }
                else if(!sf::Keyboard::isKeyPressed(sf::Keyboard::R))
                    key_lock[sf::Keyboard::R] = false;

                // tilt up
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
                {
                    _orbit_cam.phi += (float)M_PI / 90.0f * mov_scale;
                    invalidate();
                }

                // tilt down
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
                {
                    _orbit_cam.phi -= (float)M_PI / 90.0f * mov_scale;
                    invalidate();
                }

                // rotate clockwise
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
                {
                    _orbit_cam.theta += (float)M_PI / 90.0f * mov_scale;
                    invalidate();
                }

                // rotate counter-clockwise
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
                {
                    _orbit_cam.theta -= (float)M_PI / 90.0f * mov_scale;
                    invalidate();
                }

                // move camera in
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
                {
                    _orbit_cam.r -= mov_scale;
                    invalidate();
                }

                // move camera out
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::E))
                {
                    _orbit_cam.r += mov_scale;
                    invalidate();
                }

                // rotate w/ mouse click & drag
                if(sf::Mouse::isButtonPressed(sf::Mouse::Left))
                {
                    int d_x = new_mouse_pos.x - old_mouse_pos.x;
                    int d_y = new_mouse_pos.y - old_mouse_pos.y;

                    _orbit_cam.theta -= 0.005f * d_x;
                    _orbit_cam.phi -= 0.005f * d_y;

                    invalidate();
                }

                // wrap theta
                if(_orbit_cam.theta > (float)M_PI * 2.0f)
                    _orbit_cam.theta -= (float)M_PI * 2.0f;
                if(_orbit_cam.theta < 0.0f)
                    _orbit_cam.theta += (float)M_PI * 2.0f;

                // clamp phi
                if(_orbit_cam.phi > (float)M_PI)
                    _orbit_cam.phi = (float)M_PI;
                if(_orbit_cam.phi < 0.0f)
                    _orbit_cam.phi = 0.0f;
            }
            else // free camera
            {
                // reset
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::R) && !key_lock[sf::Keyboard::R])
                {
                    key_lock[sf::Keyboard::R] = true;
                    reset_cam();
                }
                else if(!sf::Keyboard::isKeyPressed(sf::Keyboard::R))
                    key_lock[sf::Keyboard::R] = false;

                // move forward
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
                {
                    _cam.translate(mov_scale * _cam.forward());
                    invalidate();
                }

                // move backwards
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
                {
                    _cam.translate(-mov_scale * _cam.forward());
                    invalidate();
                }

                // move left
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
                {
                    _cam.translate(-mov_scale * _cam.right());
                    invalidate();
                }

                // move right
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
                {
                    _cam.translate(mov_scale * _cam.right());
                    invalidate();
                }

                // move up
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
                {
                    _cam.translate(mov_scale * glm::vec3(0.0f, 0.0f, 1.0f));
                    invalidate();
                }

                // move down
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::E))
                {
                    _cam.translate(-mov_scale * glm::vec3(0.0f, 0.0f, 1.0f));
                    invalidate();
                }

                // rotate view with mouse click & drag
                if(sf::Mouse::isButtonPressed(sf::Mouse::Left))
                {
                    int d_x = new_mouse_pos.x - old_mouse_pos.x;
                    int d_y = new_mouse_pos.y - old_mouse_pos.y;

                    _cam.rotate(0.001f * d_y, _cam.right());
                    _cam.rotate(0.001f * d_x, glm::vec3(0.0f, 0.0f, 1.0f));

                    invalidate();
                }
            }

            const int zoom_timeout = 200;

            // zoom in
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Z) && zoom_delay.getElapsedTime().asMilliseconds() >= zoom_timeout)
            {
                _scale *= 2.0f;
                zoom_delay.restart();
                invalidate();
            }

            // zoom out
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::X) && zoom_delay.getElapsedTime().asMilliseconds() >= zoom_timeout)
            {
                _scale *= 0.5f;
                zoom_delay.restart();
                invalidate();
            }

            // move cursor with arrow keys
            if(draw_cursor_flag && _active_graph)
            {
                const int cursor_timeout = 200;
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && cursor_delay.getElapsedTime().asMilliseconds() >= cursor_timeout)
                {
                    _active_graph->move_cursor(Graph::UP);
                    cursor_delay.restart();
                    invalidate();
                }

                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && cursor_delay.getElapsedTime().asMilliseconds() >= cursor_timeout)
                {
                    _active_graph->move_cursor(Graph::DOWN);
                    cursor_delay.restart();
                    invalidate();
                }

                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && cursor_delay.getElapsedTime().asMilliseconds() >= cursor_timeout)
                {
                    _active_graph->move_cursor(Graph::LEFT);
                    cursor_delay.restart();
                    invalidate();
                }

                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && cursor_delay.getElapsedTime().asMilliseconds() >= cursor_timeout)
                {
                    _active_graph->move_cursor(Graph::RIGHT);
                    cursor_delay.restart();
                    invalidate();
                }
            }
        }
        old_mouse_pos = new_mouse_pos;
    }
    return true;
}

// GTK key press handler
bool Graph_disp::key_press(GdkEventKey * e)
{
    // returning true means we don't propagate key presses up - keep them in this widget
    // (and disregard them entirely because we're using SFML for handling the keyboard, not GTK)
    return true;
}
