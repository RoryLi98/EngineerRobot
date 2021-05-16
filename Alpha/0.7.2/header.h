#ifndef HEADER_H
#define HEADER_H

#include <iostream>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <OpenNI.h>
#include <iomanip>
#include "serial.h"
#define MINWIDTH 70
#define MAXWIDTH 90
#define MINHEIGHT 80
#define MAXHEIGHT 110

using namespace std;
using namespace openni;
using namespace cv;

#define s1 3
#define s2 20
int s3 = 20, s4 = 3;

union data_send_float
{
    float data_float;
    char data_uint8[4];
};

Mat getHistograph(const Mat grayImage)
{
    //定义求直方图的通道数目，从0开始索引
    int channels[] = {0};
    //定义直方图的在每一维上的大小，例如灰度图直方图的横坐标是图像的灰度值，就一维，bin的个数
    //如果直方图图像横坐标bin个数为x，纵坐标bin个数为y，则channels[]={1,2}其直方图应该为三维的，Z轴是每个bin上统计的数目
    const int histSize[] = {256};
    //每一维bin的变化范围
    float range[] = {0, 255};

    //所有bin的变化范围，个数跟channels应该跟channels一致
    const float *ranges[] = {range};

    //定义直方图，这里求的是直方图数据
    Mat hist;                                                                         //opencv中计算直方图的函数，hist大小为256*1，每行存储的统计的该行对应的灰度值的个数
    calcHist(&grayImage, 1, channels, Mat(), hist, 1, histSize, ranges, true, false); //cv中是cvCalcHist

    //找出直方图统计的个数的最大值，用来作为直方图纵坐标的高
    double maxValue = 0;
    //找矩阵中最大最小值及对应索引的函数
    minMaxLoc(hist, 0, &maxValue, 0, 0);
    //最大值取整
    int rows = cvRound(maxValue);
    //定义直方图图像，直方图纵坐标的高作为行数，列数为256(灰度值的个数)
    //因为是直方图的图像，所以以黑白两色为区分，白色为直方图的图像
    Mat histImage = Mat::zeros(rows, 256, CV_8UC1);

    //直方图图像表示
    for (int i = 0; i < 256; i++)
    {
        //取每个bin的数目
        int temp = (int)(hist.at<float>(i, 0));
        //如果bin数目为0，则说明图像上没有该灰度值，则整列为黑色
        //如果图像上有该灰度值，则将该列对应个数的像素设为白色

        if (temp && i != 0)
        {
            //            cout<<"灰度值： "<<i<<"  个数： "<<temp<<endl;
            //            由于图像坐标是以左上角为原点，所以要进行变换，使直方图图像以左下角为坐标原点
            histImage.col(i).rowRange(Range(rows - temp, rows)) = 255;
        }
    }
    //由于直方图图像列高可能很高，因此进行图像对列要进行对应的缩减，使直方图图像更直观
    Mat resizeImage;
    resize(histImage, resizeImage, Size(512, 512));
    return resizeImage;
}

void translateTransform(cv::Mat const &src, cv::Mat &dst, int dx, int dy)
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

Mat prehandle(Mat src, int low, int high) //预处理
{
    Mat dst;
    Mat element = getStructuringElement(MORPH_ELLIPSE, Size(s1, s2));
    Mat elementClosed = getStructuringElement(MORPH_RECT, Size(s3, s4));

    //    int row = src.rows;  //行
    //    int col = src.cols;  //列
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

    inRange(src, low + 1, high - 1, dst);

    medianBlur(dst, dst, 3);
    medianBlur(dst, dst, 3);
    medianBlur(dst, dst, 3);

    dilate(dst, dst, element);
    dilate(dst, dst, element);

    morphologyEx(dst, dst, MORPH_CLOSE, elementClosed);

    return dst;
}

