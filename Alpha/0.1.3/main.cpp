#include <iostream>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <header.h>

using namespace std;
using namespace cv;

#define MINHEIGHT 2
#define MAXHEIGHT 3

int main()
{
    VideoCapture capDepth;
    VideoCapture capRGB;
    capDepth.open("Depth1.avi");
    capRGB.open("RGB1.avi");
    int move1=65,move2=42;

    Mat firstImage,secondImage,result1,result;

    //////////////////////////////////////////
    int cut_min=260,cut_max=420;
    namedWindow("Height");
    createTrackbar("Height_min","Height",&cut_min,640,NULL);
    createTrackbar("Height_max","Height",&cut_max,480,NULL);

    namedWindow("Height1");
    createTrackbar("Height_min","Height1",&move1,640,NULL);
    createTrackbar("Height_max","Height1",&move2,480,NULL);
    namedWindow("Height2");
    createTrackbar("Height_min","Height2",&s3,20,NULL);
    createTrackbar("Height_max","Height2",&s4,20,NULL);
    ///////////////////////////////////////////

    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    while (1)
    {

        int t1 = getTickCount();  //getTickCount函数从0开始计时，返回自设备启后的毫秒数（不含系统暂停时间）

        Mat DEPTH,gray;
        Mat Fusion,RGB;
        capDepth >> DEPTH;
        capRGB >> RGB;
        Mat RGBtransform;
        translateTransform(RGB,RGBtransform,move1-50,move2-50);

        Fusion = 0.4*RGBtransform +0.1*DEPTH ;
 //     Fusion = RGBtransform ;
        if (DEPTH.empty())break;
        Mat ROI(DEPTH,Rect(0,cut_min,640,cut_max - cut_min));
        cvtColor(ROI,gray,CV_BGR2GRAY);

        result = gray.clone();
        result1 = gray.clone();
        firstImage = prehandle(result,14,20);
        secondImage = prehandle(result1,19,25);

        imshow("result",firstImage);
        imshow("result1",secondImage);



        imshow("ROI",ROI);

        findContours(firstImage, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE );

        vector<Rect>boundRect(contours.size());

        int max=0,max_number=0;
        if(contours.size()>0)
        {
              for(unsigned int i = 0; i < contours.size(); i++ )
               {
                  boundRect[i] = boundingRect( Mat(contours[i]) );
                  if(boundRect[i].width>max)
                      {
                         max = boundRect[i].width;
                         max_number = i;
                      }
               }
            int middle_point = boundRect[max_number].x + (boundRect[max_number].width)/2 ;
//          rectangle(Fusion,boundRect[max_number],Scalar(0,0,255),1,1,0);
            line(Fusion, Point(middle_point, 0), Point(middle_point, 480), Scalar(0, 0, 255), 1);
            line(Fusion, Point(middle_point+ (boundRect[max_number].width)/2 , 0), Point(middle_point+ (boundRect[max_number].width)/2 , 480), Scalar(0, 255, 255), 1);
            line(Fusion, Point(middle_point- (boundRect[max_number].width)/2 , 0), Point(middle_point- (boundRect[max_number].width)/2 , 480), Scalar(0, 255, 255), 1);


//             cout<<middle_point+183<<endl;
        }
//       // line(Fusion, Point(320, 0), Point(320, 480), Scalar(255, 0, 0), 3);

        imshow("Fusion",Fusion);

        int t2 = getTickCount();
        cout << "time:"<<(t2 - t1) * 1000.0 / getTickFrequency()<<"ms"<<endl;

        char c = (char)waitKey(1);
        if(c == 27 )break;

    }
    capDepth.release();
    capRGB.release();
    destroyAllWindows();
    return 0;
}
