#include "ImageAnalysis.h"

#include "Circle/mystuff.h"

#include "Circle/data.h"

#include "Circle/circle.h"

#include "Circle/Utilities.cpp"

#include "Circle/CircleFitByHyper.cpp"
#include "Circle/CircleFitByKasa.cpp"
#include "Circle/CircleFitByPratt.cpp"
#include "Circle/CircleFitByTaubin.cpp"

#include "Logging.h"

using namespace cv;

int ImageAnalysis::Draw()
{
    LOG_TRACE << "ImageAnalysis::Draw()";

    /*     for (int i = 0; i < this->rects_led.size(); i++)
    {
        cv::circle(this->image, Point(rects_led[i][0], rects_led[i][1]), roi_size_led, Scalar(65000), 10);
    }

    for (int i = 0; i < this->rects_oarl.size(); i++)
    {
        cv::circle(this->image, Point(rects_oarl[i][0], rects_oarl[i][1]), roi_size_oarl, Scalar(65000), 10);
    } */

    cv::circle(this->image, Point(circle_a, circle_b), circle_r, Scalar(100), 2);
}

int ImageAnalysis::SaveImage(std::string ImagePath)
{
    LOG_TRACE << "ImageAnalysis::SaveImage()";
    cv::imwrite(ImagePath, this->image);
}

void ImageAnalysis::CalculateImage()
{
    LOG_TRACE << "ImageAnalysis::CalculateImage()";
    CalculateCircle();
    CalculateSpotsOARL();
    CalculateDisplacements();
}

void ImageAnalysis::CalculateCircle()
{
    LOG_TRACE << "ImageAnalysis::CalculateCircle()";

    CalculateSpotsLED();
    vector<double> led_positions_x = this->led_x;
    vector<double> led_positions_y = this->led_y;

    vector<double> led_positions_x_valid = led_positions_x;
    vector<double> led_positions_y_valid = led_positions_y;

    // Removes the LED that were not detected
    led_positions_x_valid.erase(std::remove_if(led_positions_x_valid.begin(), led_positions_x_valid.end(), [](const double &x) { return x < 0; }), led_positions_x_valid.end());
    led_positions_y_valid.erase(std::remove_if(led_positions_y_valid.begin(), led_positions_y_valid.end(), [](const double &y) { return y < 0; }), led_positions_y_valid.end());

    int n_leds = led_positions_x.size();
    int n_leds_valid = led_positions_x_valid.size();
    if (n_leds_valid < 3)
    {
        // TODO: Raise some errors here
        LOG_ERROR << "Cannot fit circle with less than 3 points." << endl;
        this->circle_a = 0;
        this->circle_b = 0;
        this->circle_r = 0;
        this->circle_s = 0;
    }
    else
    {
        // Try several different fitting programs. Time them and compare.

        reals DataX[n_leds_valid];
        reals DataY[n_leds_valid];
        std::copy(led_positions_x_valid.begin(), led_positions_x_valid.end(), DataX);
        std::copy(led_positions_y_valid.begin(), led_positions_y_valid.end(), DataY);
        Data data1(n_leds_valid, DataX, DataY);

        Circle circle;
        cout.precision(10);

        circle = CircleFitByHyper(data1);
        //cout << "\n  Hyper  fit:  center ("
        //     << circle.a << "," << circle.b << ")  radius "
        //     << circle.r << "  sigma " << circle.s << endl;

        this->circle_a = circle.a;
        this->circle_b = circle.b;
        this->circle_r = circle.r;
        this->circle_s = circle.s;
    }

    this->n_leds = n_leds;
    this->n_leds_valid = n_leds_valid;
}

