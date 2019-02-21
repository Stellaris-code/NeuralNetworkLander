/*
playfield.hpp

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
#ifndef LANDER_HPP
#define LANDER_HPP

#include "playfield.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <SFML/Graphics/Text.hpp>

class LanderPlayField : public PlayField
{
public:
    const sf::Vector2f gravity        = {0.f, 3.f}; // unit/s^2
    const float        thrust_force   = -6.0f; // unit/s^2
    const float        steering_speed = 90.0f; // degrees/s

public:
    LanderPlayField(sf::Vector2i size = {800, 600});

    void  reset() override;
    void  update(float dt) override;
    void  draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    float score() const override;

private:
    void run_nn();
    void apply_forces(float delta_time);
    void move_rocket();
    void animate();
    void check_collisions();

    void calculate_score();

public:
    float thrust { 0 }; // ranges from  0 to 1
    float steer  { 0 }; // ranges from -1 to 1

private:
    sf::Vector2f m_velocity {};
    float        m_angle { 0 };

    sf::Sprite  m_rocket_sprite;
    sf::Texture m_rocket_texture;

    float m_elapsed_time { 0 };
    float m_score { 1 };
    sf::Vector2i m_size;
    sf::Font         m_da_font;
    sf::RectangleShape m_border;
    sf::RectangleShape m_landing_pad;
    mutable sf::Text m_score_text;
};

#endif // PONG_HPP
