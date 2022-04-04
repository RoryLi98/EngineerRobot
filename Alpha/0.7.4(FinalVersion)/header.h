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

float getDistance(CvPoint pointO, CvPoint pointA)
{
    float distance;
    distance = powf((pointO.x - pointA.x), 2) + powf((pointO.y - pointA.y), 2);
    distance = sqrtf(distance);
    return distance;
}

double Entropy(cv::Mat img)
{
    double temp[256] = {0.0};

    // 计算每个像素的累积值
    for (int m = 0; m < img.rows; m++)
    { // 有效访问行列的方式
        const uchar *t = img.ptr<uchar>(m);
        for (int n = 0; n < img.cols; n++)
        {
            int i = t[n];
            temp[i] = temp[i] + 1;
        }
    }

    // 计算每个像素的概率
    for (int i = 0; i < 256; i++)
    {
        temp[i] = temp[i] / (img.rows * img.cols);
    }

    double result = 0;
    // 计算图像信息熵
    for (int i = 0; i < 256; i++)
    {
        if (temp[i] == 0.0)
            result = result;
        else
            result = result - temp[i] * (log(temp[i]) / log(2.0));
    }
    return result;
}

int TurnOre(cv::Mat source)
{
    cv::Mat img = source;

    //img = img(cv::Rect(220,140,300,300));

    cv::Mat gray;
    cv::Mat dst;

//    int AbleToSwitch = 0;
//    int yaw = 0;
//    int pitch = 0;


    cvtColor(img, gray, CV_BGR2GRAY);
    threshold(gray, dst, 0, 255, CV_THRESH_OTSU);

    cv::Mat image = dst;

    cv::GaussianBlur(dst, image, cv::Size(3, 3), 0);

    imshow("OTSU", image);
    cv::Mat ROI = image.clone();

    Canny(image, image, 10, 255);

    int count = 0;
    vector<vector<cv::Point>> contours;
    vector<cv::Vec4i> hierarchy;
    cv::findContours(image, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);
    //        Mat imageContours=Mat::zeros(image.size(),CV_8UC1);
    cv::Mat Contours = cv::Mat::zeros(image.size(), CV_8UC1); //绘制
    vector<vector<cv::Point>> hull(contours.size());          //用于存放凸包
    vector<float> length(contours.size());                    //用于保存每个轮廓的长度
    vector<float> Area_contours(contours.size()), Area_hull(contours.size()), Rectangularity(contours.size()), circularity(contours.size());
    vector<vector<cv::Point2f>> DoubleRectangles;
    vector<vector<cv::Point2f>> Squares;

    float C=0;

    for (int i = 0; i < contours.size(); i++)
    {
        vector<cv::Point2f> TempPoint;
        int PointSize = 0;
        cv::RotatedRect Rect = minAreaRect(contours[i]);
        length[i] = arcLength(contours[i], true);                                   //轮廓的长度
        convexHull(cv::Mat(contours[i]), hull[i], false);                           //把凸包找出来，寻找凸包函数
        Area_contours[i] = contourArea(contours[i]);                                //轮廓面积
        Area_hull[i] = contourArea(hull[i]);                                        //凸包面积
        Rectangularity[i] = Area_contours[i] / Area_hull[i];                        //矩形度
        circularity[i] = (4 * 3.1415 * Area_contours[i]) / (length[i] * length[i]); //圆形度

        //      if(contours[i].size()>50&&contours[i].size()<150)
        //      abs(Rect.size.width/Rect.size.height)<1.2
        //      if(abs(Rect.size.width/Rect.size.height)<1.1&&abs(Rect.size.width/Rect.size.height)>0.9&&contours[i].size()>60&&contours[i].size()<180)
        //      if(abs(Rect.size.width/Rect.size.height)<1.2&&abs(Rect.size.width/Rect.size.height)>0.8&&contours[i].size()>60&&Rectangularity[i]>0.7)
        if (abs(Rect.size.width / Rect.size.height) < 1.3 && abs(Rect.size.width / Rect.size.height) > 0.7 && contours[i].size() > 80 && Rectangularity[i] > 0.6 && hierarchy[i][2] == -1)
        {
            cv::Point2f vertices[4];
            Rect.points(vertices);

            for (int j = 0; j < 4; j++) //逐条边绘制
            {
                cv::line(Contours, vertices[j], vertices[(j + 1) % 4], cv::Scalar(255));
                cv::Point2f Temp = vertices[j];
                float MinDistance = 888;
                cv::Point2f MinPoint = cv::Point(-1, -1);
                for (int k = 0; k < contours[i].size(); k++)
                {
                    float Distance;
                    //                  cout<<contours[i][k].x<<" "<<contours[i][k].y<<endl;
                    Distance = getDistance(Temp, contours[i][k]);
                    if (Distance < MinDistance)
                    {
                        MinDistance = Distance;
                        MinPoint = contours[i][k];
                    }
                }
                if (MinDistance < (Rect.size.width + Rect.size.height) / 5)
                {
                    circle(Contours, MinPoint, 2, cv::Scalar(255), 3, CV_AA, 0); //中途显示
                                                                                 //                  cv::Point2f P=Point(MinPoint.x,MinPoint.y);
                    TempPoint.push_back(MinPoint);
                    //                  cout<<TempPoint.size()<<endl;
                    PointSize++;
                }
            }

            count++;
            //        cout<<"Area_contours[i]:"<<Area_contours[i]<<endl;
            //        cout<<"Area_hull[i]:"<<Area_hull[i]<<endl;
            //        cout<<"Rectangularity[i]:"<<Rectangularity[i]<<endl;
            //        cout<<"circularity[i]:"<<circularity[i]<<endl;

            for (int j = 0; j < contours[i].size(); j++)
            {
                //绘制出contours向量内所有的像素点
                cv::Point P = cv::Point(contours[i][j].x, contours[i][j].y);
                Contours.at<uchar>(P) = 255;
            }

            //        imshow("Coage",temp); //轮廓
            //        char a = (char)waitKey(0);
        }

        //      cout<<TempPoint.size()<<endl;
        if (PointSize == 3)
        {
            float min = 888;
            int MinPointNum = -1;
            for (int i = 0; i < 3; i++)
            {
                float a = getDistance(TempPoint[i], TempPoint[(i + 1) % 3]);
                float b = getDistance(TempPoint[i], TempPoint[(i + 2) % 3]);
                if (abs(a - b) < min)
                {
                    min = abs(a - b);
                    MinPointNum = i;
                }
                //              cout<<"abs(a-b):"<<abs(a-b)<<"a:"<<a<<"b:"<<b<<endl;
                //              cout<<PointSize<<endl;
            }
            //          if(abs(((TempPoint[(MinPointNum + 1) % 3].y- TempPoint[MinPointNum].y)/(TempPoint[(MinPointNum + 1) % 3].x- TempPoint[MinPointNum].x)*(TempPoint[(MinPointNum + 2) % 3].y- TempPoint[MinPointNum].y)/(TempPoint[(MinPointNum + 2) % 3].x- TempPoint[MinPointNum].x))+1));
            //          cout<<abs((TempPoint[(MinPointNum + 1) % 3].y- TempPoint[MinPointNum].y)/(TempPoint[(MinPointNum + 1) % 3].x- TempPoint[MinPointNum].x))<<endl;

            float a = getDistance(TempPoint[MinPointNum], TempPoint[(MinPointNum + 1) % 3]);
            float b = getDistance(TempPoint[MinPointNum], TempPoint[(MinPointNum + 2) % 3]);
            float c = getDistance(TempPoint[(MinPointNum + 1) % 3], TempPoint[(MinPointNum + 2) % 3]);

            if (abs(a * a + b * b - c * c) < c * c * 0.3) //可调
            {
                vector<cv::Point2f> Temp;
                Temp.push_back(TempPoint[MinPointNum]);
                if (abs(TempPoint[(MinPointNum + 1) % 3].y - TempPoint[MinPointNum].y) < abs(TempPoint[(MinPointNum + 2) % 3].y - TempPoint[MinPointNum].y))
                {
                    Temp.push_back(TempPoint[(MinPointNum + 1) % 3]);
                    Temp.push_back(TempPoint[(MinPointNum + 2) % 3]);
                }
                else
                {
                    Temp.push_back(TempPoint[(MinPointNum + 2) % 3]);
                    Temp.push_back(TempPoint[(MinPointNum + 1) % 3]);
                }
                circle(img, Temp[0], 2, cv::Scalar(0, 0, 255), 3, CV_AA, 0); //中途显示
                circle(img, Temp[1], 2, cv::Scalar(0, 255, 0), 3, CV_AA, 0); //中途显示
                circle(img, Temp[2], 2, cv::Scalar(255, 0, 0), 3, CV_AA, 0); //中途显示
                DoubleRectangles.push_back(Temp);
            }
        }

        if (PointSize == 4)
        {
            int SquareFlag = 1;
            for (int i = 0; i < 4; i++)
            {
                float a = getDistance(TempPoint[i], TempPoint[(i + 1) % 4]);
                float b = getDistance(TempPoint[i], TempPoint[(i + 2) % 4]);
                float c = getDistance(TempPoint[i], TempPoint[(i + 3) % 4]);
                vector<float> TempDistances;
                TempDistances.push_back(a);
                TempDistances.push_back(b);
                TempDistances.push_back(c);
                sort(TempDistances.begin(), TempDistances.end());
                //cout<<abs(TempDistances[0]*TempDistances[0]+TempDistances[1]*TempDistances[1]-TempDistances[2]*TempDistances[2])<<"::::"<<(abs(TempDistances[0]*TempDistances[0]+TempDistances[1]*TempDistances[1]-TempDistances[2]*TempDistances[2]))/(TempDistances[2]*TempDistances[2])<<endl;
                if (abs(TempDistances[0] * TempDistances[0] + TempDistances[1] * TempDistances[1] - TempDistances[2] * TempDistances[2]) >
                    TempDistances[2] * TempDistances[2] * 0.3) //可调
                {
                    SquareFlag = 0;
                    break;
                }
            }
            if (SquareFlag)
            {
                circle(img, TempPoint[0], 2, cv::Scalar(0, 255, 255), 3, CV_AA, 0); //中途显示
                circle(img, TempPoint[1], 2, cv::Scalar(0, 255, 255), 3, CV_AA, 0); //中途显示
                circle(img, TempPoint[2], 2, cv::Scalar(0, 255, 255), 3, CV_AA, 0); //中途显示
                circle(img, TempPoint[3], 2, cv::Scalar(0, 255, 255), 3, CV_AA, 0); //中途显示
                vector<cv::Point2f> Temp;
                Temp.push_back(TempPoint[0]);
                Temp.push_back(TempPoint[1]);
                Temp.push_back(TempPoint[2]);
                Temp.push_back(TempPoint[3]);
                Squares.push_back(Temp);
            }
        }

    } //for循环结束

    cv::Point2f KeyPoint1 = cv::Point(-1, -1);
    cv::Point2f KeyPoint2 = cv::Point(-1, -1);
    cv::Point2f MinPointA = cv::Point(-1, -1);
    cv::Point2f MinPointB = cv::Point(-1, -1);
    if (DoubleRectangles.size() > 2 && Squares.size() > 0)
    {
        float min = 888, A=0, B=0;
        //                float min = 888,A,B,C;
        cv::Point2f MinPoint = cv::Point(-1, -1);

        for (int i = 0; i < DoubleRectangles.size() - 2; i++)
        {
            for (int j = i + 1; j < DoubleRectangles.size() - 1; j++)
            {
                for (int k = j + 1; k < DoubleRectangles.size(); k++)
                {

                    vector<cv::Point2f> Temp(3);
                    Temp[0] = DoubleRectangles[i][0];
                    Temp[1] = DoubleRectangles[j][0];
                    Temp[2] = DoubleRectangles[k][0];

                    for (int l = 0; l < 3; l++)
                    {
                        float a = getDistance(Temp[l], Temp[(l + 1) % 3]);
                        float b = getDistance(Temp[l], Temp[(l + 2) % 3]);
                        if (abs(a - b) < min)
                        {
                            min = abs(a - b);
                            MinPoint = Temp[l];
                            MinPointA = Temp[(l + 1) % 3];
                            MinPointB = Temp[(l + 2) % 3];
                            A = a;
                            B = b;
                            C = getDistance(Temp[(l + 1) % 3], Temp[(l + 2) % 3]);
                        }
                    }
                }
            }
        }

        if (abs(A * A + B * B - C * C) < C * C * 0.3) //TempDistances
        {
            KeyPoint1 = MinPoint;
            //                      circle(img, KeyPoint1 , 3, cv::Scalar(106,68,20), 3, CV_AA, 0);         //中途显示
            float Verticalrate = 888888;
            cv::Point2f BestPoint = cv::Point(-1, -1);
            for (int i = 0; i < Squares.size(); i++)
            {
                float k1tok2Max = -1;
                cv::Point2f MaxPoint = cv::Point(-1, -1);
                for (int j = 0; j < 4; j++)
                {
                    float k1tok2 = getDistance(KeyPoint1, Squares[i][j]);
                    if (k1tok2 > k1tok2Max)
                    {
                        k1tok2Max = k1tok2;
                        //                               cout<<k1tok2<<endl;
                        MaxPoint = Squares[i][j];
                    }
                }
                //                      circle(img, MaxPoint , 5, cv::Scalar(0,0,0), 5, CV_AA, 0);          //中途显示
                float a = getDistance(MaxPoint, MinPointA);
                float b = getDistance(MaxPoint, MinPointB);
                //                          cout<<abs(a*a+b*b-C*C)<<endl;
                if (Verticalrate > abs(a * a + b * b - C * C)) //可调
                {
                    Verticalrate = abs(a * a + b * b - C * C);
                    BestPoint = MaxPoint;
                }
            }
            //                      cout<<Verticalrate<<endl;
            if (Verticalrate != 888888)
            {
                KeyPoint2 = BestPoint;
                //                          circle(img, KeyPoint2 , 3, cv::Scalar(106,68,20), 3, CV_AA, 0);         //中途显示
            }
        }

//        float kkk = getDistance(KeyPoint1, KeyPoint2);
        //                  cout<< abs(kkk-C)<<endl;
        //                  cout<< KeyPoint1.x<<"   "<< KeyPoint2.x<<endl;
        if (KeyPoint1.x != -1 && KeyPoint2.x != -1 && abs(getDistance(KeyPoint1, KeyPoint2) - C) < 250) //可调
        {
            cv::line(img, KeyPoint1, MinPointA, cv::Scalar(0, 0, 255), 10);
            cv::line(img, KeyPoint1, MinPointB, cv::Scalar(0, 0, 255), 10);
            cv::line(img, MinPointA, MinPointB, cv::Scalar(0, 0, 255), 10);
            cv::line(img, KeyPoint1, KeyPoint2, cv::Scalar(255, 0, 0), 10); //KeyPoint1三角形直角点 KeyPoint2正方形角点

            cv::Point2f TempCenter = cv::Point2f(-1, -1);
            TempCenter.x = (KeyPoint1.x + KeyPoint2.x) / 2;
            TempCenter.y = (KeyPoint1.y + KeyPoint2.y) / 2;

            if (KeyPoint2.x > TempCenter.x && KeyPoint2.y > TempCenter.y)
            {
//                cout << "右下 0 -1" << endl;
//                AbleToSwitch = 1;
//                yaw = 0;
//                pitch = -1;
                return 1;
            }

            if (KeyPoint2.x < TempCenter.x && KeyPoint2.y > TempCenter.y)
            {
//                cout << "左下 -1 -1" << endl;
//                AbleToSwitch = 1;
//                yaw = -1;
//                pitch = -1;
                return 2;
            }

            if (KeyPoint2.x < TempCenter.x && KeyPoint2.y < TempCenter.y)
            {
//                cout << "左上 0 1" << endl;
//                AbleToSwitch = 1;
//                yaw = 0;
//                pitch = 1;
                return 3;
            }

            if (KeyPoint2.x > TempCenter.x && KeyPoint2.y < TempCenter.y)
            {
//                cout << "右上 1 -1" << endl;
//                AbleToSwitch = 1;
//                yaw = 1;
//                pitch = -1;
                return 4;
            }
        }
    }

    //==============================================================================================================

    if (DoubleRectangles.size() > 1 && Squares.size() > 1)
    {
        cv::Point2f TempCenter = cv::Point2f(-1, -1);
        int Found = 0;
        vector<cv::Point> ResultKeyPoint;
        for (int i = 0; i < DoubleRectangles.size() - 1; i++)
        {
            for (int j = i + 1; j < DoubleRectangles.size(); j++)
            {
                TempCenter.x = (DoubleRectangles[i][0].x + DoubleRectangles[j][0].x) / 2;
                TempCenter.y = (DoubleRectangles[i][0].y + DoubleRectangles[j][0].y) / 2;

                vector<cv::Point> TempSquareKeyPoint;
                for (int k = 0; k < Squares.size(); k++)
                {
                    int MaxDistance = -1;
                    int MaxNumber = -1;
                    for (int l = 0; l < 4; l++)
                    {
                        if (MaxDistance < getDistance(TempCenter, Squares[k][l]))
                        {
                            MaxDistance = getDistance(TempCenter, Squares[k][l]);
                            MaxNumber = l;
                        }
                    }
                    TempSquareKeyPoint.push_back(Squares[k][MaxNumber]);
                }

                for (int m = 0; m < TempSquareKeyPoint.size() - 1; m++)
                {
                    for (int n = m + 1; n < TempSquareKeyPoint.size(); n++)
                    {
                        if (abs(getDistance(TempCenter, TempSquareKeyPoint[n]) - getDistance(TempCenter, TempSquareKeyPoint[m])) < 0.2 * (getDistance(TempCenter, TempSquareKeyPoint[n]) + getDistance(TempCenter, TempSquareKeyPoint[m])) / 2)
                        {
                            cv::line(img, TempSquareKeyPoint[n], TempSquareKeyPoint[m], cv::Scalar(0, 0, 255), 10);
                            Found = 1;
                            float DistanceRatio = getDistance(TempSquareKeyPoint[m], TempSquareKeyPoint[n]);
                            cv::Mat BarcodeROI;
                            //cout<<TempSquareKeyPoint[n]<<TempSquareKeyPoint[m]<<endl;
                            if (TempSquareKeyPoint[n].y > TempSquareKeyPoint[m].y && TempSquareKeyPoint[n].x > TempSquareKeyPoint[m].x)
                                BarcodeROI = ROI(cv::Rect(TempSquareKeyPoint[m].x + 0.15 * DistanceRatio, TempSquareKeyPoint[m].y + 0.15 * DistanceRatio, abs(TempSquareKeyPoint[m].x - TempSquareKeyPoint[n].x) * 0.6, abs(TempSquareKeyPoint[m].y - TempSquareKeyPoint[n].y) * 0.6));
                            else if (TempSquareKeyPoint[n].y > TempSquareKeyPoint[m].y && TempSquareKeyPoint[n].x < TempSquareKeyPoint[m].x)
                                BarcodeROI = ROI(cv::Rect(TempSquareKeyPoint[n].x + 0.15 * DistanceRatio, TempSquareKeyPoint[m].y + 0.15 * DistanceRatio, abs(TempSquareKeyPoint[m].x - TempSquareKeyPoint[n].x) * 0.6, abs(TempSquareKeyPoint[m].y - TempSquareKeyPoint[n].y) * 0.6));
                            else if (TempSquareKeyPoint[n].y < TempSquareKeyPoint[m].y && TempSquareKeyPoint[n].x > TempSquareKeyPoint[m].x)
                                BarcodeROI = ROI(cv::Rect(TempSquareKeyPoint[m].x + 0.15 * DistanceRatio, TempSquareKeyPoint[n].y + 0.15 * DistanceRatio, abs(TempSquareKeyPoint[m].x - TempSquareKeyPoint[n].x) * 0.6, abs(TempSquareKeyPoint[m].y - TempSquareKeyPoint[n].y) * 0.6));
                            else if (TempSquareKeyPoint[n].y < TempSquareKeyPoint[m].y && TempSquareKeyPoint[n].x < TempSquareKeyPoint[m].x)
                                BarcodeROI = ROI(cv::Rect(TempSquareKeyPoint[n].x + 0.15 * DistanceRatio, TempSquareKeyPoint[n].y + 0.15 * DistanceRatio, abs(TempSquareKeyPoint[m].x - TempSquareKeyPoint[n].x) * 0.6, abs(TempSquareKeyPoint[m].y - TempSquareKeyPoint[n].y) * 0.6));
                            cv::imshow("Barcode", BarcodeROI);
                            cout << "Entropy:" << Entropy(BarcodeROI) << endl;
                            if (Entropy(BarcodeROI) > 1)
                            {
                                //Found = 2;//找到条码面
//                                cout << "条码 0 2" << endl;
//                                AbleToSwitch = 1;
//                                yaw = 0;
//                                pitch = 2;
                                return 5;
                            }
                            else
                            {
//                                cout << "顶面 0 0" << endl;
//                                AbleToSwitch = 1;
//                                yaw = 0;
//                                pitch = 0;
                                return 6;
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}


#endif // HEADER_H
