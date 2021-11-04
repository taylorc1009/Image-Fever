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
#include <fstream>
#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// example folder to load images
#define IMAGES_DIRECTORY "C:\\Users\\taylo\\source\\repos\\taylorc1009\\Image-Fever\\unsorted"
#define PLACEHOLDER_IMAGE "C:\\Users\\taylo\\source\\repos\\taylorc1009\\Image-Fever\\placeholder.jpg"

namespace fs = std::filesystem;

std::mutex mut;

void outputTimes(std::shared_ptr<std::vector<std::chrono::milliseconds>> times) {
    std::ofstream csv;
    csv.open("times.csv");

    if (csv.is_open()) {
        std::string CSVString;

        unsigned int i = 1;
        for (std::chrono::milliseconds& time : (*times))
            CSVString.append(std::to_string(i++) + ',' + std::to_string(time.count()) + '\n');

        csv << CSVString;

        csv.close();
    }
    else
        std::cout << "(!) failed to open/create CSV file" << std::endl;
}

void loadImageData(std::shared_ptr<std::vector<image>> images, std::string path)
{
    int width, height, n; // n is the number of components that you retrieved from the image. 3 if it's RGB only (all JPG images should be 3) or 4 if it's RGBA (e.g. some PNG images)
    auto imgdata = (char*)stbi_load(path.c_str(), &width, &height, &n, 0);

    std::vector<uint8_t> image_data(imgdata, imgdata + width * height * n);

    /* This is the only place which the Mutex is applied as the "images" vector is modified here by multiple "loadImageData" threads
     * The thread for calculating the median hues doesn't modify the list, but the objects inside, which all have only one corresponding thread anyway
     * The thread for sorting the vector of images uses C++'s "std::sort", which is only running in parallel of the UI thread as I can't parallelise "std::sort" any further
     */
    std::lock_guard<std::mutex> lock(mut);
    images->push_back(image(path, image_data));

    stbi_image_free(imgdata);
}

void t_sortImagesByHue(std::shared_ptr<std::vector<std::thread>> threadPool, std::shared_ptr<std::vector<image>> images, std::shared_ptr<std::vector<std::chrono::milliseconds>> times, std::chrono::system_clock::time_point threadingStart) {
    std::cout << "Image Sorting thread started" << std::endl;

    std::sort(images->begin(), images->end(), [](image a, image b) { return a.getMedianHue() < b.getMedianHue(); });

    auto stop = std::chrono::system_clock::now();
    auto totalTimeOfSort = stop - threadingStart;

    std::chrono::milliseconds time = std::chrono::duration_cast<std::chrono::milliseconds>(totalTimeOfSort);
    std::cout << "Image Sorting thread elapsed time: " << time.count() / 1000.0 << "s" << std::endl;

    times->push_back(time);
}

void t_calculateMedianHues(std::shared_ptr<std::vector<std::thread>> threadPool, std::shared_ptr<std::vector<image>> images, std::shared_ptr<std::vector<std::chrono::milliseconds>> times, std::chrono::system_clock::time_point threadingStart) {
    std::cout << "Calculate Median Hues thread started" << std::endl;

    for (unsigned int i = 0; i < images->size(); i++) { // minus 1 to leave a thread for the UI
        threadPool->push_back(std::thread(&image::calculateMedianHue, std::ref(images->at(i))));

        // prevents thrashing of the CPU; if the amount of threads in the thread pool is reaching the number of threads in the system hardware, then join each thread until the pool is empty
        if (threadPool->size() >= std::thread::hardware_concurrency() - 2) { // minus 2 instead of 1 to leave one thread for the UI
            for (std::thread& t : (*threadPool))
                t.join();
            threadPool->clear();
        }
    }
    for (std::thread& t : (*threadPool))
        t.join();
    threadPool->clear();

    auto stop = std::chrono::system_clock::now();
    auto totalTimeOfThreadPool = stop - threadingStart;

    std::chrono::milliseconds time = std::chrono::duration_cast<std::chrono::milliseconds>(totalTimeOfThreadPool);
    std::cout << "Calculate Median Hues thread elapsed time: " << time.count() / 1000.0 << "s" << std::endl;

    times->push_back(time);
    
    std::thread sortByHuesThread(t_sortImagesByHue, threadPool, images, times, threadingStart);
    sortByHuesThread.join();
}

