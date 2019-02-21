#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <cassert>
#include <memory>

#include "network.hpp"
#include "common.hpp"
#include "genetic_operations.hpp"
#include "pong.hpp"
#include "lander.hpp"

sf::Color paddle_colors[6] =
{
    sf::Color::White,
    sf::Color::Blue,
    sf::Color::Red,
    sf::Color::Green,
    sf::Color::Yellow,
    sf::Color::Cyan
};

const size_t fields_column_count = 5;
const size_t fields_line_count   = 4;

int main()
{
    int generation = 0;

    int field_width  = gameWidth;
    int field_height = gameHeight;

    // Load the text font
    sf::Font font;
    if (!font.loadFromFile("resources/sansation.ttf"))
        return EXIT_FAILURE;

    std::vector<sf::Text> field_numbers(fields_line_count*fields_column_count);
    for (size_t i { 0 }; i < fields_column_count; ++i)
    {
        for (size_t j { 0 }; j < fields_line_count; ++j)
        {
            auto& number = field_numbers[j + i*fields_line_count];
            number.setCharacterSize(100);
            number.setFillColor(sf::Color(200, 200, 200, 100));
            number.setFont(font);
            number.setString(std::to_string(i + j*fields_column_count + 1));
            auto textRect = number.getLocalBounds();
            number.setOrigin(textRect.left + textRect.width/2.0f,
                             textRect.top  + textRect.height/2.0f);
            number.move((float)i*field_width/fields_column_count + field_width/fields_column_count/2.f, (float)j*field_height/fields_line_count + field_height/fields_column_count/2.f);

        }
    }

    std::vector<PlayField*> fields;
    for (size_t i { 0 }; i < fields_column_count*fields_line_count; ++i)
    {
        //fields.emplace_back(new PongPlayField(sf::Vector2i{field_width, field_height}, sf::Color::White, paddle_colors[i%6]));
        fields.emplace_back(new LanderPlayField(sf::Vector2i{field_width, field_height}));
    }

    for (size_t i { 0 }; i < fields_column_count; ++i)
    {
        for (size_t j { 0 }; j < fields_line_count; ++j)
        {
            fields[j + i*fields_line_count]->move((float)i*field_width/fields_column_count, (float)j*field_height/fields_line_count);
            fields[j + i*fields_line_count]->setScale(1.f/fields_column_count, 1.f/fields_line_count);
        }
    }

    //    fields[0]->move(0, 0);
    //    fields[1]->move(0, field_height);
    //    fields[2]->move(0, field_height*2);
    //    fields[3]->move(field_width, 0);
    //    fields[4]->move(field_width, field_height);
    //    fields[5]->move(field_width, field_height*2);

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Create the window of the application
    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight, 32), "SFML Pong",
                            sf::Style::Titlebar | sf::Style::Close);
    window.setVerticalSyncEnabled(true);

    // Initialize the pause message
    sf::Text pauseMessage;
    pauseMessage.setFont(font);
    pauseMessage.setCharacterSize(40);
    pauseMessage.setPosition(170.f, 150.f);
    pauseMessage.setFillColor(sf::Color::White);

    sf::Text genMessage;
    genMessage.setFont(font);
    genMessage.setCharacterSize(40);
    genMessage.setPosition(20.f, gameHeight);
    genMessage.setFillColor(sf::Color::White);

    bool space_pressed = false;

    sf::Clock clock;

    while (window.isOpen())
    {
        space_pressed = false;
        // Handle events
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Window closed or escape key pressed: exit
            if ((event.type == sf::Event::Closed) ||
                    ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape)))
            {
                window.close();
                break;
            }

            // Space key pressed: play
            if (((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Space)))
                space_pressed = true;

            // Window size changed, adjust view appropriately
            if (event.type == sf::Event::Resized)
            {
                sf::View view;
                view.setSize(windowWidth, windowHeight);
                view.setCenter(windowWidth/2.f, windowHeight/2.f);
                window.setView(view);
            }
        }

        if (space_pressed ||
                std::all_of(fields.begin(), fields.end(), [](const PlayField* field) { return !field->playing(); }))
        {
            ++generation;

            genMessage.setString(L"Génération : " + std::to_wstring(generation));

            std::vector<const PlayField*> field_ptrs;
            field_ptrs.reserve(fields.size());
            for (const auto* field : fields)
                field_ptrs.emplace_back(field);

            std::sort(field_ptrs.begin(), field_ptrs.end(), [](const PlayField* lhs, const PlayField* rhs)
            { return lhs->score() > rhs->score(); });

            //field_ptrs = select(field_ptrs, 2);

            auto offspring = breed(field_ptrs[0]->net, field_ptrs[1]->net, fields.size()); // the parents

            for (size_t i { 0 }; i < fields.size(); ++i)
            {
                auto* field = fields[i];

                field->reset();
                // (re)start the game
                field->set_playing(true);
                clock.restart();
            }

            // replace the nets but the 3 last with the offspring
            for (size_t i { 0 }; i < fields.size()-3; ++i)
            {
                fields[i]->net = offspring[i];
            }
        }

        for (size_t i { 0 }; i < fields.size(); ++i)
        {
            auto& field = fields[i];
            if (field->playing())
            {
                float deltaTime = clock.getElapsedTime().asSeconds();

                field->update(deltaTime);
            }
        }

        clock.restart();

        // Clear the window
        window.clear(sf::Color(50, 200, 50));

        for (const auto& num : field_numbers)
            window.draw(num);

        if (std::any_of(fields.begin(), fields.end(), [](const PlayField* field)
        { return field->playing(); })
                )
        {
            for (size_t i { 0 }; i < fields.size(); ++i)
            {
                window.draw(*fields[i]);
            }
        }
        else
        {

        }

        window.draw(genMessage);

        // Display things on screen
        window.display();
    }

    return EXIT_SUCCESS;
}

