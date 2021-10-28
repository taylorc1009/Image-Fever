#include <mutex>
#include <string>
#include <vector>
#include <math.h>
#include <iostream>
#include <SFML/Graphics.hpp>

typedef struct {
	double r; // a fraction between 0 and 1
	double g; // a fraction between 0 and 1
	double b; // a fraction between 0 and 1
} rgb;

typedef struct {
	double h; // angle in degrees
	double s; // a fraction between 0 and 1
	double v; // a fraction between 0 and 1
} hsv;

class image
{
private:
	std::string path;
	std::vector<uint8_t> imageData;
	double medianHue = 0;
	hsv rgb2hsv(rgb in);
public:
	image(std::string _path, std::vector<uint8_t> _imageData) : path(_path), imageData(_imageData) {}
	~image() = default;
	[[nodiscard]] std::string getPath() const { return path; }
	[[nodiscard]] std::vector<uint8_t> getImageData() const { return imageData; }
	[[nodiscard]] double getMedianHue();
	void calculateMedianHue();
};

