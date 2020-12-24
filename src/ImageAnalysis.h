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
    ImageAnalysis(Mat image, std::string image_flip = "None", int image_transpose = 0, int iBitsPerPixel=8)
    {
        int flip_code;
        if (image_flip == "Vertical")
            flip_code = 0;
        else if (image_flip == "Horizontal")
            flip_code = 1;
        else if (image_flip == "Both")
            flip_code = -1;
        else if (image_flip == "None")
            flip_code = -999;
        else
            flip_code = -999;

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        this->image = image.clone(); // TODO: decide on pointer or copying.
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        std::cout << "Time difference [Clone] = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;

        if (image_transpose != 0)
        {
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            transpose(this->image, this->image);
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            std::cout << "Time difference [Transpose] = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
        }
        if (flip_code != -999)
        {
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            flip(this->image, this->image, flip_code);
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            std::cout << "Time difference [Flip] = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
        }
        
        // Converting image if needed
        begin = std::chrono::steady_clock::now();
        if (iBitsPerPixel == 16)
            this->image.convertTo(this->image, CV_8UC1, 1 / 256.0); //TODO: try with 16bit images
        end = std::chrono::steady_clock::now();
        std::cout << "Time difference [Convert] = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;

        // To remove the noise
        //TODO: these values for 8bit. Decide for 16bit values.
        begin = std::chrono::steady_clock::now();
        cv::threshold(this->image, this->image, 10, 255, THRESH_TOZERO);
        end = std::chrono::steady_clock::now();
        std::cout << "Time difference [Threshold] = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
    };

    int Draw();
    int CalculateCircle();
    int SaveImage(std::string ImagePath);
    int StoreResults();
    vector<double> GetResults();
    std::string PrintResults();
    vector<vector<double>> CalculateSpots();

    //private:

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