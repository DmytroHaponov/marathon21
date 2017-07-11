#include "stdafx.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <cassert>
#include <stdint.h>
//#define CATCH_CONFIG_MAIN 
//#include "catch.hpp"

// Grayscale image, each pixel is 8-bit unsigned value:
// 0 means black, 255 means white, values between are shades of gray.
// Binary image: pixel values equal to one of: 0, 255.
// 4-connectivity is assumed i.e. for pixel its neighbors are up, down,
// left and right from it.
// Pixels outside the image i.e. with x < 0, y < 0, x >= width, y >= height
// are assumed to have value 0 (zero)
// Image pixels have x coordinate in range [0, width-1], x grows from left to right
// y coordinate in range [0, height-1], y grows from top to bottom
// Thus (0, 0) is upper left corner of the image.
class GrayImage
{
public:
    typedef uint8_t pixel_t;

    // creates empty image
    GrayImage();

    // creates image of given size filled with black pixels
    GrayImage(int height, int width);

    // creates binary image from flat string of 'x's and 'o's
    GrayImage(int height, int width, const std::string& data);

    // prints image with zeros shown as 'o's, 255 as 'x's, all other as '?'
    // useful for debugging binary image algorithms
    void print() const;

    pixel_t& operator()(int y, int x);
    const pixel_t& operator()(int y, int x) const;

    // content is lost, resulting image has all pixels black (0)
    void resize(int height, int width);

    void fill(pixel_t value);

    int getHeight() const;
    int getWidth() const;

    // only binary PGM with max value 255 is supported
    int loadFromPGM(const std::string& pathToPGMFile);

    // save as binary PGM with max value of 255
    int saveToPGM(const std::string& pathToPGMFile);

    friend bool operator==(const GrayImage& one, const GrayImage& two);
    friend bool operator!=(const GrayImage& one, const GrayImage& two);

    // Rotate image clockwise 90 degrees around image's center.
    // If original image has dimensions height * width, resulting image
    // will have dimensions width * height.
    // Rotating 4 times should result in image equal to original.
    // Note: implementation shall use O(1) additional memory i.e.
    // one can't just create temporary image, write data to it and then
    // copy them to image.
    void rotateCw90();

    // Rotate image counter clockwise 90 degrees around image's center.
    // If original image has dimensions height * width, resulting image
    // will have dimensions width * height.
    // Rotating 4 times should result in image equal to original.
    // Note: implementation shall use O(1) additional memory i.e.
    // one can't just create temporary image, write data to it and then
    // copy them to image.
    void rotateCcw90();

    // Move each point (y,x) on source image to (y+dy, x+dx) on result image.
    // Result image has the same size, as source one.
    // dy and dx may be positive or negative.
    // Points translated from outside of the image has black color (zero value).
    // Note: implementation shall use O(1) additional memory i.e.
    // one can't just create temporary image, write data to it and then
    // copy them to image.
    void translateInplace(int dy, int dx);

private:
    int height_;
    int width_;
    std::vector<pixel_t> data_;
};

GrayImage::GrayImage()
    : height_(0)
    , width_(0)
{
}

GrayImage::GrayImage(int height, int width)
    : height_(height)
    , width_(width)
    , data_(height * width)
{
    assert(height > 0  &&  width > 0);
}

GrayImage::GrayImage(int height, int width, const std::string& data)
    : height_(height)
    , width_(width)
    , data_(height * width)
{
    assert(height > 0  &&  width > 0  &&  data.length() == height * width);

    for (size_t i = 0; i < data.length(); ++i)
        data_[i] = data[i] == 'o' ? 0 : 255;
}

void GrayImage::print() const
{
    for (int y = 0; y < height_; ++y)
    {
        for (int x = 0; x < width_; ++x)
        {
            uint8_t p = (*this)(y, x);
            char ch = p == 255 ? 'x' : (p == 0 ? 'o' : '?');
            std::cout << ch;
        }
        std::cout << std::endl;
    }
}

GrayImage::pixel_t& GrayImage::operator()(int y, int x)
{
    assert(y >= 0  &&  y < height_);
    assert(x >= 0  &&  x < width_);
    int offset = y * width_ + x;
    return data_[offset];
}

const GrayImage::pixel_t& GrayImage::operator()(int y, int x) const
{
    assert(y >= 0  &&  y < height_);
    assert(x >= 0  &&  x < width_);
    int offset = y * width_ + x;
    return data_[offset];
}

