#include "ImageAnalysis.h"

#include "Circle/mystuff.h"
#include "Circle/data.h"
#include "Circle/circle.h"
#include "Circle/Utilities.cpp"

#include "Circle/CircleFitByHyper.cpp"
#include "Circle/CircleFitByKasa.cpp"
#include "Circle/CircleFitByPratt.cpp"
#include "Circle/CircleFitByTaubin.cpp"

using namespace cv;

int ImageAnalysis::Draw()
{
    for (int i = 0; i < this->rects_led.size(); i++)
    {
        cv::circle(this->image, Point(rects_led[i][0], rects_led[i][1]), roi_size_led , Scalar(65000), 10);
    }

    for (int i = 0; i < this->rects_oarl.size(); i++)
    {
        cv::circle(this->image, Point(rects_oarl[i][0], rects_oarl[i][1]), roi_size_oarl, Scalar(65000), 10);
    }

    cv::circle(this->image, Point(circle_a, circle_b), circle_r, Scalar(65000), 10);
}

int ImageAnalysis::SaveImage(std::string ImagePath)
{
    cv::imwrite(ImagePath, this->image);
}

int ImageAnalysis::CalculateCircle()
{
    //cout << "CalculateCircle()" << endl;

    vector<vector<double>> led_positions = CalculateSpots();
    int n_leds = led_positions.size();
    if (n_leds < 3)
    {
        // TODO: Raise some errors here
        cout << "Cannot fit circle with less than 3 points." << endl;
    }
    else
    {
        // Try several different fitting programs. Time them and compare.
        reals DataX[n_leds];
        reals DataY[n_leds];

        for (int i = 0; i < n_leds; i++)
        {
            DataX[i] = led_positions[i][0];
            DataY[i] = led_positions[i][1];
        }

        Data data1(n_leds, DataX, DataY);
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
}

int ImageAnalysis::StoreResults()
{

}

std::string ImageAnalysis::PrintResults()
{
    //cout << circle_a << " " << circle_b << " " << circle_r << " " << circle_s << endl;
    std::ostringstream stream;
    stream << circle_a <<" " << circle_b << " " << circle_r << " " << circle_s;
    return stream.str();

}

vector<vector<double>> ImageAnalysis::CalculateSpots()
{
    // Calculates the center of spots. For LED or OARL.
    // Returns vector of centers of spots.

    //cout << "CalculateSpots()" << endl;
    vector<vector<double>> barycenter;

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
            cout << "ROI Moment normalization of LED " << i << " is 0." << endl;
            //barycenter.resize(0);
            //return barycenter;
        }

        else
        {
            // Barycenter position inside the ROI of the LED
            vector<double> barycenter_LED = {mu.m10 / mu.m00, mu.m01 / mu.m00};
            //Barycenter position in the whole image coordinates
            vector<double> barycenter_global = {rects_led[i][0] - roi_size_led / 2. + barycenter_LED[0], rects_led[i][1] - roi_size_led / 2. + barycenter_LED[1]};
            barycenter.push_back(barycenter_global); // x and y coordinates for each LED
        }
    }

    return barycenter;

    //cv::imwrite("/home/lstoperator/CDM/images/Rectangles.png", image);
}