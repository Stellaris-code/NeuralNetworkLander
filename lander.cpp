/*
LanderPlayField.cpp

Copyright (c) 10 Yann BOUCHER (yann)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include "lander.hpp"

#include <cmath>
#include <cstring>
#include <cassert>

#include <SFML/Graphics/RenderTarget.hpp>

inline double to_radians(double degrees)
{
    return degrees * M_PI / 180.0;
}

LanderPlayField::LanderPlayField(sf::Vector2i size)
    : m_size(size)
{
    m_rocket_texture.loadFromFile("resources/lander_spritesheet.png");

    m_rocket_sprite.setTexture(m_rocket_texture);
    m_rocket_sprite.setTextureRect({40, 57, 20, 25});
    m_rocket_sprite.setScale(5.f, 5.f);
    m_rocket_sprite.setOrigin(20/2.f, 25/2.f);

    m_border.setSize(sf::Vector2f{m_size} - sf::Vector2f{3, 3});
    m_border.setFillColor(sf::Color::Transparent);
    m_border.setOutlineThickness(3*2);
    m_border.setOutlineColor(sf::Color::Black);

    m_landing_pad.setSize({200, 15});
    m_landing_pad.setPosition({size.x/2.f - 200/2.f, (float)size.y - 15.f - 1});
    m_landing_pad.setFillColor(sf::Color(100, 100, 100));
    m_landing_pad.setOutlineThickness(3);
    m_landing_pad.setOutlineColor(sf::Color::White);

    m_da_font.loadFromFile("resources/sansation.ttf");
    m_score_text.setFont(m_da_font);
    m_score_text.setCharacterSize(80);
    m_score_text.move(20, 0);

    LanderPlayField::reset();
}

void LanderPlayField::reset()
{
    // Inputs : algebraic_pad_distance_x, y, vert_speed, horiz_speed, angle, steer, thrust
    // Outputs : thrust, steer
    nn_init(net, 5, 1, 4, 2);

    m_velocity = {0, 0};
    m_angle = 0;
    m_elapsed_time = 0;
    m_score = 1;
    m_score_text.setFillColor(sf::Color::White);
    m_border.setFillColor(sf::Color::Transparent);

    m_rocket_sprite.setPosition(sf::Vector2f{m_size / 2} - sf::Vector2f{m_size.x/4.f, m_size.y/4.f});

    m_angle = 45;

    thrust = 0.2;
    steer  = 0;
}

void LanderPlayField::run_nn()
{
    //net.inputs[0] = m_rocket_sprite.getPosition().x;
    net.inputs[0] = m_rocket_sprite.getPosition().y;
    net.inputs[1] = m_velocity.x;
    net.inputs[2] = m_velocity.y;
    net.inputs[3] = m_angle;
    net.inputs[4] = m_rocket_sprite.getPosition().x - (m_landing_pad.getPosition().x+m_landing_pad.getLocalBounds().width/2.f);
    //net.inputs[5] = steer;
    //net.inputs[6] = thrust;

    nn_run(net);

    thrust = net.outputs[0];
    steer  = 1 - net.outputs[1] * 2; // normalize to [-1; 1]
}

void LanderPlayField::update(float delta_time)
{
    if (!playing())
        return;

    m_elapsed_time += delta_time;

    if (m_elapsed_time > 10.f) // kill agents that take more than 10 sec to land
    {
        m_playing = false;
        return;
    }

    run_nn();

    apply_forces(delta_time);
    move_rocket();
    check_collisions();

    animate();

    // shorter landings get more score; penalize time spent in weird attitudes
    if (std::abs(m_angle) > 20)
        m_score -= delta_time * std::abs(m_angle) * 2;
    else
        m_score -= delta_time * 5;

    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "Score : %.3f", m_score);

    m_score_text.setString(buffer);
}

void LanderPlayField::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    states.transform *= getTransform();

    if (playing())
    {
        //target.draw(m_ball, states);
    }
    target.draw(m_rocket_sprite, states);
    target.draw(m_landing_pad, states);
    target.draw(m_score_text, states);
    target.draw(m_border, states);
}

void LanderPlayField::animate()
{
    const float anim_duration = 0.20f;

    float time_modulus = std::fmod(m_elapsed_time, anim_duration);
    if (time_modulus < anim_duration/2)
    {
        m_rocket_sprite.setTextureRect({20, 57, 20, 25});
    }
    else
    {
        m_rocket_sprite.setTextureRect({40, 57, 20, 25});
    }
}

void LanderPlayField::check_collisions()
{
    auto bounding_box = m_rocket_sprite.getGlobalBounds();

    if (bounding_box.top > 157*4+60 || bounding_box.left < 0 || bounding_box.left + bounding_box.width > m_size.x ||
            bounding_box.top < 0) // hardcoded for your pleasure
    {
        m_playing = false;
        calculate_score();
    }
}

void LanderPlayField::calculate_score()
{
    //m_score = 0;

    // score based on impact velocity
    double velocity = std::sqrt(m_velocity.x*m_velocity.x + m_velocity.y*m_velocity.y);
    //m_score += 1000 - velocity*100;

    float angle_delta = std::abs(m_angle - 0);


    if (angle_delta > 3)
        m_score -= angle_delta; // penalize tilted landing
    else
        m_score += (3 - angle_delta);

    if (m_velocity.y <= 3)
        m_score += (3-velocity)*100; // reward slow vertical landings
    else
        m_score -= velocity*20;

    // on map
    if (m_rocket_sprite.getPosition().x >= m_landing_pad.getPosition().x + 30 &&
            m_rocket_sprite.getPosition().x <= m_landing_pad.getPosition().x+m_landing_pad.getLocalBounds().width)
    {
        m_score += 200; // yayy !
        m_border.setFillColor(sf::Color(0, 255, 0, 100));
    }
    else
    {
        // distance to the center of the pad
        m_score -= std::abs(m_rocket_sprite.getPosition().x - (m_landing_pad.getPosition().x+m_landing_pad.getLocalBounds().width/2.f));
        m_border.setFillColor(sf::Color(255, 0, 0, 100));
    }

    // didn't touch the ground
    if (m_rocket_sprite.getGlobalBounds().top < 157*4+60)
    {
        m_score = -100000 + m_rocket_sprite.getPosition().y; // reward lowest individuals
    }
}

float LanderPlayField::score() const
{
    return m_score;
}

void LanderPlayField::apply_forces(float delta_time)
{
    assert(thrust >= 0 && thrust <= 1);


    sf::Vector2f direction;
    direction.x = cos(to_radians(m_angle + 90));
    direction.y = sin(to_radians(m_angle + 90));

    m_velocity += gravity * delta_time + direction * thrust_force * thrust * delta_time;

    m_angle += steering_speed * steer * delta_time;
}

void LanderPlayField::move_rocket()
{
    m_rocket_sprite.setRotation(m_angle);
    m_rocket_sprite.move(m_velocity);
}
