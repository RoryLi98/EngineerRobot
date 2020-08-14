#include <iostream>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <header.h>
#include <iomanip>
using namespace std;
using namespace cv;


int main()
{
    VideoCapture capDepth;
    VideoCapture capRGB;
    capDepth.open("Depth.avi");
    capRGB.open("RGB.avi");
    int move1=65,move2=42;
    Target target;
    Mat firstsrc,secondsrc,result1,result;

    //////////////////////////////////////////
    int cut_min=200,cut_max=420;
    namedWindow("Height");
    createTrackbar("Height_min","Height",&cut_min,480,NULL);
    createTrackbar("Height_max","Height",&cut_max,480,NULL);
    namedWindow("Height1");
    createTrackbar("Height_min","Height1",&move1,640,NULL);
    createTrackbar("Height_max","Height1",&move2,480,NULL);
    namedWindow("Height2");
    createTrackbar("Height_min","Height2",&s3,20,NULL);
    createTrackbar("Height_max","Height2",&s4,20,NULL);
    ///////////////////////////////////////////

    vector<int> list;

    while (1)
    {

//        int t1 = getTickCount();  //getTickCount函数从0开始计时，返回自设备启后的毫秒数（不含系统暂停时间）
        Mat DEPTH,gray;
        Mat Fusion,RGB;
        capDepth >> DEPTH;
        capRGB >> RGB;
        Mat RGBtransform;
        translateTransform(RGB,RGBtransform,move1-50,move2-50);

 //     Fusion = 0.2*RGBtransform +0.1*DEPTH ;
//        Fusion = RGBtransform ;
        Fusion = DEPTH;
        if (DEPTH.empty())break;
        Mat ROI(DEPTH,Rect(0,cut_min,640,cut_max-cut_min));
        cvtColor(ROI,gray,CV_BGR2GRAY);
        imshow("ROI",ROI);
        result = gray.clone();
        result1 = gray.clone();
        firstsrc = prehandle(result,14,20);
        secondsrc = prehandle(result1,19,25);
        imshow("result",firstsrc);
        imshow("result1",secondsrc);

        vector<int>result(2);
        int aim = getAim(firstsrc,secondsrc);

        if(aim>2000)
        {
            line(Fusion, Point(aim-2500+320, 0), Point(aim-2500+320, 480), Scalar( 0, 255,0), 1);
            result[0]=2;
            result[1]=aim%1000-500;
        }
        else if(aim>1000)
        {
            line(Fusion, Point(aim-1500+320, 0), Point(aim-1500+320, 480), Scalar( 0, 0,255), 1);
            result[0]=1;
            result[1]=aim%1000-500;
        }
        else if(aim==250)
        {
            arrowedLine(Fusion,Point(520,100),Point(600,100),Scalar(0,0,255),4);
            result[1]=250;
        }
        else if(aim==-250)
        {
            arrowedLine(Fusion,Point(40,100),Point(120,100),Scalar(0,0,255),4);
            result[1]=-250;
        }
        else if(aim==888)
        {
            if(target.leftflag==1)
                result[1]= -250;
            else if(target.rightflag==1)
                result[1]= 250;
            else if(target.rightflag==0&&target.leftflag==0)
                result[1]= 888;
        }

        cout<<"---------------"<<endl;
        cout<<"|4="<<target.position4<<" 5 ="<<target.position5<<" 6="<<target.position6<<"|"<<endl;
        cout<<"|1="<<target.position1<<"aim="<<result[1]<<" 3="<<target.position3<<"|"<<endl;
        cout<<"---------------"<<endl;

        if(result[1]<10&&result[1]>-10)
        {
            getAllInformation(firstsrc,secondsrc,target,120,520);
            target.refreshDirection();
        }



        imshow("Fusion",Fusion);

//        int t2 = getTickCount();
//        cout << "time:"<<(t2 - t1) * 1000.0 / getTickFrequency()<<"ms"<<endl;

        char c = (char)waitKey(1);
        if(c == 27 )break;

    }
    capDepth.release();
    capRGB.release();
    destroyAllWindows();
    return 0;
}
