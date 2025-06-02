#ifndef IMAGEANALYSIS_H_
#define IMAGEANALYSIS_H_

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/video/video.hpp>

#include <chrono>

#include "Config.h"
// TODO: Remove this later and use explicit namespaces
using namespace cv;
using namespace std;

class ImageAnalysis
{
public:
    //Need a constructor
    ImageAnalysis(Mat image, map<std::string,std::string> config, std::string image_flip = "None", int image_transpose = 0, int iBitsPerPixel=8);

    void Draw();
    void SaveImage(std::string ImagePath);
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
    int roi_size_led;// = 40;  // Integration region in pixels for LEDs. Check if big enough.
    int roi_size_oarl;// = 60; // Integration region in pixels for OARL. Check if big enough.

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

    vector<vector<int>> rects_led;// = LoadLedLoc();

    vector<vector<int>> rects_oarl;// = LoadOARLLoc();

    map<std::string,std::string> m_config;
};

#endif //  IMAGEANALYSIS_H_