void t_loadImages(std::shared_ptr<std::vector<image>> images, std::shared_ptr<std::vector<std::chrono::milliseconds>> times) {
    std::cout << "Image Loading thread started" << std::endl;

    std::shared_ptr<std::vector<std::thread>> threadPool = std::make_shared<std::vector<std::thread>>();

    auto start = std::chrono::system_clock::now();
    
    
    for (fs::directory_iterator dirItr(IMAGES_DIRECTORY), endItr; dirItr != endItr; dirItr++) { // minus 1 to leave a thread for the UI
        /* I tried to combine "std::thread(&image::calculateMedianHue, std::ref(images->at(imagesProcessed)" from "t_calculateMedianHues" with this thread
         * The purpose of this was that an image's median hue could be calculated as soon as the image is loaded
         * But, the class method "image::calculateMedianHue" cannot be invoked until the image is loaded and an object is constructed
         * Because of this, if "image::calculateMedianHue" is invoked here, there is no way to specify which object to invoke it on without waiting until it's loaded (by joining the thread)
         * Therefore, combining "t_calculateMedianHues" with this function wouldn't make a difference as we'd still need to wait
         */
        threadPool->push_back(std::thread(loadImageData, images, dirItr->path().u8string()));

        // prevents thrashing of the CPU; if the amount of threads in the thread pool is reaching the number of threads in the system hardware, then join each thread until the pool is empty
        if (threadPool->size() >= std::thread::hardware_concurrency() - 2) { // minus 2 instead of 1 to leave one thread for the UI
            for (std::thread& t : (*threadPool))
                t.join();
            threadPool->clear();
        }
    }
    for (std::thread& t : (*threadPool))
        t.join();
    threadPool->clear();

    auto stop = std::chrono::system_clock::now();
    auto totalTimeOfThreadPool = stop - start;

    std::chrono::milliseconds time = std::chrono::duration_cast<std::chrono::milliseconds>(totalTimeOfThreadPool);
    std::cout << "Image Loading thread elapsed time: " << time.count() / 1000.0 << "s" << std::endl;

    times->push_back(time);
    
    std::thread getHuesThread(t_calculateMedianHues, threadPool, images, times, start);
    getHuesThread.join();

    outputTimes(times);
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

void sequentialOperations(std::shared_ptr<std::vector<image>> images, std::shared_ptr<std::vector<std::chrono::milliseconds>> times) { // the non-parallelised image loading, convertion, and sorting function
    std::cout << "Sequential Operations function started" << std::endl;
    
    auto start = std::chrono::system_clock::now();
    
    int i = 0;
    for (auto& p : fs::directory_iterator(IMAGES_DIRECTORY))
        loadImageData(images, p.path().u8string());

    auto stop = std::chrono::system_clock::now();
    auto timeElapsed = stop - start;

    std::chrono::milliseconds time = std::chrono::duration_cast<std::chrono::milliseconds>(timeElapsed);
    std::cout << "Sequential Operations function, Load Images elapsed time (s): " << time.count() / 1000.0 << std::endl;

    times->push_back(time);


    for (auto& img : (*images))
        img.calculateMedianHue();

    stop = std::chrono::system_clock::now();
    timeElapsed = stop - start;

    time = std::chrono::duration_cast<std::chrono::milliseconds>(timeElapsed);
    std::cout << "Sequential Operations function, Calculate Median Hues elapsed time (s): " << time.count() / 1000.0 << std::endl;

    times->push_back(time);
    
    std::sort(images->begin(), images->end(), [](image a, image b) { return a.getMedianHue() < b.getMedianHue(); });

    stop = std::chrono::system_clock::now();
    timeElapsed = stop - start;

    time = std::chrono::duration_cast<std::chrono::milliseconds>(timeElapsed);
    std::cout << "Sequential Operations function, Image Sorting elapsed time (s): " << time.count() / 1000.0 << std::endl;

    times->push_back(time);

    outputTimes(times);
}

int main()
{
    std::srand(static_cast<unsigned int>(std::time(NULL)));

    std::shared_ptr<std::vector<std::chrono::milliseconds>> times = std::make_shared<std::vector<std::chrono::milliseconds>>(); // a vector to store the list of times that will be outputted to a CSV

    std::shared_ptr<std::vector<image>> images = std::make_shared<std::vector<image>>();

    std::future<int> UIFuture = std::async(UIThread, images);

    std::thread loadImagesThread(t_loadImages, images, times);
    //std::thread sequentialOperationsThread(sequentialOperations, images, times);

    return UIFuture.get();
}
