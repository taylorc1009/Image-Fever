// This is a chopped Pong example from SFML examples

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics.hpp>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <filesystem>
#include <unordered_map>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace fs = std::filesystem;

sf::Vector2f ScaleFromDimensions(const sf::Vector2u& textureSize, int screenWidth, int screenHeight)
{
    float scaleX = screenWidth / float(textureSize.x);
    float scaleY = screenHeight / float(textureSize.y);
    float scale = std::min(scaleX, scaleY);
    return { scale, scale };
}

int main()
{
    std::srand(static_cast<unsigned int>(std::time(NULL)));

    // example folder to load images
    constexpr char* image_folder = "C:\\Users\\taylo\\source\\repos\\taylorc1009\\Image-Fever\\unsorted";
    std::vector<std::string> imageFilenames;
    std::unordered_map<std::string, std::vector<uint8_t>> images;

	for (auto& p : fs::directory_iterator(image_folder)) {
        std::string filePath = p.path().u8string();
        imageFilenames.push_back(filePath);

		int width, height, n; // n is the number of components that you retrieved from the image. 3 if it's RGB only (all JPG images should be 3) or 4 if it's RGBA (e.g. some PNG images)
		auto imgdata = (char*)stbi_load(filePath.c_str(), &width, &height, &n, 0);
        std::vector<uint8_t> image_data(imgdata, imgdata + width * height * n);
        images[filePath] = image_data; //for some reason I can't merge this line with the line above, by bringing the "image_data" constructor here, as it cannot resolve the "image_data" symbol
        stbi_image_free(imgdata);
    }

    // Define some constants
    const float pi = 3.14159f;
    const int gameWidth = 800;
    const int gameHeight = 600;

    int imageIndex = 0;

    // Create the window of the application
    sf::RenderWindow window(sf::VideoMode(gameWidth, gameHeight, 32), "Image Fever",
                            sf::Style::Titlebar | sf::Style::Close);
    window.setVerticalSyncEnabled(true);

    // Load an image to begin with
    sf::Texture texture;
    if (!texture.loadFromFile(imageFilenames[imageIndex]))
        return EXIT_FAILURE;
    sf::Sprite sprite (texture);
    // Make sure the texture fits the screen
    sprite.setScale(ScaleFromDimensions(texture.getSize(),gameWidth,gameHeight));

    sf::Clock clock;
    while (window.isOpen())
    {
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
            
            // Window size changed, adjust view appropriately
            if (event.type == sf::Event::Resized)
            {
                sf::View view;
                view.setSize(gameWidth, gameHeight);
                view.setCenter(gameWidth/2.f, gameHeight/2.f);
                window.setView(view);
            }

            // Arrow key handling!
            if (event.type == sf::Event::KeyPressed)
            {
                // adjust the image index
                if (event.key.code == sf::Keyboard::Key::Left)
                    imageIndex = (imageIndex + imageFilenames.size() - 1) % imageFilenames.size();
                else if (event.key.code == sf::Keyboard::Key::Right)
                    imageIndex = (imageIndex + 1) % imageFilenames.size();
                // get image filename
                const auto& imageFilename = imageFilenames[imageIndex];
                // set it as the window title 
                window.setTitle(imageFilename);
                // ... and load the appropriate texture, and put it in the sprite
                if (texture.loadFromFile(imageFilename))
                {
                    sprite = sf::Sprite(texture);
                    sprite.setScale(ScaleFromDimensions(texture.getSize(), gameWidth, gameHeight));
                }
            }
        }

        // Clear the window
        window.clear(sf::Color(0, 0, 0));
        // draw the sprite
        window.draw(sprite);
        // Display things on screen
        window.display();
    }

    return EXIT_SUCCESS;
}
