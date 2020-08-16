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
#define MINWIDTH 70
#define MAXWIDTH 90
#define MINHEIGHT 80
#define MAXHEIGHT 110
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

bool judge_direction(vector<int>list)
{
    int num = 0;
    for(int i=0;i<20;i++)
        num+=list[i];
    if(num>0) return 1;
    else return 0;
}


int getMainTarget(Mat src,int a,int b)
{
    int Offset = 888;
    const int rows = src.rows;
    Mat maintarget = Mat(src,Rect(a,0,b-a,rows));

    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours(maintarget, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE );

    vector<Rect>boundRect(contours.size());

    int max=0,max_number=0;
    if(contours.size()>0)
    {
          for(unsigned int i = 0; i < contours.size(); i++ )
           {
              boundRect[i] = boundingRect( Mat(contours[i]) );
            if((boundRect[i].width>max)&&(boundRect[i].height>MINHEIGHT))
           // if(boundRect[i].width>max)
                  {
                     max = boundRect[i].width;
                     max_number = i;
                  }
           }
        if(max!=0)
        {
        int TargetCenterX = boundRect[max_number].x + (boundRect[max_number].width)/2 ;
//        cout<<"WIDTH:"<<boundRect[max_number].width<<endl;
//        cout<<"HEIGHT:"<<boundRect[max_number].height<<endl;
        Offset = TargetCenterX - (b-a)/2;
        }

    }
    return Offset;
}



class Target
{
public:
    int position1= 0;
    int position2= 0;
    int position3= 0;
    int position4= 0;
    int position5= 0;
    int position6= 0;
    int leftflag = 0 ;
    int rightflag = 0 ;

    void refreshDirection()
    {
       if(position1==1||position4==1)leftflag = 1;
       if(position3==1||position6==1)rightflag = 1;

    }
    int getDirection()
    {
        if(leftflag||rightflag==1)
        {
            if(leftflag==1&&rightflag==1)return 2;
            else if(rightflag==1)return 2;
            else if(leftflag==1)return 1;
        }
        return 0;
    }

};

int ifbox(Mat src)          //左右有没有箱子
{
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours(src, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE );
    vector<Rect>boundRect(contours.size());
    if(contours.size()>0)
    {
          for(unsigned int i = 0; i < contours.size(); i++ )
           {
              boundRect[i] = boundingRect( Mat(contours[i]) );
//              if((boundRect[i].width>MINWIDTH)&&(boundRect[i].width>MAXWIDTH)&&(boundRect[i].height>MINHEIGHT)&&(boundRect[i].height>MAXHEIGHT))
                if(boundRect[i].width>70)
                  {
                     return 1;
                  }
           }
    }
    return 0;
}


void getAllInformation(Mat firstsrc,Mat secondsrc,Target& target,int a,int b)
{
    const int rows = firstsrc.rows;

    Mat firstleft = Mat(firstsrc,Rect(0,0,a,rows));
    Mat firstright = Mat(firstsrc,Rect(b,0,640-b,rows));
    Mat secondleft = Mat(secondsrc,Rect(0,0,a,rows));
//    Mat secondmid = Mat(secondsrc,Rect(a,0,b-a,rows));
    Mat secondright = Mat(secondsrc,Rect(b,0,640-b,rows));

    target.position1 = ifbox(firstleft);
    target.position3 = ifbox(firstright);

    target.position4 = ifbox(secondleft);
    target.position6 = ifbox(secondright);


}
void deleteFlag(Target &target,vector<int>& list)
{
    if(list.empty()==0)
    {
        int result = 0;
        int left = 0;
        int right = 0;
        vector<int>::iterator it;
        for (it = list.begin(); it != list.end(); it++)
        {
              if(*it<0)left++;
              if(*it>0)right++;
              if(left>20)
              {
                    result = 1;
                    break;
              }
              if(right>20)
              {
                    result = 2;
                    break;
              }
        }
        if(result==1)target.leftflag = 0;
        if(result==2)target.rightflag = 0;
        cout<<result <<endl;
        list.clear();
    }
}


int getAim(Mat firstsrc,Mat secondsrc)
{
    const int rows = firstsrc.rows;

    int aim = getMainTarget(firstsrc,120,520);
    if(aim == 888)
    {

        if(ifbox(Mat(firstsrc,Rect(520,0,120,rows))))return 250;
        if(ifbox(Mat(firstsrc,Rect(0,0,120,rows))))return (-250);
        aim = 999;
        aim = getMainTarget(secondsrc,120,520);
        if(aim == 888)
        {
            if(ifbox(Mat(secondsrc,Rect(520,0,120,rows))))return 250;
            if(ifbox(Mat(secondsrc,Rect(0,0,120,rows))))return -250;
            return 888;
        }
        return 2500+aim;
//        return aim;
    }
    return 1500+aim;
//    return aim;

}







//}
#endif // HEADER_H