void GrayImage::resize(int height, int width)
{
    assert (height > 0  &&  width > 0);
    height_ = height;
    width_ = width;
    std::vector<pixel_t> data(height * width);
    data_.swap(data);
}

void GrayImage::fill(GrayImage::pixel_t value)
{
    std::fill(data_.begin(), data_.end(), value);
}

int GrayImage::getHeight() const
{
    return height_;
}

int GrayImage::getWidth() const
{
    return width_;
}

int GrayImage::loadFromPGM(const std::string& pathToPGMFile)
{
	std::ifstream ifs(pathToPGMFile.c_str());

	if (!ifs.is_open())
	{
		std::cerr << "Failed to open file for reading: " << pathToPGMFile << std::endl;
		return -1;
	}

	std::string magic;
	ifs >> magic;

	if (magic != "P5")
	{
		std::cerr << "Unrecognized magic: " + magic << std::endl;
		return -1;
	}

	int width;
	ifs >> width;

	if (width <= 0)
	{
		std::cerr << "Invalid width: " << width << std::endl;
		return -1;
	}

	int height;
	ifs >> height;

	if (height <= 0)
	{
		std::cerr << "Invalid height: " << height << std::endl;
		return -1;
	}

	int maxValue;
	ifs >> maxValue;

	if (maxValue != 255)
	{
		std::cerr << "Only max value of 255 is supported, got " << maxValue << std::endl;
		return -1;
	}

	ifs.ignore();

	resize(height, width);
	ifs.read((char*)&data_[0], width * height);

	if (!ifs)
	{
		std::cerr << "Error reading pixel data" << std::endl;
		return -1;
	}

	return 0;
}

int GrayImage::saveToPGM(const std::string& pathToPGMFile)
{
	std::ofstream ofs(pathToPGMFile.c_str());

	if (!ofs.is_open())
	{
		std::cerr << "Failed to open file for writing: " << pathToPGMFile << std::endl;
		return -1;
	}

	ofs << "P5" << '\n' << width_ << ' ' << height_ << '\n' << 255 << '\n';
	ofs.write((const char*)&data_[0], width_ * height_);

	if (!ofs)
	{
		std::cerr << "Error writing pixel data" << std::endl;
		return -1;
	}

	return 0;
}

inline bool operator==(const GrayImage& one, const GrayImage& two)
{
    return one.height_ == two.height_
            &&  one.width_ == two.width_
            &&  one.data_ == two.data_;
}

inline bool operator!=(const GrayImage& one, const GrayImage& two)
{
    return !(one == two);
}

std::string get_data_str(const GrayImage& image)
{
	std::string cur_str;
	int height = image.getHeight();
	int width = image.getWidth();
	for(int cur_height = 0; cur_height < height; ++cur_height)
	{
		for (int cur_width = 0; cur_width < width; ++cur_width)
		{
			image(cur_height,cur_width) == 0 ? cur_str.append("o") : cur_str.append("x");
		}
	}
	return cur_str;
}

// You have to implement only function(s) you are asked to implement

// Move each point (y,x) on source image to (y+dy, x+dx) on result image.
// Result image has the same size, as source one.
// dy and dx may be positive or negative.
// Points translated from outside of the image has black color (zero value).
GrayImage translate(const GrayImage& image, int dy, int dx)
{
	if(dy == 0 && dx == 0)
		return image;

	int width = image.getWidth();
	int height = image.getHeight();
	if(std::abs(dx) >= width || std::abs(dy) >= height)
		return GrayImage(height, width, std::string(width*height, 'x'));

	std::string cur_str = get_data_str(image);
	int str_size = static_cast<int>(cur_str.length());
	std::string to_create_from(str_size, 'x');
	for (int str_pos = 0; str_pos < str_size; ++str_pos)
	{
		int cur_pos = str_pos + dy*width + dx;
		if (cur_pos >= 0 && cur_pos < str_size)
			to_create_from.at(cur_pos) = cur_str.at(str_pos);
	}
	return GrayImage(height, width, to_create_from);
}

bool isBinary(const GrayImage& image)
{
	//! pity cannot do smth like
	// return std::all_of(image.data_.begin(), image.data_.end(), [](pixel_t pix){return (pix != 0 && pix != 255);});

	//! is empty image binary? - I think not
	int height = image.getHeight();
	int width = image.getWidth();

	if(!height || !width)
		return false;

	for(int cur_height = 0; cur_height < height; ++cur_height)
	{
		for (int cur_width = 0; cur_width < width; ++cur_width)
		{
			if(image(cur_height,cur_width) != 0 && image(cur_height,cur_width) != 255)
				return false;
		}
	}
	return true;
}

