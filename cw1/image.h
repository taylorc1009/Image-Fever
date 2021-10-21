#include <mutex>
#include <string>
#include <vector>

class image
{
private:
	std::string path;
	std::vector<uint8_t> imageData;
public:
	image(std::string _path, std::vector<uint8_t> _imageData) : path(_path), imageData(_imageData) {}
	~image() = default;
	[[nodiscard]] std::string getPath() const { return path; }
	[[nodiscard]] std::vector<uint8_t> getImageData() const { return imageData; }
};

