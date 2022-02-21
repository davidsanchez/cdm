#ifndef ImageAnalysis_H_
#define ImageAnalysis_H_

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/video/video.hpp>

#include <chrono>

// TODO: Remove this later and use explicit namespaces
using namespace cv;
using namespace std;

class ImageAnalysis
{
public:
    //Need a constructor
    ImageAnalysis(Mat image, std::string image_flip = "None", int image_transpose = 0, int iBitsPerPixel=8);

    int Draw();
    int SaveImage(std::string ImagePath);
    int StoreResults();
    std::string PrintResults();
    vector<uchar> GetImageToPublish(std::string inputText);

    vector<double> GetCircleResults();
    vector<double> GetDisplacementResults();
    vector<double> GetLEDxResults();
    vector<double> GetLEDyResults();
    vector<double> GetOARLxResults();
    vector<double> GetOARLyResults();
    vector<double> GetOARLmeanResults();

    void CalculateImage();
    void CalculateCircle();
    void CalculateSpotsLED();
    void CalculateSpotsOARL();
    void CalculateDisplacements();

    //private:

    std::vector<int> compression_params = {CV_IMWRITE_PNG_COMPRESSION, 0};
    Mat resized_image;
    vector<unsigned char> published_image;

    Mat image;
    double px2arcsec = 7.35;
    int roi_size_led = 40;  // Integration region in pixels for LEDs. Check if big enough.
    int roi_size_oarl = 60; // Integration region in pixels for OARL. Check if big enough.

    double circle_a = 0;
    double circle_b = 0;
    double circle_r = 0;
    double circle_s = 0;
    vector<double> led_x;
    vector<double> led_y;
    vector<double> oarl_x;
    vector<double> oarl_y;

    double oarl_x_mean = 0;
    double oarl_y_mean = 0;
    double displacement_x = 0;
    double displacement_y = 0;
    int n_leds = 0;
    int n_leds_valid = 0;

    vector<double> led_result;

    vector<vector<int>> rects_led{
        // double commented were not used in 6LED approach
        {809, 1679},  // second most left, upper
        {465, 2288},  // //most left, upper
        {460, 2967},  // //most left, lower
        {814, 3567},  // //second most left, lower
        {1334, 3893}, // //most bottom, left one
        {2153, 3893}, // most bottom, right one
        {2683, 3567}, // second most right, lower
        {3028, 2972}, // most right, lower
        {3032, 2293}, // defective one. most right, upper
        {2688, 1688}, // second most right, upper
        {1340, 1366}, // //uncovered new most top left
        {2162, 1368}  // //uncovered new most top right
    };

    vector<vector<int>> rects_oarl{
        {1540, 4372},
        {1986, 4367}};
};

#endif //  ImageAnalysis_H_