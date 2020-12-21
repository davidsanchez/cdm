#ifndef ImageAnalysis_H_
#define ImageAnalysis_H_

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/video/video.hpp>

// TODO: Remove this later and use explicit namespaces
using namespace cv;
using namespace std;

class ImageAnalysis
{
public:
    //Need a constructor
    ImageAnalysis(Mat image)
    {
        this->image = image.clone(); // TODO: decide on pointer or copying.
        flip(this->image, this->image, 0); // Vertical flipping of image so it is upright
        this->image.convertTo(this->image, CV_8UC1, 1/256.0); //TODO: try with 16bit images

        // To remove the noise
        //TODO: these values for 8bit. Decide for 16bit values.
        cv::threshold(this->image, this->image, 10, 255, THRESH_TOZERO);

    };

    int Draw();
    int CalculateCircle();
    vector <vector<double>> CalculateSpots();

//private:

    Mat image;
    double px2arcsec = 7.35;
    int roi_size_led = 40; // Integration region in pixels for LEDs. Check if big enough.
    int roi_size_oarl = 60; // Integration region in pixels for OARL. Check if big enough.

    vector <vector<int>> rects_led 
    {
        // double commented were not used in 6LED approach
        { 809, 1679},  // second most left, upper
        { 465, 2288}, // //most left, upper
        { 460, 2967}, // //most left, lower
        { 814, 3567}, // //second most left, lower
        {1334, 3893}, // //most bottom, left one 
        {2153, 3893}, // most botton, right one
        {2683, 3567}, // second most right, lower
        {3028, 2972}, // most right, lower
        {3032, 2293}, // defective one. most right, upper
        {2688, 1688}, // second most right, upper
        {1340, 1366}, // //uncovered new most top left
        {2162, 1368} // //uncovered new most top right
    };

    vector <vector<int>> rects_oarl
    {
        {1540, 4372},
        {1986, 4367}
    };
};

#endif //  ImageAnalysis_H_