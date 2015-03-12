//#include "hdf5.h"
//#define FILE "file.h5"
//#include "H5Cpp.h"
#ifndef H5_NO_NAMESPACE
using namespace H5;
#endif

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/opencv.hpp"


#include <iostream>


using namespace cv;
using namespace std;

//const int MASK_THRESHOLD = 180;
//const int window_size = 500;
//const char file_name[] = "/Users/ajaver/Desktop/Test_Gecko_Compression/frame2.jpg";

#define BUFF_SIZE 25
const char file_name[] = "/Users/ajaver/Desktop/Test_Gecko_Compression/CaptureTest_90pc_Ch1_19022015_210644.mjpg";

Mat getMaskImage(Mat);

int main()
{
VideoCapture vid(file_name);
if(!vid.isOpened())
return -1;

Mat image_buff[BUFF_SIZE];
Mat image, mask, image_small;
Mat min_image;
bool bSuccess;
int frame_number = 0;
int buff_number = 0;

double t = (double)getTickCount();

while( frame_number < 100)
{
bSuccess = vid.read(image);
if (!bSuccess)
break;

cvtColor(image, image, CV_RGB2GRAY);

buff_number = frame_number%BUFF_SIZE;
image_buff[buff_number] = image;

if (buff_number == 0)
min_image = image.clone();
else
min_image = min(min_image, image);


if (buff_number == BUFF_SIZE-1)
{
mask = getMaskImage(min_image);
for (int kk =0; kk<BUFF_SIZE; kk++){
bitwise_and(mask, image_buff[kk], image_buff[kk]);
}
}

frame_number++;
}

t = ((double)getTickCount() - t)/getTickFrequency();
cout << "Times passed in seconds: " << t << endl;

//
//     image = imread(file_name, CV_LOAD_IMAGE_GRAYSCALE);   // Read the file
//
//     if(! image.data )                              // Check for invalid input
//     {
//     cout <<  "Could not open or find the image" << std::endl ;
//     return -1;
//     }
//
//     namedWindow( "Display window", CV_WINDOW_NORMAL | CV_WINDOW_FREERATIO );// Create a window for display.
//
//    namedWindow( "Components", 1 );
//     resize(image, image_small, cvSize(window_size, window_size));
//     imshow( "Components", image_small );
//     resizeWindow("Display window", window_size,window_size);
//
//    Mat image_masked = image.clone();
//
//    double t = (double)getTickCount();
//    for (int k = 0; k < 100; k++)
//         //getMaskedImage(image, image_masked);
//
//    t = ((double)getTickCount() - t)/getTickFrequency();
//    cout << "Times passed in seconds: " << t << endl;
//
//
//     namedWindow( "Components_2", 1 );
//     resize(image_masked, image_small, cvSize(window_size, window_size));
//     imshow( "Components_2", image_small);
//     resizeWindow("Display window", window_size,window_size);
//     waitKey(0);


//bitwise_and(mask, image, mask);
cout << "Finished" << endl;

return 0;
}

Mat getMaskImage(Mat image){
const int MAX_AREA = 5000;
const int MIN_AREA = 100;

Size image_size = image.size();
const int IM_LIMX = image_size.width-1;
const int IM_LIMY = image_size.height-1;


double contour_area;
vector<vector<Point> > contours;
vector<Vec4i> hierarchy;

Mat mask(image.size(), image.type());

//get binary image using adapttive thresholding
adaptiveThreshold(image, mask, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 61, 15);

//find image contours (connected components)
Mat temp_image;
mask.copyTo(temp_image);
findContours( temp_image, contours, hierarchy,
CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);


if (!contours.empty()){
for (int i=0; i<contours.size(); ++i) {
// remove the contours that is toching the image border
for (int j = 0; j<contours[i].size(); j++){
if(contours[i][j].x==1 || contours[i][j].x==IM_LIMX || contours[i][j].y==1 || contours[i][j].y==IM_LIMY) {
drawContours(mask, contours, i, 0, -1);
break;
}
}
//remove objects that are too small or too big
contour_area = contourArea(contours[i]);
if ( contour_area < MIN_AREA || contour_area > MAX_AREA)
drawContours(mask, contours, i, 0, -1);

}
}

//make sure the border pixels are zero (after filling the border contours, it is comun to have residual pixels in the mask
mask.row(0) = mask.row(0)*0;
mask.col(0) = mask.col(0)*0;
mask.row(IM_LIMX) = mask.row(IM_LIMX)*0;
mask.col(IM_LIMY) = mask.col(IM_LIMY)*0;

Mat element = cv::getStructuringElement(MORPH_ELLIPSE, Size(9, 9), Point(2,2));
cv::dilate(mask, mask, element, Point(-1,-1), 3);

//select a ROI for the timestamp
rectangle(mask, Point(0,0), Point(479,15), 255 , CV_FILLED);


//bitwise_and(mask, image, masked_image);
return mask;
}
