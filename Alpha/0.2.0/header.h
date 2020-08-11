#ifndef HEADER_H
#define HEADER_H

#include <iostream>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

#define s1 1
#define s2 30
int s3=3,s4=3;
void translateTransform(cv::Mat const& src, cv::Mat& dst, int dx, int dy)
{
    CV_Assert(src.depth() == CV_8U);

    const int rows = src.rows;
    const int cols = src.cols;

    dst.create(rows, cols, src.type());

    Vec3b *p;
    for (int i = 0; i < rows; i++)
    {
        p = dst.ptr<Vec3b>(i);
        for (int j = 0; j < cols; j++)
        {
            //平移后坐标映射到原图像
            int x = j - dx;
            int y = i - dy;

            //保证映射后的坐标在原图像范围内
            if (x >= 0 && y >= 0 && x < cols && y < rows)
                p[j] = src.ptr<Vec3b>(y)[x];
        }
    }
}

Mat prehandle(Mat src,int low,int high)
{
//    int row = src.rows;  //行
//    int col = src.cols;  //列
    Mat dst;
    Mat element = getStructuringElement(MORPH_ELLIPSE, Size(s1,s2));
    Mat elementClosed = getStructuringElement(MORPH_RECT, Size(s3,s4));
    //Mat elementClosed = getStructuringElement(MORPH_ELLIPSE, Size(3,3));
//    for(int x = 0;x<row;x++)
//    {
//        uchar* data = src.ptr<uchar>(x); //获取第x行的头指针
//        for(int y = 0;y<col;y++)
//        {
//            int num = (int)data[y];  //强制转换为整型数据和阈值进行比较
//            if((num>low) & (num<high))
//                data[y] = 255;
//            else
//                data[y] = 0;
//        }
//    }

    inRange(src, low+1, high-1, dst);

    medianBlur(dst,dst,3);
    medianBlur(dst,dst,3);
    medianBlur(dst,dst,3);
    medianBlur(dst,dst,3);
    medianBlur(dst,dst,3);
    medianBlur(dst,dst,3);
    medianBlur(dst,dst,3);
    medianBlur(dst,dst,3);

    dilate(dst,dst, element);

    morphologyEx(dst, dst, MORPH_CLOSE,elementClosed);

    return dst;
}

bool judge_direction(int center,int flag)
{
    int time;
    static vector<int>list;
    if(flag==1)
        {
          for(unsigned i=0;i<list.size();i++);

        }
    else
        list.push_back(center);

    return 0;
}

Rect process(Mat& src,Mat& dst)
{

    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    findContours(src, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE );

    vector<Rect>boundRect(contours.size());

    int max=0,max_number=0;
    if(contours.size()>0)
    {
          for(unsigned int i = 0; i < contours.size(); i++ )
           {
              boundRect[i] = boundingRect( Mat(contours[i]) );
           // if((boundRect[i].width>max)&(boundRect[i].height>MINHEIGHT))
              if(boundRect[i].width>max)
                  {
                     max = boundRect[i].width;
                     max_number = i;
                  }
           }

        if(max!=0)
        {

        int middle_point = boundRect[max_number].x + (boundRect[max_number].width)/2 ;
        rectangle(dst,boundRect[max_number],Scalar(0,0,255),1,1,0);
        line(dst, Point(middle_point, 0), Point(middle_point, 480), Scalar(0, 0, 255), 1);
        line(dst, Point(middle_point+ (boundRect[max_number].width)/2 , 0), Point(middle_point+ (boundRect[max_number].width)/2 , 480), Scalar(0, 255, 255), 1);
        line(dst, Point(middle_point- (boundRect[max_number].width)/2 , 0), Point(middle_point- (boundRect[max_number].width)/2 , 480), Scalar(0, 255, 255), 1);
        rectangle(dst,boundRect[max_number],Scalar(0,0,255),1,1,0);
        cout<<"WIDTH:"<<boundRect[max_number].width<<endl;
        cout<<"HEIGHT:"<<boundRect[max_number].height<<endl;

        }

    }
    return Rect(0,0,0,0);

}
class target
{
public:
    int position1=0;
    int position2=0;
    int position3=0;
    int position4=0;
    int position5=0;
    int position6=0;
    int leftflag = 0 ;
    int rightflag = 0 ;
private:
    bool getdirection()      //0无1左2右
    {
       if(position1==1||position4==1)leftflag = 1;
       if(position1==3||position6==1)rightflag = 1;

    }
};
#endif // HEADER_H
