
#include <SFML/Graphics.hpp>
#include <iostream>
#include "Player.h"
#include "Map.h"

int main()
{
    // - - - - - - - - - - - - Initialise - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    sf::ContextSettings settings;
    // settings.antiAliasingLevel = 8;
    sf::RenderWindow window(sf::VideoMode({ 244,288 }), "DIG DUG", sf::Style::Resize, sf::State::Windowed, settings);
    // - - - - - - - - - - - - Initialise - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    sf::Clock clock;

    Map map;
    Player player(&map);
    map.loadFromFile("Assets/Map/test.rmap");
    player.Initialise();

    // - - - - - - - - - - - - Load - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    player.Load();
    map.printInfo();



    while (window.isOpen())
    {
        // - - - - - - - - - - - - Update - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
        sf::Time deltaTimeTimer = clock.restart();
        float deltaTime = deltaTimeTimer.asSeconds(); // Convert milliseconds to seconds

        sf::Vector2f mousePosition = window.mapPixelToCoords(sf::Mouse::getPosition(window));


        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }


        // - - - - - - - - - - - - Update - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
        
        player.Update(deltaTime);


        // - - - - - - - - - - - - Draw - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
        window.clear(sf::Color::Black);
        map.draw(window);
        player.Draw(window);

        window.display();


        // - - - - - - - - - - - - Draw - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    }
}