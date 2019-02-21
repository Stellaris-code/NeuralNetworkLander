/*
PongPlayField.cpp

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

#include "pong.hpp"

#include <cmath>
#include <cstring>

#include <SFML/Graphics/RenderTarget.hpp>

bool PongPlayField::test_paddle_hit(const sf::Vector2f& ball, const sf::Vector2f& paddle)
{
    return ball.x - ballRadius < paddle.x &&
            ball.y + ballRadius >= paddle.y - paddleSize.y / 2 &&
            ball.y - ballRadius <= paddle.y + paddleSize.y / 2;
}

float PongPlayField::new_angle(const sf::Vector2f& ball, const sf::Vector2f& paddle_center)
{
    float offset = ball.y - paddle_center.y;

    //assert(std::abs(offset) < (paddleSize.y/2));

    float normalized_offset = offset / (paddleSize.y/2);

    return normalized_offset * max_bounce_angle;
}

PongPlayField::PongPlayField(sf::Vector2i size, sf::Color ball_color, sf::Color pad_color)
    : m_size(size)
{
    m_paddle.setSize(paddleSize - sf::Vector2f(3*2, 3*2));
    m_paddle.setOutlineThickness(3*2);
    m_paddle.setOutlineColor(sf::Color::Black);
    m_paddle.setFillColor(pad_color);
    m_paddle.setOrigin(paddleSize / 2.f);

    m_ball.setRadius(ballRadius - 3*2);
    m_ball.setOutlineThickness(3*2);
    m_ball.setOutlineColor(sf::Color::Black);
    m_ball.setFillColor(ball_color);
    m_ball.setOrigin(ballRadius / 2, ballRadius / 2);

    m_border.setSize(sf::Vector2f{m_size} - sf::Vector2f{3, 3});
    m_border.setFillColor(sf::Color::Transparent);
    m_border.setOutlineThickness(3);
    m_border.setOutlineColor(sf::Color::Black);

    m_da_font.loadFromFile("resources/sansation.ttf");
    m_score_text.setFont(m_da_font);
    m_score_text.setCharacterSize(80);
    m_score_text.move(20, 0);

    PongPlayField::reset();
}

void PongPlayField::reset()
{
    nn_init(net, 3, 1, 2, 1);

    // Reset the position of the paddles and ball
    m_paddle.setPosition(10 + paddleSize.x / 2, m_size.y / 2);
    m_ball.setPosition(m_size.x / 2, m_size.y / 2);

    // Reset the ball angle
    do
    {
        // Make sure the ball initial angle is not too much vertical
        ball_angle = (std::rand() % 360) * 2 * M_PI / 360;
    }
    while (std::abs(std::cos(ball_angle)) < 0.7f);

    m_score = 1;
    m_score_text.setFillColor(sf::Color::White);
}

void PongPlayField::update(float deltaTime)
{
    if (!playing())
        return;

    m_score += deltaTime * 10;

    net.inputs[0] = ball_pos().x;
    net.inputs[1] = ball_pos().y;
    net.inputs[2] = paddle_pos().y;

    nn_run(net);

    dir = 1.0 - net.outputs[0]*2;

    // Move the player's paddle
    if (dir > 0 &&
            (m_paddle.getPosition().y - paddleSize.y / 2 > 5.f))
    {
        m_paddle.move(0.f, -paddleSpeed * deltaTime);
    }
    if (dir < 0 &&
            (m_paddle.getPosition().y + paddleSize.y / 2 < m_size.y - 5.f))
    {
        m_paddle.move(0.f, paddleSpeed * deltaTime);
    }

    // Move the ball
    float factor = ballSpeed * deltaTime;
    m_ball.move(std::cos(ball_angle) * factor, std::sin(ball_angle) * factor);

    // Check collisions between the ball and the screen
    if (m_ball.getPosition().x - ballRadius < 0.f)
    {
        // just lost
        m_playing = false;

        float distance = std::abs(m_ball.getPosition().y - m_paddle.getPosition().y);
        m_score -= distance / 4.0f; // networks with paddles far from the ball lose more

        m_score_text.setFillColor(sf::Color::Red);
    }

    // bottom
    if (m_ball.getPosition().y - ballRadius < 0.f)
    {
        ball_angle = -ball_angle;
        m_ball.setPosition(m_ball.getPosition().x, ballRadius + 0.1f);
    }
    // top
    if (m_ball.getPosition().y + ballRadius > m_size.y)
    {
        ball_angle = -ball_angle;
        m_ball.setPosition(m_ball.getPosition().x, m_size.y - ballRadius - 0.1f);
    }

    // Check the collisions between the ball and the paddles
    // Left Paddle
    if (test_paddle_hit(m_ball.getPosition(), m_paddle.getPosition()))
    {
        if (m_ball.getPosition().y > m_paddle.getPosition().y)
            ball_angle = M_PI - ball_angle + (std::rand() % 20) * M_PI / 180;
        else
            ball_angle = M_PI - ball_angle - (std::rand() % 20) * M_PI / 180;

        ball_angle = new_angle(m_ball.getPosition(), m_paddle.getPosition());

        //printf("new angle would be %f\n", new_angle(m_ball.getPosition(), m_paddle.getPosition()) * 180 / pi);

        m_ball.setPosition(m_paddle.getPosition().x + ballRadius + paddleSize.x / 2 + 0.1f, m_ball.getPosition().y);
    }

    // wall bounce
    if (m_ball.getPosition().x + ballRadius > m_size.x)
    {
        ball_angle = -(M_PI + std::fmod((double)std::rand(), max_bounce_angle*2) - max_bounce_angle);

        m_ball.setPosition(m_size.x - ballRadius - paddleSize.x / 2 - 0.1f, m_ball.getPosition().y);
    }

    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "Score : %.3f", m_score);

    m_score_text.setString(buffer);
}

void PongPlayField::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    states.transform *= getTransform();


    m_score_text.setFont(m_da_font);
    if (playing())
    {
        target.draw(m_ball, states);
    }
    target.draw(m_paddle, states);
    target.draw(m_score_text, states);
    target.draw(m_border, states);
}

float PongPlayField::score() const
{
    return m_score;
}
