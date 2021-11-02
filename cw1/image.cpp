#include "image.h"

hsv image::rgb2hsv(const rgb in)
{
    hsv out;
    double min, max, delta;

    min = in.r < in.g ? in.r : in.g;
    min = min < in.b ? min : in.b;

    max = in.r > in.g ? in.r : in.g;
    max = max > in.b ? max : in.b;

    out.v = max; // v
    delta = max - min;
    if (delta < 0.00001)
    {
        out.s = 0;
        out.h = 0; // undefined, maybe nan?
        return out;
    }
    if (max > 0.0) { // NOTE: if Max is == 0, this divide would cause a crash
        out.s = (delta / max); // s
    }
    else {
        // if max is 0, then r = g = b = 0              
        // s = 0, h is undefined
        out.s = 0.0;
        out.h = NAN; // its now undefined
        return out;
    }
    if (in.r >= max) // > is bogus, just keeps compilor happy
        out.h = (in.g - in.b) / delta; // between yellow & magenta
    else
        if (in.g >= max)
            out.h = 2.0 + (in.b - in.r) / delta; // between cyan & yellow
        else
            out.h = 4.0 + (in.r - in.g) / delta; // between magenta & cyan

    out.h *= 60.0; // degrees

    if (out.h < 0.0)
        out.h += 360.0;

    return out;
}

void image::calculateMedianHue() {
    std::vector<uint8_t> imgData = this->getImageData();
    std::vector<double> HuesList = std::vector<double>();

    /* This for loop is very time consuming as there are a lot of pixels in each image
     * However, there is no point parallelising it as that would mean we could only have one "image::calculateMedianHue()" thread running at a time to prevent CPU thrashing
     * Therefore, in theory, if we did parallelise it, it would take just as long, if not longer to finish
     */
    for (unsigned int i = 0; i < imgData.size(); i += 3) {
        rgb pixelRGB;

        pixelRGB.r = imgData[i];
        pixelRGB.g = imgData[i + 1];
        pixelRGB.b = imgData[i + 2];

        HuesList.push_back(rgb2hsv(pixelRGB).h);
    }

    std::sort(HuesList.begin(), HuesList.end(), [](double a, double b) { return a > b; });
    
    double medianHue = HuesList.size() % 2 ? HuesList[HuesList.size() / 2] : (HuesList[HuesList.size() / 2] + HuesList[(HuesList.size() / 2) - 1]) / 2;

    this->medianHue = medianHue;

    //std::cout << "calc" << this->medianHSV.h << ", " << this->medianHSV.s << ", " << this->medianHSV.v << std::endl;
}

double image::getMedianHue() {
    //std::cout << this->medianHue << std::endl;
    double intpart;
    return this->medianHue == 0 ? NULL : modf((this->medianHue / 360 + 1 / 6), &intpart);
}
