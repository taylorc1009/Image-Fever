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
#include <mutex>
#include <thread>
#include <future>
#include <chrono>
#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// example folder to load images
#define IMAGES_DIRECTORY "C:\\Users\\taylo\\source\\repos\\taylorc1009\\Image-Fever\\unsorted"
#define PLACEHOLDER_IMAGE "C:\\Users\\taylo\\source\\repos\\taylorc1009\\Image-Fever\\placeholder.jpg"

namespace fs = std::filesystem;

std::mutex mut;

void loadImageData(std::shared_ptr<std::vector<image>> images, std::string path)
{
    int width, height, n; // n is the number of components that you retrieved from the image. 3 if it's RGB only (all JPG images should be 3) or 4 if it's RGBA (e.g. some PNG images)
    auto imgdata = (char*)stbi_load(path.c_str(), &width, &height, &n, 0);

    std::vector<uint8_t> image_data(imgdata, imgdata + width * height * n);

    std::lock_guard<std::mutex> lock(mut);
    images->push_back(image(path, image_data));

    stbi_image_free(imgdata);
}

void threadSortImagesByHue(std::shared_ptr<std::vector<std::thread>> threadPool, std::shared_ptr<std::vector<image>> images, std::chrono::system_clock::time_point threadingStart) {
    for (std::thread& t : (*threadPool))
        t.join();

    auto stop = std::chrono::system_clock::now();
    auto totalTimeOfThreadPool = stop - threadingStart;
    std::cout << "Calculate Median Hues thread elapsed time (s): " << std::chrono::duration_cast<std::chrono::milliseconds>(totalTimeOfThreadPool).count() / 1000.0 << std::endl;

    threadPool->clear();

    std::cout << "Image Sorting thread started" << std::endl;

    std::sort(images->begin(), images->end(), [](image a, image b) { return a.getMedianHue() < b.getMedianHue(); });

    stop = std::chrono::system_clock::now();
    auto totalTimeOfSort = stop - threadingStart;
    std::cout << "Image Sorting thread elapsed time (s): " << std::chrono::duration_cast<std::chrono::milliseconds>(totalTimeOfThreadPool).count() / 1000.0 << std::endl;
}

void threadCalculateMedianHues(std::shared_ptr<std::vector<std::thread>> threadPool, std::shared_ptr<std::vector<image>> images, std::chrono::system_clock::time_point threadingStart) {
    for (std::thread& t : (*threadPool))
        t.join();

    auto stop = std::chrono::system_clock::now();
    auto totalTimeOfThreadPool = stop - threadingStart;
    std::cout << "Image Loading thread elapsed time (s): " << std::chrono::duration_cast<std::chrono::milliseconds>(totalTimeOfThreadPool).count() / 1000.0 << std::endl;

    threadPool->clear();
    
    std::cout << "Calculate Median Hues thread started" << std::endl;

    for (auto &img : (*images))
        threadPool->push_back(std::thread(&image::calculateMedianHue, std::ref(img)));
    
    std::thread sortByHuesThread(threadSortImagesByHue, threadPool, images, threadingStart);
    sortByHuesThread.join();
}

void threadLoadImages(std::shared_ptr<std::vector<image>> images) {
    std::cout << "Image Loading thread started" << std::endl;

    std::shared_ptr<std::vector<std::thread>> threadPool = std::make_shared<std::vector<std::thread>>();

    auto start = std::chrono::system_clock::now();

    for (auto& p : fs::directory_iterator(IMAGES_DIRECTORY))
        threadPool->push_back(std::thread(loadImageData, images, p.path().u8string()));
    
    std::thread getHuesThread(threadCalculateMedianHues, threadPool, images, start);
    getHuesThread.join();
}

sf::Vector2f ScaleFromDimensions(const sf::Vector2u& textureSize, int screenWidth, int screenHeight)
{
    float scaleX = screenWidth / float(textureSize.x);
    float scaleY = screenHeight / float(textureSize.y);
    float scale = std::min(scaleX, scaleY);
    return { scale, scale };
}

int UIThread(std::shared_ptr<std::vector<image>> images) {
    std::cout << "UI thread started" << std::endl;

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
    if (!texture.loadFromFile((*images)[imageIndex].getPath()))
        return EXIT_FAILURE;
    sf::Sprite sprite(texture);
    // Make sure the texture fits the screen
    sprite.setScale(ScaleFromDimensions(texture.getSize(), gameWidth, gameHeight));

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
                view.setCenter(gameWidth / 2.f, gameHeight / 2.f);
                window.setView(view);
            }

            // Arrow key handling!
            if (event.type == sf::Event::KeyPressed)
            {
                // adjust the image index
                if (event.key.code == sf::Keyboard::Key::Left)
                    imageIndex = (imageIndex + images->size() - 1) % images->size();
                else if (event.key.code == sf::Keyboard::Key::Right)
                    imageIndex = (imageIndex + 1) % images->size();

                if (imageIndex < images->size()) {
                    // get image filename
                    const auto& imageFilename = (*images)[imageIndex].getPath();
                    // set it as the window title 
                    window.setTitle(imageFilename);
                    // ... and load the appropriate texture, and put it in the sprite
                    if (!texture.loadFromFile(imageFilename))
                        texture.loadFromFile((std::string)PLACEHOLDER_IMAGE);
                }
                else
                    texture.loadFromFile((std::string)PLACEHOLDER_IMAGE);

                sprite = sf::Sprite(texture);
                sprite.setScale(ScaleFromDimensions(texture.getSize(), gameWidth, gameHeight));
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

int main()
{
    std::srand(static_cast<unsigned int>(std::time(NULL)));

    std::shared_ptr<std::vector<image>> images = std::make_shared<std::vector<image>>();

    std::future<int> UIFuture = std::async(UIThread, images);

    std::thread loadImagesThread(threadLoadImages, images);

    return UIFuture.get();
}
