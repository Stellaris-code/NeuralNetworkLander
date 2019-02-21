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
#ifndef PONG_HPP
#define PONG_HPP

#include "playfield.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>

#include <SFML/Graphics/Text.hpp>

// TODO : récompenser le mouvement

class PongPlayField : public PlayField
{
public:
    const sf::Vector2f paddleSize { 25.f*2, 100.f*2 };
    const float ballRadius = 10.f*2;
    const float max_bounce_angle = 3.1415926/4;
    // Define the paddles properties
    const float paddleSpeed = 500.f*2;
    const float ballSpeed   = 600.f*2;

public:
    PongPlayField(sf::Vector2i size = {800, 600}, sf::Color ball_color = sf::Color::White, sf::Color pad_color = sf::Color(100, 100, 200));

    void  reset() override;
    void  update(float dt) override;
    void  draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    float score() const override;

    sf::Vector2f ball_pos() const
    { return m_ball.getPosition(); }
    sf::Vector2f paddle_pos() const
    { return m_paddle.getPosition(); }

private:
    bool test_paddle_hit(const sf::Vector2f& ball, const sf::Vector2f& paddle);
    float new_angle(const sf::Vector2f& ball, const sf::Vector2f& paddle_center);

public:
    double dir { 0 };
    float ball_angle { 0 };

private:
    float m_score { 1 };
    sf::Vector2i m_size;
    sf::RectangleShape m_paddle;
    sf::CircleShape m_ball;
    sf::RectangleShape m_border;
    sf::Font m_da_font;
    mutable sf::Text m_score_text;
};

#endif // PONG_HPP