void ImageAnalysis::CalculateDisplacements()
{
    LOG_TRACE << "ImageAnalysis::CalculateDisplacement()";

    double oarl_x_mean;
    double oarl_y_mean;
    double displacement_x;
    double displacement_y;

    // Checks if the OARLs are properly detected.
    if (this->oarl_x[0] > 0 && this->oarl_x[1] > 0)
    {
        oarl_x_mean = (this->oarl_x[0] + this->oarl_x[1])/2.0;
        oarl_y_mean = (this->oarl_y[0] + this->oarl_y[1])/2.0;
        displacement_x = oarl_x_mean - this->circle_a;
        displacement_y = oarl_y_mean - this->circle_b;
    }
    else
    {
        oarl_x_mean = -1;
        oarl_y_mean = -1;
        displacement_x = -1;
        displacement_y = -1;
    }

    this->oarl_x_mean = oarl_x_mean;
    this->oarl_y_mean = oarl_y_mean;
    this->displacement_x = displacement_x;
    this->displacement_y = displacement_y;
}

void ImageAnalysis::CalculateSpotsLED()
{
    LOG_TRACE << "ImageAnalysis::CalculateSpotsLED()";

    // Calculates the center of spots for LED.
    // Returns vector of centers of spots.

    //cout << "CalculateSpots()" << endl;
    vector<vector<double>> barycenter;
    vector<double> barycenter_x;
    vector<double> barycenter_y;

    for (int i = 0; i < this->rects_led.size(); i++)
    {
        //Create the rectangle ROI
        cv::Rect roi(rects_led[i][0] - roi_size_led / 2., rects_led[i][1] - roi_size_led / 2., roi_size_led, roi_size_led);
        //cv::rectangle(this->image, roi, Scalar( 65000 ), 10 );
        Mat image_roi = image(roi);
        // Calculates image moments, used for centre of gravity calculations
        Moments mu = moments(image_roi);

        if (mu.m00 == 0)
        {
            //TODO: Raise some warning! Or just report number of found LEDs always
            // LED is skipped in this case.
            LOG_ERROR << "ROI Moment normalization of LED " << i << " is 0." << endl;
            barycenter_x.push_back(-1); // sets to -1 if the LED not detected
            barycenter_y.push_back(-1); // sets to -1 if the LED not detected

            //barycenter.resize(0);
            //return barycenter;
        }

        else
        {
            // Barycenter position inside the ROI of the LED
            //vector<double> barycenter_LED = {mu.m10 / mu.m00, mu.m01 / mu.m00};
            double barycenter_LED_x = (mu.m10 / mu.m00);
            double barycenter_LED_y = (mu.m01 / mu.m00);

            //Barycenter position in the whole image coordinates
            //vector<double> barycenter_global = {rects_led[i][0] - roi_size_led / 2. + barycenter_LED[0], rects_led[i][1] - roi_size_led / 2. + barycenter_LED[1]};
            double barycenter_global_x = rects_led[i][0] - roi_size_led / 2. + barycenter_LED_x;
            double barycenter_global_y = rects_led[i][1] - roi_size_led / 2. + barycenter_LED_y;

            // Making the vector of results
            //barycenter.push_back(barycenter_global); // x and y coordinates for each LED
            barycenter_x.push_back(barycenter_global_x); // x coordinates for each LED
            barycenter_y.push_back(barycenter_global_y); // x coordinates for each LED
        }
    }

    this->led_x = barycenter_x;
    this->led_y = barycenter_y;

    //cv::imwrite("/home/lstoperator/CDM/images/Rectangles.png", image);
}

