#include <iostream>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using namespace std;
using namespace cv;

#define s1 1
#define s2 25

int w1=34,h1=295,l1=149,k1=60;
int w2=183,h2=245,l2=284,k2=140;
int w3=496,h3=265,l3=127,k3=80;

int main()
{
    VideoCapture cap;

    cap.open("Depth11.avi");

    // Check if camera opened successfully
    if (!cap.isOpened())
    {
        cout << "Error opening video stream" << endl;
        return -1;
    }

    Mat dstImage,dilate1,result;

    //////////////////////////////////////////
    namedWindow("LEFT");
    createTrackbar("X","LEFT",&w1,640,NULL);
    createTrackbar("Y","LEFT",&h1,480,NULL);
    createTrackbar("WIDTH","LEFT",&l1,640,NULL);
    createTrackbar("HEIGHT","LEFT",&k1,480,NULL);
    namedWindow("MID");
    createTrackbar("X","MID",&w2,640,NULL);
    createTrackbar("Y","MID",&h2,480,NULL);
    createTrackbar("WIDTH","MID",&l2,640,NULL);
    createTrackbar("HEIGHT","MID",&k2,480,NULL);
    namedWindow("RIGHT");
    createTrackbar("X","RIGHT",&w3,640,NULL);
    createTrackbar("Y","RIGHT",&h3,480,NULL);
    createTrackbar("WIDTH","RIGHT",&l3,640,NULL);
    createTrackbar("HEIGHT","RIGHT",&k3,480,NULL);
    ///////////////////////////////////////////

    Mat elementClosed = getStructuringElement(MORPH_ELLIPSE, Size(5,5));

    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    while (1)
    {

        int t1 = getTickCount();  //getTickCount函数从0开始计时，返回自设备启动后的毫秒数（不含系统暂停时间）

        Mat src,gray;
        cap >> src;

        if (src.empty())
            break;

        cvtColor(src,gray,CV_BGR2GRAY);

        Mat element = getStructuringElement(MORPH_ELLIPSE, Size(s1,s2));

        dstImage = gray.clone();
        int row = dstImage.rows;  //行
        int col = dstImage.cols;  //列

        for(int x = 0;x<row;x++)
        {
            uchar* data = dstImage.ptr<uchar>(x); //获取第x行的头指针
            for(int y = 0;y<col;y++)
            {
                int num = (int)data[y];  //强制转换为整型数据和阈值进行比较
                if((num>14) & (num<25))
                    data[y] = 255;
                else
                    data[y] = 0;
            }
        }

        medianBlur(dstImage,dstImage, 5 );
        medianBlur(dstImage,dstImage, 5 );
        medianBlur(dstImage,dstImage, 5 );
        medianBlur(dstImage,dstImage, 5 );
        dilate(dstImage, dilate1, element);
        dilate(dstImage, dilate1, element);
//      imshow("dilate", dilate1);

        morphologyEx(dilate1, result, MORPH_CLOSE,elementClosed);

        imshow("Output", result);

        ///////////////////////////////////
        Mat Left(result,Rect(w1,h1,l1,k1));
        Mat Mid(result,Rect(w2,h2,l2,k2));
        Mat Right(result,Rect(w3,h3,l3,k3));
        ///////////////////////////////////

        imshow("Left",Left);
        imshow("Mid",Mid);
        imshow("Right",Right);

        findContours( Mid, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE );

        vector<Rect> boundRect( contours.size() );

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

             line(src, Point(middle_point+183, 0), Point(middle_point+183, 480), Scalar(0, 0, 255), 3);

             cout<<middle_point+183<<endl;
        }

        imshow("src",src);

        int t2 = getTickCount();
        cout << "time:"<<(t2 - t1) * 1000.0 / getTickFrequency()<<"ms"<<endl;

        char c = (char)waitKey(0);
        if(c == 27 )break;
    }
    cap.release();
    destroyAllWindows();
    return 0;
}

