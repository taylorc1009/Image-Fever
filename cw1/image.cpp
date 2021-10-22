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

void image::calculateMedianHSV() {
    std::vector<uint8_t> imgData = this->getImageData();
    hsv totalHSV = { 0 }, medianHSV = { 0 };

    for (unsigned int i = 0; i < imgData.size(); i += 3) {
        rgb pixelRGB;

        pixelRGB.r = imgData[i];
        pixelRGB.g = imgData[i + 1];
        pixelRGB.b = imgData[i + 2];

        hsv pixelHSV = rgb2hsv(pixelRGB);

        totalHSV.h += pixelHSV.h;
        totalHSV.s += pixelHSV.s;
        totalHSV.v += pixelHSV.v;
    }

    this->medianHSV.h = totalHSV.h / (imgData.size() / 3);
    this->medianHSV.s = totalHSV.h / (imgData.size() / 3);
    this->medianHSV.v = totalHSV.h / (imgData.size() / 3);
}