void ImageAnalysis::CalculateSpotsOARL()
{
    LOG_TRACE << "ImageAnalysis::CalculateSpotsOARL()";

    // Calculates the center of spots for OARL.
    // Returns vector of centers of spots.

    //cout << "CalculateSpots()" << endl;
    vector<vector<double>> barycenter;
    vector<double> barycenter_x;
    vector<double> barycenter_y;

    for (int i = 0; i < this->rects_oarl.size(); i++)
    {
        //Create the rectangle ROI
        cv::Rect roi(rects_oarl[i][0] - roi_size_oarl / 2., rects_oarl[i][1] - roi_size_oarl / 2., roi_size_oarl, roi_size_oarl);
        //cv::rectangle(this->image, roi, Scalar( 65000 ), 10 );
        Mat image_roi = image(roi);
        // Calculates image moments, used for centre of gravity calculations
        Moments mu = moments(image_roi);

        if (mu.m00 == 0)
        {
            //TODO: Raise some warning! Or just report number of found OARLs always
            // OARL is skipped in this case.
            LOG_ERROR << "ROI Moment normalization of OARL " << i << " is 0." << endl;
            barycenter_x.push_back(-1); // sets to -1 if the OARL not detected
            barycenter_y.push_back(-1); // sets to -1 if the OARL not detected

            //barycenter.resize(0);
            //return barycenter;
        }

        else
        {
            // Barycenter position inside the ROI of the OARL
            double barycenter_OARL_x = (mu.m10 / mu.m00);
            double barycenter_OARL_y = (mu.m01 / mu.m00);

            //Barycenter position in the whole image coordinates
            double barycenter_global_x = rects_oarl[i][0] - roi_size_oarl / 2. + barycenter_OARL_x;
            double barycenter_global_y = rects_oarl[i][1] - roi_size_oarl / 2. + barycenter_OARL_y;

            // Making the vector of results
            barycenter_x.push_back(barycenter_global_x); // x coordinates for each OARL
            barycenter_y.push_back(barycenter_global_y); // x coordinates for each OARL
        }
    }

    this->oarl_x = barycenter_x;
    this->oarl_y = barycenter_y;
}

vector<uchar> ImageAnalysis::GetImageToPublish(string inputText)
{
    LOG_TRACE << "ImageAnalysis::GetImageToPublish()";

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    // INTER_NEAREST is the fastest algorithm but has the worst quality results.
    // But we are not interested in the quality of the this image as it only used for checking if everything is OK.
    ImageAnalysis::Draw();
    resize(this->image, resized_image, cv::Size(0, 0), 0.15, 0.15, INTER_NEAREST);
    putText(resized_image,
            inputText,                                                        // Text
            Point((int)resized_image.cols / 12, (int)resized_image.rows / 8), // Coordinates
            FONT_HERSHEY_SIMPLEX,                                             // Font type
            1,                                                                // Font scale
            255,                                                              // Color
            1,                                                                // Thickness
            8                                                                 // Line type
    );
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    LOG_INFO << "Time difference [Get image for publishing - resize] = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
    begin = std::chrono::steady_clock::now();
    cv::imencode(".png", resized_image, published_image, compression_params); // Compresses and converts image to memory buffer (bytestring) so that it can be published to OPCUA datapoint
    end = std::chrono::steady_clock::now();
    LOG_INFO << "Time difference [Get image for publishing - imencode] = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;

    return published_image;
}

int ImageAnalysis::StoreResults()
{
}

std::string ImageAnalysis::PrintResults()
{
    LOG_TRACE << "ImageAnalysis::PrintResults()";

    //cout << circle_a << " " << circle_b << " " << circle_r << " " << circle_s << endl;
    std::ostringstream stream;
    stream << circle_a << " " << circle_b << " " << circle_r << " " << circle_s;
    return stream.str();
}

vector<double> ImageAnalysis::GetCircleResults()
{
    vector<double> results = {circle_a, circle_b, circle_r, circle_s};
    return results;
}

vector<double> ImageAnalysis::GetDisplacementResults()
{
    vector<double> results = {displacement_x, displacement_y, 0.0};
    return results;
}

vector<double> ImageAnalysis::GetLEDxResults()
{
    return led_x;
}

vector<double> ImageAnalysis::GetLEDyResults()
{
    return led_y;
}

vector<double> ImageAnalysis::GetOARLxResults()
{
    return oarl_x;
}

vector<double> ImageAnalysis::GetOARLyResults()
{
    return oarl_y;
}

vector<double> ImageAnalysis::GetOARLmeanResults()
{
    vector<double> results = {oarl_x_mean, oarl_y_mean};
    return results;
}