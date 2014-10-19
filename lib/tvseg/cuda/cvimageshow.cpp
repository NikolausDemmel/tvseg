
#include "cvimageshow.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif

#include "opencv2/opencv.hpp"

#ifdef __clang__
#pragma clang diagnostic pop
#endif

namespace tvseg {
namespace cuda {


    using std::cout;
    using std::endl;


    bool autoForward = false;


    // Mouse Click Events: onMouseClick
    void CVonMouseClick(int event, int x, int y, int /*flags*/, void* userdata)
    {
        int r = y;
        int c = x;

        cv::Mat* img = (cv::Mat*)userdata;


        switch( event ) {
        case CV_EVENT_LBUTTONDOWN:  // Mouse left button down on

            if(img->type() == CV_8UC3){         // uchar 0 ... 255
                cout << "RGB Pixel (" << r << "," << c << ")\t [" << (int) img->at<cv::Vec3b>(r,c)[2] << ",\t" << (int) img->at<cv::Vec3b>(r,c)[1] << ",\t" << (int) img->at<cv::Vec3b>(r,c)[0] << "]" << endl;
            }
            else if(img->type() == CV_32FC3){   // float: 0 ... 1
                cout << "RGB Pixel (" << r << "," << c << ")\t [" << (float) img->at<cv::Vec3f>(r,c)[2] << ",\t" << (float) img->at<cv::Vec3f>(r,c)[1] << ",\t" << (float) img->at<cv::Vec3f>(r,c)[0] << "]" << endl;
            }
            else if(img->type() == CV_32FC1){
                cout << "Pixel (" << r << "," << c << ")\t [" << (float) img->at<float>(r,c) << "]" << endl;
            }
            else if(img->type() == CV_32SC1){   // int: 0 ... 1
                cout << "Pixel (" << r << "," << c << ")\t [" << (int) img->at<int>(r,c) << "]" << endl;
            }
            else if(img->type() == CV_8UC1){   // bool: 0 or 1
                cout << "Pixel (" << r << "," << c << ")\t [" << (bool) img->at<bool>(r,c) << "]" << endl;
            }
            else{
                cout << "onMouseClick.cpp: No type of the image found" << endl;
            }

            break;

    //    case CV_EVENT_LBUTTONUP:
    //        cout << "Mouse left button up on (" << x << ", " << y << ")" << endl;
    //        break;
        }
    }



    void cvImageShow(float* data, int rows, int cols, int nRegions, const char* winName)
    {
        //cv::Mat myimage = cv::Mat(rows, cols, CV_32F, data);

        int dim_size[3] = {nRegions, rows, cols};
        cv::Mat Mat_data(3, dim_size, CV_32FC1, data);


        cv::Mat show_Mat_data(rows, nRegions * cols, CV_32FC1);
        show_Mat_data.setTo(0);
        for(int r = 0; r < nRegions; r++){
            // to get the values which convertTo(I,CV_32FC3, 1/255.0f, 0); are included in Mat_data
            cv::Mat Mat_data_r(rows, cols, CV_32FC1, Mat_data.ptr<float>(r,0,0));
            double minVal, maxVal;
            cv::minMaxLoc(Mat_data_r, &minVal, &maxVal);
            //cout << "min and max Val: " << minVal << " " << maxVal << endl;

            // to get the values into show_Mat_data
            cv::Mat show_Mat_data_r(show_Mat_data, cv::Rect(r * cols, 0, cols, rows));

            // "shifts" the values from Mat_data to show_Mat_data and converts the values to be from 0 ... 1
            Mat_data_r.copyTo(show_Mat_data_r);
        }


        // normalize show_Mat_data to be between 0 ... 1:
        double minVal, maxVal;
        cv::minMaxLoc(show_Mat_data, &minVal, &maxVal);
        cout << " min Val of " << winName << " = " << minVal << "\tmax Val = " << maxVal << endl;
        show_Mat_data -= minVal;
        show_Mat_data /= maxVal - minVal;


        // show Image
        //std::string winName = "Mat_data";
        cv::namedWindow(winName, CV_WINDOW_NORMAL);
        cv::imshow(winName, show_Mat_data);

        // MouseCallback aktivieren
//        cv::setMouseCallback(winName,&CVonMouseClick,(void*)&show_Mat_data);

//        while(true)
//        {
//            int k = cv::waitKey(10);    // vorher: waitKey(50)
//            if(k == 97)
//                autoForward = !autoForward;

//            if(k==32 || autoForward)
//                break;
//        }

    }

}
}
