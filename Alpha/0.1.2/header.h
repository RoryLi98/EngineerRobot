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
#define s2 25

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
    Mat elementClosed = getStructuringElement(MORPH_ELLIPSE, Size(2,2));

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

//    dilate(dst,dst, element);

//    morphologyEx(dst, dst, MORPH_CLOSE,elementClosed);

    return dst;
}


#endif // HEADER_H
