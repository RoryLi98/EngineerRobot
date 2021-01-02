#include <header.h>

int main()
{
//    VideoWriter video("Depth.avi", CV_FOURCC('M', 'J', 'P', 'G'), 30, Size(640,480));
//    VideoWriter video1("RGB.avi", CV_FOURCC('M', 'J', 'P', 'G'), 30, Size(640,480));

    Status rc = STATUS_OK;
    OpenNI::initialize();// 初始化OpenNI环境
    showdevice();

    Device Orbbec;// 声明并打开Device设备。
    const char * deviceURL = openni::ANY_DEVICE;  //设备名
    rc = Orbbec.open(deviceURL);

    // 创建深度数据流
    VideoStream streamDepth;
    rc = streamDepth.create(Orbbec, SENSOR_DEPTH);
    if (rc == STATUS_OK)
    {
        // 设置深度图像视频模式
        VideoMode mModeDepth;
        // 分辨率大小
        mModeDepth.setResolution(640, 480);
        // 每秒30帧
        mModeDepth.setFps(30);
        // 像素格式
        mModeDepth.setPixelFormat(PIXEL_FORMAT_DEPTH_1_MM);
        streamDepth.setVideoMode(mModeDepth);
        // 打开深度数据流
        rc = streamDepth.start();
        if (rc != STATUS_OK)
        {
            cerr << "无法打开深度数据流：" << OpenNI::getExtendedError() << endl;
            streamDepth.destroy();
        }
    }
    else
    {
        cerr << "无法创建深度数据流：" << OpenNI::getExtendedError() << endl;
    }

//    namedWindow("Depth Image", CV_WINDOW_AUTOSIZE); // 创建OpenCV图像窗口

    int iMaxDepth = streamDepth.getMaxPixelValue(); // 获得最大深度值

    VideoFrameRef frameDepth;                       // 循环读取数据流信息并保存在VideoFrameRef中
    VideoCapture capRGB(2);                         // 彩色摄像头 *******在工控机上可能要改

    int move1=65,move2=42;                          //固定距离彩色摄像头与深度摄像头
    int cut_min=235,cut_max=335;                    //ROI剪辑高度
    Target target;
    Mat firstsrc,secondsrc,result1,result;

    //////////////////////////////////////////
    namedWindow("Height");                                  //调节第一二排的剪辑高度
    createTrackbar("Height_min","Height",&cut_min,480,NULL);
    createTrackbar("Height_max","Height",&cut_max,480,NULL);
    namedWindow("MOVE");                                    //用来调节彩色图和深度图混合的偏移，已经调好了
    createTrackbar("MOVE1","MOVE",&move1,640,NULL);
    createTrackbar("MOVE2","MOVE",&move2,480,NULL);
    namedWindow("elementClosed");                           //调闭运算的算子，之后再优化
    createTrackbar("Height_min","elementClosed",&s3,20,NULL);
    createTrackbar("Height_max","elementClosed",&s4,20,NULL);
    ///////////////////////////////////////////

    vector<int> list;                                       //用来装偏移数据，只用于DeletedFlag

    int fd2car=openPort("/dev/RMCWB");  //开串口
    cout<<"fd2car:"<<fd2car<<endl;
    if(fd2car!=-1)
    {
        configurePort(fd2car);
    }
    union data_send_float data_send;
    int cnt=0;

    while (true)
    {

        Mat DEPTH,gray;
        Mat Fusion,RGB;

        char c = (char)waitKey(1);
        if(c == 27 )break;            //ESC 退出
        if(c == 32 )                  //SPACE 暂停
        {
            while(true)
            {
                capRGB >> RGB;
                rc = streamDepth.readFrame(&frameDepth);
                cout<<"Pausing"<<endl;
                char c = (char)waitKey(1);
                if(c == 99 )break;    //C 继续
            }
        }

        capRGB >> RGB;
        // 读取数据流
        rc = streamDepth.readFrame(&frameDepth);
        Mat mScaledDepth, hScaledDepth,c3hScaledDepth;
        // 将深度数据转换成OpenCV格式
        const Mat mImageDepth(frameDepth.getHeight(), frameDepth.getWidth(), CV_16UC1, (void*)frameDepth.getData());
        // 为了让深度图像显示的更加明显一些，将CV_16UC1 ==> CV_8U格式
        mImageDepth.convertTo(mScaledDepth, CV_8U, 255.0 / iMaxDepth);
        // 水平镜像深度图
        hMirrorTrans(mScaledDepth, hScaledDepth);
        // 显示出深度图像
        c3hScaledDepth = convertTo3Channels(hScaledDepth);

//        imshow("Depth Image", c3hScaledDepth);
//        imshow("RGB Image",rgb);

        DEPTH = c3hScaledDepth.clone();

//        int t1 = getTickCount();                  //getTickCount函数从0开始计时，返回自设备启后的毫秒数（不含系统暂停时间）

        Mat RGBtransform;
        translateTransform(RGB,RGBtransform,move1-50,move2-50);
        Fusion = 0.3*RGBtransform +0.5*DEPTH ;      //RGB图和深度图混合 观看
//        Fusion = RGBtransform ;                   //纯彩色图         观看
//        Fusion = DEPTH;                           //纯深度图         观看
        if (DEPTH.empty())break;
        Mat ROI(DEPTH,Rect(0,cut_min,640,cut_max-cut_min));  //切割ROI
        cvtColor(ROI,gray,CV_BGR2GRAY);
//        imshow("ROI",ROI);
        result = gray.clone();                      //result 为第一排图像
        result1 = gray.clone();                     //result1 为第二排图像

        Mat CalcROI=Mat(gray,Rect(212,0,471-212,gray.rows));

        cv::threshold(CalcROI, CalcROI, 14, 255, THRESH_TOZERO);
        cv::threshold(CalcROI, CalcROI, 20, 255, THRESH_TOZERO_INV);

//        Mat CalcHistograph = getHistograph(CalcROI);
//        imshow("CalcHistograph",CalcHistograph);

        Mat Calc2ROI = CalcROI.clone();
        Mat ROIdetect = CalcROI.clone();
        inRange(CalcROI, 14, 20, Calc2ROI);

        vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;
        findContours(Calc2ROI, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE );

        vector<RotatedRect>boundRect(contours.size());

    //    imshow("MainSRC",maintarget);
    //    cout<<"contours.size():"<<contours.size()<<endl;

        Point2f vertices[4];      //定义矩形的4个顶点
        int max=0,max_number=0,Flag=0;
        if(contours.size()>0)
        {
              for(unsigned int i = 0; i < contours.size(); i++ )
               {
                boundRect[i] = minAreaRect(Mat(contours[i]));
                if(boundRect[i].size.width *boundRect[i].size.height>max)
                     {
                         max = boundRect[i].size.width * boundRect[i].size.height;
                         max_number = i;
                     }
               }
              if(max!=0&&boundRect[max_number].size.width *boundRect[max_number].size.height>100)
              {
               boundRect[max_number].points(vertices);   //计算矩形的4个顶点  左下0左上1右上2右下3
               Flag =  1;
              }
        }

        int AbleToGrab = 1;
        if(Flag==1)
        {
                for (int i = 0; i < 4; i++)
                    line(CalcROI, vertices[i], vertices[(i + 1) % 4], Scalar(255),1);

        //        cout<<(vertices[2].y-vertices[1].y)/(vertices[2].x-vertices[1].x)<<endl;
                if((vertices[2].y-vertices[1].y)/(vertices[2].x-vertices[1].x)<-5)
                {
                    Point2f temp = vertices[0];
                    vertices[0] = vertices[1];
                    vertices[1] = vertices[2];
                    vertices[2] = vertices[3];
                    vertices[3] = temp;
                }

                Point2f Detect[2];
                Detect[0].x = (vertices[0].x + vertices[1].x)/2;
                Detect[0].y = (vertices[0].y + vertices[1].y)/2;
                Detect[1].x = (vertices[3].x + vertices[2].x)/2;
                Detect[1].y = (vertices[3].y + vertices[2].y)/2;
        //        cout<<vertices[0]<<vertices[1]<<vertices[2]<<vertices[3]<<endl;

                circle(CalcROI,Detect[0], 5, Scalar(255), 3);
                circle(CalcROI,Detect[1], 5, Scalar(255), 3);

                int time=30,Threshold=470;
                int lcount = 0,rcount = 0,lvalueSum = 0,rvalueSum = 0;
                float k1 = (vertices[2].y-vertices[1].y)/(vertices[2].x-vertices[1].x);
                float k2 = (vertices[3].y-vertices[0].y)/(vertices[3].x-vertices[0].x);

        //        imshow("ROIdetect",ROIdetect);

                while(lcount<=time||rcount<=time)
                {
                    int lvalue = (int)ROIdetect.at<uchar>(Detect[0].y, Detect[0].x);
                    int rvalue = (int)ROIdetect.at<uchar>(Detect[1].y, Detect[1].x);
        //            cout<<lvalue<<":"<<rvalue<<endl;
                    if(lvalue!=0&&lcount<=time)
                    {
                        lvalueSum += lvalue;
                        lcount++;
        //                cout<<lcount<<"--"<<lvalue<<endl;
                        circle(CalcROI,Detect[0], 1, Scalar(255), 1);
                    }
                    Detect[0].x += 1;
                    Detect[0].y = Detect[0].y+k1*1;
                    if(rvalue!=0&&rcount<=time)
                    {
                        rvalueSum += rvalue;
                        rcount++;
        //                cout<<rcount<<"--"<<rvalue<<endl;
                        circle(CalcROI,Detect[1], 1, Scalar(255), 1);
                    }
                    Detect[1].x -= 1;
                    Detect[1].y = Detect[1].y-k2*1;
                }

                cout<<"lvalueSum:"<<lvalueSum<<"rvalueSum:"<<rvalueSum<<endl;;
                cout<<lvalueSum - rvalueSum<<endl;

                if(lvalueSum - rvalueSum>5)
                {
                    arrowedLine(Fusion,Point(50,160),Point(50,120),Scalar(0,255,0),3,8,0,0.4);
                    AbleToGrab = 0;
                }
                else if(lvalueSum - rvalueSum<-5)
                {
                    arrowedLine(Fusion,Point(590,160),Point(590,120),Scalar(0,255,0),3,8,0,0.4);
                    AbleToGrab = 0;
                }
                else if(lvalueSum>Threshold&&rvalueSum>Threshold)
                {
                    arrowedLine(Fusion,Point(50,160),Point(50,120),Scalar(0,255,0),3,8,0,0.4);
                    arrowedLine(Fusion,Point(590,160),Point(590,120),Scalar(0,255,0),3,8,0,0.4);
                    AbleToGrab = 0;
                }

                imshow("ProcessedImage",CalcROI);
        }

        firstsrc = prehandle(result,14,20);         //第一排的阈值
        secondsrc = prehandle(result1,19,27);       //第二排的阈值
//        imshow("第一排",firstsrc);
//        imshow("第二排",secondsrc);
        vector<float>result(2);

        int aim = getAim(firstsrc,secondsrc);       //主函数

//       cout<<"out aim:"<<aim<<endl;
        if(aim>2000)
        {
            line(Fusion, Point(aim-2500+345, 0), Point(aim-2500+345, 480), Scalar( 0, 255,0), 1);
            result[0]=2;
            result[1]=aim%1000-500;
            list.push_back(result[1]);
        }
        else if(aim>1000)
        {
            line(Fusion, Point(aim-1500+345, 0), Point(aim-1500+345, 480), Scalar( 0, 0,255), 1);
            result[0]=1;
            result[1]=aim%1000-500;
            list.push_back(result[1]);
        }
        else if(aim==250)
        {
            result[1]=250;
            list.clear();
        }
        else if(aim==-250)
        {
            result[1]=-250;
            list.clear();
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

        if(result[1]<7&&result[1]>-7)
        {
            if(AbleToGrab==1)
            cv::putText(Fusion, "$GRAB$", Point(220,100), cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(0, 255, 255), 4);
            else cv::putText(Fusion, "MOVE", Point(250,100), cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(0, 255, 0), 2);
        }
        else if(result[1]>0&&result[1]<800)
        {
            arrowedLine(Fusion,Point(520,100),Point(600,100),Scalar(0,0,255),4,8,0,0.4);
            cv::putText(Fusion, "MOVE", Point(250,100), cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(0, 255, 0), 2);
        }
        else if(result[1]<0)
        {
            arrowedLine(Fusion,Point(120,100),Point(40,100),Scalar(0,0,255),4,8,0,0.4);
            cv::putText(Fusion, "MOVE", Point(250,100), cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(0, 255, 0), 2);
        }

//        cout<<"---------------"<<endl;
//        cout<<"|4="<<target.position4<<" 5 ="<<target.position5<<" 6="<<target.position6<<"|"<<endl;
//        cout<<"|1="<<target.position1<<"aim="<<result[1]<<" 3="<<target.position3<<"|"<<endl;
//        cout<<"leftflag="<<target.leftflag<<"  rightflag="<<target.rightflag<<endl;
//        cout<<"---------------"<<endl;
//        这些是看位置标志位的

        if(result[1]<10&&result[1]>-10)
        {
            getAllInformation(firstsrc,secondsrc,target,218,496);
            target.refreshDirection();
            deleteFlag(target,list);
        }

        imshow("Fusion",Fusion);

//        int t2 = getTickCount();
//        cout << "time:"<<(t2 - t1) * 1000.0 / getTickFrequency()<<"ms"<<endl;

//        video.write(c3hScaledDepth);
//        video1.write(rgb);

//------------------------------------------------------------------
        data_send.data_float = result[1];     //数据装载
//        cout<<data_send.data_float<<endl;
        cout<<result[1]<<endl;                //唯一COUT

//        for(int i =0 ;i<4;i++)
//        {
//            cout<<(char)data_send[0].data_uint8[i]<<" "<<endl;
//        }

        sendXYZ(fd2car,data_send,1,cnt);      //发送数据
        cnt=(cnt+1)%255;                      //累加
//------------------------------------------------------------------
    }

    capRGB.release();

//    video.release();
//    video1.release();

    streamDepth.destroy();// 关闭数据流
    Orbbec.close(); // 关闭设备
    OpenNI::shutdown();// 最后关闭OpenNI

    return 0;
}
