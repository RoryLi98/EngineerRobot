
//  OSTU 二值化 找轮廓 画轮廓

#include <iostream>
#include <opencv2/opencv.hpp>
#include <algorithm>
using namespace std;
using namespace cv;

float getDistance(CvPoint pointO,CvPoint pointA )
{
    float distance;
    distance = powf((pointO.x - pointA.x),2) + powf((pointO.y - pointA.y),2);
    distance = sqrtf(distance);
    return distance;
}

int main()
{
    cv::VideoCapture capture(0);

    if (!capture.isOpened())
    {
       std::cout << "Read video Failed !" << std::endl;
       return 0;
    }
    Mat img;

    while(true)
    {
    capture >> img;

    if (img.empty())
    {
        cout <<"Error: Could not load image" <<endl;
        return 0;
    }

    char c = (char)waitKey(1);
    if(c == 27 )break;            //ESC 退出

    Mat gray;
    cvtColor(img, gray, CV_BGR2GRAY);

    Mat dst;
    threshold(gray, dst, 0, 255, CV_THRESH_OTSU);

//    double scale = 1;
//    Size dsize = Size(img.cols*scale, img.rows*scale);
//    resize(img, img, dsize);
//    resize(gray, gray, dsize);
//    resize(dst, dst, dsize);
//    imshow("dst", dst);

    Mat image;
    image = dst;
    GaussianBlur(dst,image,Size(3,3),0);

//    imshow("OTSU1",image);
//    medianBlur(image,image,3);
//    medianBlur(image,image,3);
//    medianBlur(image,image,3);

    imshow("OTSU",image);

    Canny(image,image,10,255);


//    Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
//    morphologyEx(image, image, MORPH_OPEN, element);//闭运算滤波
    int count=0;
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(image,contours,hierarchy,CV_RETR_TREE,CV_CHAIN_APPROX_NONE);
    Mat imageContours=Mat::zeros(image.size(),CV_8UC1);
    Mat Contours=Mat::zeros(image.size(),CV_8UC1);  //绘制
    vector<vector<Point>> hull(contours.size());//用于存放凸包
    vector<float> length(contours.size());//用于保存每个轮廓的长度
    vector<float> Area_contours(contours.size()), Area_hull(contours.size()), Rectangularity(contours.size()), circularity(contours.size());
    vector<vector<Point>> DoubleRectangle;


    for(int i=0;i<contours.size();i++)
    {
      vector<Point2f> TempPoint;
//      TempPoint.clear();
      int PointSize = 0;
      RotatedRect Rect =  minAreaRect(contours[i]);
      length[i] = arcLength(contours[i], true);//轮廓的长度
      convexHull(Mat(contours[i]), hull[i], false);//把凸包找出来，寻找凸包函数
      Area_contours[i] = contourArea(contours[i]); //轮廓面积
      Area_hull[i] = contourArea(hull[i]); //凸包面积
      Rectangularity[i] = Area_contours[i] / Area_hull[i]; //矩形度
      circularity[i] = (4 * 3.1415*Area_contours[i]) / (length[i] * length[i]);//圆形度
//      Rect.size.width  Rect.size.height
//      cout<<<<endl;
//      if(contours[i].size()>50&&contours[i].size()<150)
//      abs(Rect.size.width/Rect.size.height)<1.2
//      if(abs(Rect.size.width/Rect.size.height)<1.1&&abs(Rect.size.width/Rect.size.height)>0.9&&contours[i].size()>60&&contours[i].size()<180)
//      if(abs(Rect.size.width/Rect.size.height)<1.2&&abs(Rect.size.width/Rect.size.height)>0.8&&contours[i].size()>60&&Rectangularity[i]>0.7)
      if(abs(Rect.size.width/Rect.size.height)<1.2&&abs(Rect.size.width/Rect.size.height)>0.8&&contours[i].size()>60&&Rectangularity[i]>0.7&&hierarchy[i][2]==-1)
      {
          Point2f vertices[4];
          Rect.points(vertices);

          //逐条边绘制
          for (int j = 0; j < 4; j++)
          {
              cv::line(Contours, vertices[j], vertices[(j + 1) % 4], cv::Scalar(255));
              Point2f Temp =  vertices[j];
              float MinDistance = 888;
              Point2f MinPoint = Point(-1,-1);
              for(int k=0;k<contours[i].size();k++)
              {
                  float Distance;
//                  cout<<contours[i][k].x<<" "<<contours[i][k].y<<endl;
                  Distance = getDistance(Temp,contours[i][k]);
                  if(Distance < MinDistance)
                  {
                      MinDistance = Distance;
                      MinPoint = contours[i][k];
                  }


              }
              if(MinDistance<(Rect.size.width+Rect.size.height)/4)
              {
                  circle(Contours, MinPoint, 2, Scalar(255), 3, CV_AA, 0);
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

          for(int j=0;j<contours[i].size();j++)
          {
              //绘制出contours向量内所有的像素点
              Point P=Point(contours[i][j].x,contours[i][j].y);
              Contours.at<uchar>(P)=255;
          }

//        imshow("Coage",temp); //轮廓
//        char a = (char)waitKey(0);
       }

//      cout<<TempPoint.size()<<endl;
      if(PointSize==3)
      {
          float min = 888;
          int MinPointNum = -1;
          for(int i=0;i<3;i++)
          {
              float a = getDistance(TempPoint[i], TempPoint[(i + 1) % 3]);
              float b = getDistance(TempPoint[i], TempPoint[(i + 2) % 3]);
              if(abs(a-b)<min)
              {
                  min = abs(a-b);
                  MinPointNum = i;
              }
//              cout<<"abs(a-b):"<<abs(a-b)<<"a:"<<a<<"b:"<<b<<endl;
//              cout<<PointSize<<endl;
          }
//          if(abs(((TempPoint[(MinPointNum + 1) % 3].y- TempPoint[MinPointNum].y)/(TempPoint[(MinPointNum + 1) % 3].x- TempPoint[MinPointNum].x)*(TempPoint[(MinPointNum + 2) % 3].y- TempPoint[MinPointNum].y)/(TempPoint[(MinPointNum + 2) % 3].x- TempPoint[MinPointNum].x))+1));
//          cout<<abs((TempPoint[(MinPointNum + 1) % 3].y- TempPoint[MinPointNum].y)/(TempPoint[(MinPointNum + 1) % 3].x- TempPoint[MinPointNum].x))<<endl;
          vector<Point2f> Temp;
          Temp.push_back(TempPoint[MinPointNum]);
          if(abs(TempPoint[(MinPointNum + 1) % 3].y- TempPoint[MinPointNum].y)< abs(TempPoint[(MinPointNum + 2) % 3].y- TempPoint[MinPointNum].y))
          {
              Temp.push_back(TempPoint[(MinPointNum + 1) % 3]);
              Temp.push_back(TempPoint[(MinPointNum + 2) % 3]);
          }
          else
          {
              Temp.push_back(TempPoint[(MinPointNum + 2) % 3]);
              Temp.push_back(TempPoint[(MinPointNum + 1) % 3]);
          }
          circle(img, Temp[0], 2, Scalar(0,0,255), 3, CV_AA, 0);
          circle(img, Temp[1], 2, Scalar(0,255,0), 3, CV_AA, 0);
          circle(img, Temp[2], 2, Scalar(255,0,0), 3, CV_AA, 0);
      }

      if(PointSize==4)
      {
          int SquareFlag = 1;
          for(int i=0;i<4;i++)
          {
              float a = getDistance(TempPoint[i], TempPoint[(i + 1) % 4]);
              float b = getDistance(TempPoint[i], TempPoint[(i + 2) % 4]);
              float c = getDistance(TempPoint[i], TempPoint[(i + 3) % 4]);
              vector<float>Temp;
              Temp.push_back(a);
              Temp.push_back(b);
              Temp.push_back(c);
              sort(Temp.begin(),Temp.end());
//              cout<<Temp[0]<<":"<<Temp[1]<<":"<<Temp[2]<<endl;
              cout<<Temp[0]*Temp[0]+Temp[1]*Temp[1]<<"==="<<Temp[2]*Temp[2]<<endl;
//              cout<<PointSize<<endl;
              if(abs(Temp[0]*Temp[0]+Temp[1]*Temp[1]-Temp[2]*Temp[2])>Temp[2]*Temp[2]*0.2)
              {
                  SquareFlag = 0;
                  break;
              }
          }
          if(SquareFlag)
          {
              circle(img, TempPoint[0], 2, Scalar(0,255,255), 3, CV_AA, 0);
              circle(img, TempPoint[1], 2, Scalar(0,255,255), 3, CV_AA, 0);
              circle(img, TempPoint[2], 2, Scalar(0,255,255), 3, CV_AA, 0);
              circle(img, TempPoint[3], 2, Scalar(0,255,255), 3, CV_AA, 0);
          }
//          circle(img, TempPoint[MinPointNum], 2, Scalar(0,255,255), 3, CV_AA, 0);
      }




//           drawContours(imageContours,contours,i,Scalar(255),1,8,hierarchy);

    }
//        imshow("Contours Image",imageContours); //轮廓
        imshow("Contours Image",Contours); //轮廓

//        Mat Cdst , corner_img;//corner_img存放检测后的角点图像
//        cornerHarris(Contours, corner_img, 2,3, 0.01);//cornerHarris角点检测
//        threshold(corner_img,Cdst, 0.015, 255, CV_THRESH_BINARY);
//        imshow("Cdst", Cdst);

        cout<<"========================================="<<count<<endl;
//        imshow("Point of Contours",Contours);   //向量contours内保存的所有轮廓点集
        imshow("SourceImage",img);

    }
    return 0;
}