// Set pixels that are less than thr to zero, others to 255
GrayImage threshold(const GrayImage& image, uint8_t thr)
{
	std::string cur_str;
	int height = image.getHeight();
	int width = image.getWidth();
	for(int cur_height = 0; cur_height < height; ++cur_height)
	{
		for (int cur_width = 0; cur_width < width; ++cur_width)
		{
			image(cur_height,cur_width) < thr ? cur_str.append("o") : cur_str.append("x");
		}
	}
	return GrayImage(height, width, cur_str);
}

// Should be applied only to binary image
// Background is black (0), foreground is white (255).
// Foreground, consisting of one or more connected components, may have one or more holes:
// connected components of black, that doesn't touch image border (4-connectivity
// is assumed).
// The resulting image should have all holes filled with white.
GrayImage binaryFillHoles(const GrayImage& image);

// Let's call this image src and resulting image dst. src(y, x) - pixels of source
// image with coordinates (y, x).
// dst(y,x) = 255, iff src(y, x) = 0 and path exists in source image (4-connectivity
// is assumed) from (y, x) to image border such that all pixels on this path are 0.
GrayImage binaryBackground(const GrayImage& image);



int main(int argc, char *argv[])
{
    // TODO: insert your code below
	std::string for_im1("xoxxoxxxx");
	GrayImage im1(3,3,for_im1);

	GrayImage im_trans = translate(im1, 0, -1);

	im_trans.print();

    return 0;
}
//
//TEST_CASE( "is binary" ) 
//{
//	std::string for_im1("xoxxoxxxx");
//	GrayImage im1(3,3,for_im1);
//	REQUIRE( isBinary(im1) == true );
//}
//
//TEST_CASE( "is binary for empty" ) 
//{
//	GrayImage im1;
//	REQUIRE( isBinary(im1) == false );
//}
//
//TEST_CASE( "threshold" ) 
//{
//	std::string for_im1("xoxxoxxxx");
//	GrayImage im1(3,3,for_im1);
//	REQUIRE( threshold(im1, 10) == im1 );
//}
//
//
//TEST_CASE( "move 3*3 for dx = 1" ) 
//{
//	std::string for_im1("xoxxoxxxx");
//	GrayImage im1(3,3,for_im1);
//	GrayImage im_trans = translate(im1, 0, 1);
//	REQUIRE( im_trans == GrayImage(3,3,"xxoxxoxxx") );
//}
//
//TEST_CASE( "move 3*3 for dy = 1" ) 
//{
//	std::string for_im1("xoxxoxxxx");
//	GrayImage im1(3,3,for_im1);
//	GrayImage im_trans = translate(im1, 1, 0);
//	REQUIRE( im_trans == GrayImage(3,3,"xxxxoxxox") );
//}
//
//TEST_CASE( "move 3*3 for dx =1 dy = 1" ) 
//{
//	std::string for_im1("xoxxoxxxx");
//	GrayImage im1(3,3,for_im1);
//	GrayImage im_trans = translate(im1, 1, 1);
//	REQUIRE( im_trans == GrayImage(3,3,"xxxxxoxxo") );
//}
//
//TEST_CASE( "move 3*3 for dx = -1" ) 
//{
//	std::string for_im1("xoxxoxxxx");
//	GrayImage im1(3,3,for_im1);
//	GrayImage im_trans = translate(im1, 0, -1);
//	REQUIRE( im_trans == GrayImage(3,3,"oxxoxxxxx") );
//}
//
//TEST_CASE( "move 3*3 for dx = -1 dy = -1" ) 
//{
//	std::string for_im1("xoxxoxxxx");
//	GrayImage im1(3,3,for_im1);
//	GrayImage im_trans = translate(im1, -1, -1);
//	REQUIRE( im_trans == GrayImage(3,3,"oxxxxxxxx") );
//}
//
//TEST_CASE( "move 3*3 for dx = 10" ) 
//{
//	std::string for_im1("xoxxoxxxx");
//	GrayImage im1(3,3,for_im1);
//	GrayImage im_trans = translate(im1, 0, 10);
//	REQUIRE( im_trans == GrayImage(3,3,"xxxxxxxxx") );
//}
//
//TEST_CASE( "move 3*3 for dy = 10" ) 
//{
//	std::string for_im1("xoxxoxxxx");
//	GrayImage im1(3,3,for_im1);
//	GrayImage im_trans = translate(im1, 10, 0);
//	REQUIRE( im_trans == GrayImage(3,3,"xxxxxxxxx") );
//}