int getMainTarget(Mat src, int a, int b) //ROI中心区域定制目标函数
{
    int Offset = 888;
    const int rows = src.rows;
    Mat maintarget = Mat(src, Rect(a, 0, b - a, rows));

    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(maintarget, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    vector<Rect> boundRect(contours.size());

    //    imshow("MainSRC",maintarget);
    //    cout<<"contours.size():"<<contours.size()<<endl;

    int max = 0, max_number = 0;
    if (contours.size() > 0)
    {
        for (unsigned int i = 0; i < contours.size(); i++)
        {
            boundRect[i] = boundingRect(Mat(contours[i]));
            //            if((boundRect[i].width>max)&&(boundRect[i].height>MINHEIGHT))
            //            if(boundRect[i].width>max)
            if ((boundRect[i].width > max) && (boundRect[i].width > 20))
            {
                cout << boundRect[i].width << endl;
                max = boundRect[i].width;
                max_number = i;
            }
        }
        if (max != 0)
        {
            int TargetCenterX = boundRect[max_number].x + (boundRect[max_number].width) / 2;
            //        cout<<"WIDTH:"<<boundRect[max_number].width<<endl;i
            //        cout<<"HEIGHT:"<<boundRect[max_number].height<<endl;
            Offset = TargetCenterX - (b - a) / 2;
            //        cout<<"OFFSET:"<<Offset<<endl;
        }
    }
    return Offset; //返回偏移量 888无目标 非888即ROI中心区域有目标
}

int ifbox(Mat src) //用来判断左右有没有箱子函数
{
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(src, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    vector<Rect> boundRect(contours.size());
    if (contours.size() > 0)
    {
        for (unsigned int i = 0; i < contours.size(); i++)
        {
            boundRect[i] = boundingRect(Mat(contours[i]));
            //              if((boundRect[i].width>MINWIDTH)&&(boundRect[i].width>MAXWIDTH)&&(boundRect[i].height>MINHEIGHT)&&(boundRect[i].height>MAXHEIGHT))
            if (boundRect[i].width > 10)
            {
                return 1;
            }
        }
    }
    return 0;
}

int getAim(Mat firstsrc, int left, int right, int Findnext) //总流程函数
{
    const int rows = firstsrc.rows;
    int aim = getMainTarget(firstsrc, left, right); //212 478 为分三份的划分值

    if (aim == 888)
    {
        if (Findnext == 1 && ifbox(Mat(firstsrc, Rect(527, 0, 60, rows))))
            return 250;
        return 888;
    }
    return aim;
}

Mat convertTo3Channels(const Mat &binImg) //单通道图像 -> 三通道图像
{
    Mat three_channel = Mat::zeros(binImg.rows, binImg.cols, CV_8UC3);
    vector<Mat> channels;
    for (int i = 0; i < 3; i++)
    {
        channels.push_back(binImg);
    }
    merge(channels, three_channel);
    return three_channel;
}

void showdevice()
{
    Array<DeviceInfo> aDeviceList;
    OpenNI::enumerateDevices(&aDeviceList);
    cout << "电脑上连接着 " << aDeviceList.getSize() << " 个设备." << endl;
    for (int i = 0; i < aDeviceList.getSize(); ++i)
    {
        const DeviceInfo &rDevInfo = aDeviceList[i];
        cout << "设备 " << i << endl;
        cout << "设备名： " << rDevInfo.getName() << endl;
        cout << "设备Id： " << rDevInfo.getUsbProductId() << endl;
        cout << "供应商名： " << rDevInfo.getVendor() << endl;
        cout << "供应商Id: " << rDevInfo.getUsbVendorId() << endl;
        cout << "设备URI: " << rDevInfo.getUri() << endl;
    }
}

void hMirrorTrans(const Mat &src, Mat &dst) //水平镜像
{
    dst.create(src.rows, src.cols, src.type());
    int rows = src.rows;
    int cols = src.cols;
    switch (src.channels())
    {
    case 1: //1通道比如深度图像
        const uchar *origal;
        uchar *p;
        for (int i = 0; i < rows; i++)
        {
            origal = src.ptr<uchar>(i);
            p = dst.ptr<uchar>(i);
            for (int j = 0; j < cols; j++)
            {
                p[j] = origal[cols - 1 - j];
            }
        }
        break;
    case 3: //3通道比如彩色图像
        const Vec3b *origal3;
        Vec3b *p3;
        for (int i = 0; i < rows; i++)
        {
            origal3 = src.ptr<Vec3b>(i);
            p3 = dst.ptr<Vec3b>(i);
            for (int j = 0; j < cols; j++)
            {
                p3[j] = origal3[cols - 1 - j];
            }
        }
        break;
    default:
        break;
    }
}

#endif // HEADER_H