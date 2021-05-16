#include <header.h>

int main()
{
    //    VideoWriter video ("Depth.avi", CV_FOURCC('M', 'J', 'P', 'G'), 30, Size(640,480));
    //    VideoWriter video1("RGB.avi", CV_FOURCC('M', 'J', 'P', 'G'), 30, Size(640,480));

    Status rc = STATUS_OK;
    OpenNI::initialize(); // 初始化OpenNI环境
    showdevice();

    Device Orbbec;                              // 声明并打开Device设备。
    const char *deviceURL = openni::ANY_DEVICE; //设备名
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

    VideoFrameRef frameDepth; // 循环读取数据流信息并保存在VideoFrameRef中
    VideoCapture capRGB(2);   // 彩色摄像头 在工控机上可能要改

    Mat firstsrc, result;

    int move1 = 65, move2 = 42;       //台上抓取   //固定距离彩色摄像头与深度摄像头
    int cut_min = 244, cut_max = 297; //台上抓取   //ROI剪辑高度
    int Left = 239;                   //台上抓取
    int Right = 527;                  //台上抓取
    int PrehandleLow = 12;            //台上抓取
    int Prehandlehigh = 20;           //台上抓取
    int ReduceValue = 465;            //台上抓取
    int Findnext = 0;

    //    int move1=65,move2=42;                     //台下抓取   //固定距离彩色摄像头与深度摄像头
    //    int cut_min=351,cut_max=413;               //台下抓取   //ROI剪辑高度
    //    int Left = 239;                            //台下抓取
    //    int Right = 527;                           //台下抓取
    //    int PrehandleLow = 12;                     //台下抓取
    //    int Prehandlehigh = 20;                    //台下抓取
    //    int ReduceValue = 465;                     //台下抓取

    namedWindow("Parameter");
    createTrackbar("cut_min", "Parameter", &cut_min, 480, NULL);
    createTrackbar("cut_max", "Parameter", &cut_max, 480, NULL);
    createTrackbar("move1", "Parameter", &move1, 640, NULL);
    createTrackbar("move2", "Parameter", &move2, 480, NULL);
    createTrackbar("elementClosed.size.w", "Parameter", &s3, 50, NULL);
    createTrackbar("elementClosed.size.h", "Parameter", &s4, 20, NULL);
    createTrackbar("Left", "Parameter", &Left, 640, NULL);
    createTrackbar("Right", "Parameter", &Right, 640, NULL);
    createTrackbar("PrehandleLow", "Parameter", &PrehandleLow, 25, NULL);
    createTrackbar("Prehandlehigh", "Parameter", &Prehandlehigh, 25, NULL);
    createTrackbar("ReduceValue", "Parameter", &ReduceValue, 1000, NULL);
    //======================================================================
    int fd2car = openPort("/dev/RMCWB"); //开串口
    ControlReciever reciever;
    if (fd2car != -1)
    {
        configurePort(fd2car);
        reciever.set_fdcar(fd2car);
    }
    //======================================================================

    int mode = 0;
    int status = 0;


    while (true)
    {
        reciever.engineer_reciever();
        mode = reciever.mode;
        status=reciever.status;
        cout << "Receive:  " << mode <<"  " <<status<<endl;
    }


    union data_send_float data_send[3];
    float LeftDvalue = 0, RightDvalue = 0;
    int cnt = 0;

    while (true)
    {

        Mat DEPTH, gray;
        Mat Fusion, RGB;

        char c = (char)waitKey(1);
        if (c == 27)
            break;                //ESC 退出
        if (c == 32 || mode == 0) //SPACE 暂停
        {
//            while (true)
//            {
//                reciever.engineer_reciever();
//                mode = reciever.mode;
//                status = reciever.status;
////                cout << "Receive:  " << mode <<"  " <<status<<endl;
////                char c = (char)waitKey(1);
////                if (c == 'c')
////                    mode = 1; //C 继续/换模式
////                if (mode != 0)
////                    break; //C 继续
//            }
        }
        //        if(mode==1)    //台上抓取
        //        {
        //            move1=65,move2=42;                     //台上抓取   //固定距离彩色摄像头与深度摄像头 //
        //            cut_min=244,cut_max=297;               //台上抓取   //ROI剪辑高度
        //            Left = 239;                            //台上抓取
        //            Right = 527;                           //台上抓取
        //            PrehandleLow = 12;                     //台上抓取
        //            Prehandlehigh = 20;                    //台上抓取
        //            ReduceValue = 465;                     //台上抓取
        //            Findnext = 1;
        //        }
        //        if(mode==2)    //台下抓取
        //        {
        //            move1=65,move2=42;                     //台下抓取   //固定距离彩色摄像头与深度摄像头 //
        //            cut_min=351,cut_max=413;               //台下抓取   //ROI剪辑高度
        //            Left = 239;                            //台下抓取
        //            Right = 527;                           //台下抓取
        //            PrehandleLow = 12;                     //台下抓取
        //            Prehandlehigh = 20;                    //台下抓取
        //            ReduceValue = 465;                     //台下抓取
        //            Findnext = 0;
        //        }

        capRGB >> RGB;
        // 读取数据流
        rc = streamDepth.readFrame(&frameDepth);
        Mat mScaledDepth, hScaledDepth, c3hScaledDepth;
        // 将深度数据转换成OpenCV格式
        const Mat mImageDepth(frameDepth.getHeight(), frameDepth.getWidth(), CV_16UC1, (void *)frameDepth.getData());
        // 为了让深度图像显示的更加明显一些，将CV_16UC1 ==> CV_8U格式
        mImageDepth.convertTo(mScaledDepth, CV_8U, 255.0 / iMaxDepth);
        // 水平镜像深度图
        hMirrorTrans(mScaledDepth, hScaledDepth);
        // 显示出深度图像
        c3hScaledDepth = convertTo3Channels(hScaledDepth);

        //        imshow("Depth Image", c3hScaledDepth);
        //        imshow("RGB Image",rgb);

        DEPTH = c3hScaledDepth.clone();

        Mat RGBtransform;
        translateTransform(RGB, RGBtransform, move1 - 50, move2 - 50);
        //        Fusion = 0.3*RGBtransform +0.5*DEPTH ;      //RGB图和深度图混合 观看
        //        Fusion = RGBtransform ;                   //纯彩色图         观看
        Fusion = DEPTH; //纯深度图         观看
        if (DEPTH.empty())
            break;
        Mat ROI(DEPTH, Rect(0, cut_min, 640, cut_max - cut_min)); //切割ROI
        cvtColor(ROI, gray, CV_BGR2GRAY);
        //        imshow("ROI",ROI);
        result = gray.clone(); //result 为第一排图像

        Mat CalcROI = Mat(gray, Rect(Left, 0, Right - Left, gray.rows));

        cv::threshold(CalcROI, CalcROI, PrehandleLow, 255, THRESH_TOZERO);
        cv::threshold(CalcROI, CalcROI, Prehandlehigh, 255, THRESH_TOZERO_INV);

        Mat Calc2ROI = CalcROI.clone();
        Mat ROIdetect = CalcROI.clone();

        inRange(CalcROI, PrehandleLow, Prehandlehigh, Calc2ROI);

        //        if(mode == 1)
        //            inRange(CalcROI, PrehandleLow, Prehandlehigh, Calc2ROI);
        //        if(mode == 2)
        //            inRange(CalcROI, PrehandleLow, Prehandlehigh, Calc2ROI);

        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        findContours(Calc2ROI, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        vector<RotatedRect> boundRect(contours.size());

        //        imshow("MainSRC",ROIdetect);
        //        cout<<"contours.size():"<<contours.size()<<endl;

        Point2f vertices[4]; //定义矩形的4个顶点
        int max = 0, max_number = 0, Flag = 0;
        if (contours.size() > 0)
        {
            for (unsigned int i = 0; i < contours.size(); i++)
            {
                boundRect[i] = minAreaRect(Mat(contours[i]));
                if (boundRect[i].size.width * boundRect[i].size.height > max)
                {
                    max = boundRect[i].size.width * boundRect[i].size.height;
                    max_number = i;
                }
            }
            if (max != 0 && boundRect[max_number].size.width * boundRect[max_number].size.height > 100)
            {
                boundRect[max_number].points(vertices); //计算矩形的4个顶点  左下0左上1右上2右下3
                Flag = 1;
            }
        }

        int AbleToGrab = 1;
        if (Flag == 1)
        {
            for (int i = 0; i < 4; i++)
                line(CalcROI, vertices[i], vertices[(i + 1) % 4], Scalar(255), 1);
            //        cout<<(vertices[2].y-vertices[1].y)/(vertices[2].x-vertices[1].x)<<endl;
            if ((vertices[2].y - vertices[1].y) / (vertices[2].x - vertices[1].x) < -5)
            {
                Point2f temp = vertices[0];
                vertices[0] = vertices[1];
                vertices[1] = vertices[2];
                vertices[2] = vertices[3];
                vertices[3] = temp;
            }

            Point2f Detect[2];
            Detect[0].x = (vertices[0].x + vertices[1].x) / 2;
            Detect[0].y = (vertices[0].y + vertices[1].y) / 2;
            Detect[1].x = (vertices[3].x + vertices[2].x) / 2;
            Detect[1].y = (vertices[3].y + vertices[2].y) / 2;

            Point2f Detect1[2];
            Detect1[0].x = (4 / 5.0 * vertices[0].x + 1 / 5.0 * vertices[1].x);
            Detect1[0].y = (4 / 5.0 * vertices[0].y + 1 / 5.0 * vertices[1].y);
            Detect1[1].x = (4 / 5.0 * vertices[3].x + 1 / 5.0 * vertices[2].x);
            Detect1[1].y = (4 / 5.0 * vertices[3].y + 1 / 5.0 * vertices[2].y);

            Point2f Detect2[2];
            Detect2[0].x = (2 / 3.0 * vertices[0].x + 1 / 3.0 * vertices[1].x);
            Detect2[0].y = (2 / 3.0 * vertices[0].y + 1 / 3.0 * vertices[1].y);
            Detect2[1].x = (2 / 3.0 * vertices[3].x + 1 / 3.0 * vertices[2].x);
            Detect2[1].y = (2 / 3.0 * vertices[3].y + 1 / 3.0 * vertices[2].y);

            //        cout<<vertices[0]<<vertices[1]<<vertices[2]<<vertices[3]<<endl;

            circle(CalcROI, Detect[0], 3, Scalar(255), 3);
            circle(CalcROI, Detect[1], 3, Scalar(255), 3);
            circle(CalcROI, Detect1[0], 3, Scalar(255), 3);
            circle(CalcROI, Detect1[1], 3, Scalar(255), 3);
            circle(CalcROI, Detect2[0], 3, Scalar(255), 3);
            circle(CalcROI, Detect2[1], 3, Scalar(255), 3);
            int time = 30, Threshold = 850;

            int lcount = 0, rcount = 0, lcount1 = 0, rcount1 = 0, lcount2 = 0, rcount2 = 0, lvalueSum = 0, rvalueSum = 0;
            float k1 = (vertices[2].y - vertices[1].y) / (vertices[2].x - vertices[1].x);
            float k2 = (vertices[3].y - vertices[0].y) / (vertices[3].x - vertices[0].x);

            //        imshow("ROIdetect",ROIdetect);

            while (lcount <= time || rcount <= time || lcount1 <= time || rcount1 <= time || lcount2 <= time || rcount2 <= time)
            {
                //cout<<lcount<<":"<<rcount<<":"<<lcount1<<":"<<rcount1<<":"<<lcount2<<":"<<rcount2<<endl;
                int lvalue = (int)ROIdetect.at<uchar>(Detect[0].y, Detect[0].x);
                int rvalue = (int)ROIdetect.at<uchar>(Detect[1].y, Detect[1].x);

                int lvalue1 = (int)ROIdetect.at<uchar>(Detect1[0].y, Detect1[0].x);
                int rvalue1 = (int)ROIdetect.at<uchar>(Detect1[1].y, Detect1[1].x);

                int lvalue2 = (int)ROIdetect.at<uchar>(Detect2[0].y, Detect2[0].x);
                int rvalue2 = (int)ROIdetect.at<uchar>(Detect2[1].y, Detect2[1].x);

                if (lcount <= time)
                {
                    if (lvalue != 0)
                    {
                        lvalueSum += lvalue;
                        lcount++;
                        circle(CalcROI, Detect[0], 1, Scalar(255), 1);
                    }
                    Detect[0].x += 1;
                    Detect[0].y = Detect[0].y + k1 * 1;
                }
                if (lcount1 <= time)
                {
                    if (lvalue1 != 0)
                    {
                        lvalueSum += lvalue1;
                        lcount1++;
                        circle(CalcROI, Detect1[0], 1, Scalar(255), 1);
                    }
                    Detect1[0].x += 1;
                    Detect1[0].y = Detect1[0].y + k1 * 1;
                }
                if (lcount2 <= time)
                {
                    if (lvalue2 != 0)
                    {
                        lvalueSum += lvalue2;
                        lcount2++;
                        circle(CalcROI, Detect2[0], 1, Scalar(255), 1);
                    }
                    Detect2[0].x += 1;
                    Detect2[0].y = Detect2[0].y + k1 * 1;
                }
                if (rcount <= time)
                {
                    if (rvalue != 0)
                    {
                        rvalueSum += rvalue;
                        rcount++;
                        circle(CalcROI, Detect[1], 1, Scalar(255), 1);
                    }
                    Detect[1].x -= 1;
                    Detect[1].y = Detect[1].y - k2 * 1;
                }

                if (rcount1 <= time)
                {
                    if (rvalue1 != 0)
                    {
                        rvalueSum += rvalue1;
                        rcount1++;
                        //                cout<<lcount<<"--"<<lvalue<<endl;
                        circle(CalcROI, Detect1[1], 1, Scalar(255), 1);
                    }
                    Detect1[1].x -= 1;
                    Detect1[1].y = Detect1[1].y - k2 * 1;
                }
                if (rcount2 <= time)
                {
                    if (rvalue2 != 0)
                    {
                        rvalueSum += rvalue2;
                        rcount2++;
                        circle(CalcROI, Detect2[1], 1, Scalar(255), 1);
                    }
                    Detect2[1].x -= 1;
                    Detect2[1].y = Detect2[1].y - k2 * 1;
                }
            }

            LeftDvalue = lvalueSum;
            RightDvalue = rvalueSum;

            cout << "lvalueSum:" << lvalueSum << "rvalueSum:" << rvalueSum << endl;
            cout << "lvalueSum-rvalueSum :" << lvalueSum - rvalueSum << endl;

            if (lvalueSum - rvalueSum > 15)
            {
                arrowedLine(Fusion, Point(50, 160), Point(50, 120), Scalar(0, 255, 0), 3, 8, 0, 0.4);
                AbleToGrab = 0;
            }
            else if (lvalueSum - rvalueSum < -15)
            {
                arrowedLine(Fusion, Point(590, 160), Point(590, 120), Scalar(0, 255, 0), 3, 8, 0, 0.4);
                AbleToGrab = 0;
            }
            else if (lvalueSum > Threshold && rvalueSum > Threshold)
            {
                //                    cout<<lvalueSum>Threshold<<":"<<rvalueSum>Threshold<<endl;
                arrowedLine(Fusion, Point(50, 160), Point(50, 120), Scalar(0, 255, 0), 3, 8, 0, 0.4);
                arrowedLine(Fusion, Point(590, 160), Point(590, 120), Scalar(0, 255, 0), 3, 8, 0, 0.4);
                AbleToGrab = 0;
            }

            imshow("ProcessedImage", CalcROI);
        }

        firstsrc = prehandle(result, PrehandleLow, Prehandlehigh); //第一排的阈值
        imshow("firstsrc", firstsrc);
        float FinalResult;

        int aim = getAim(firstsrc, Left, Right, Findnext); //主函数

        if (aim < 250 && aim > -250)
        {
            line(Fusion, Point(aim + (Right - Left) / 2 + Left, 0), Point(aim + (Right - Left) / 2 + Left, 480), Scalar(0, 255, 0), 1);
            FinalResult = aim;
        }
        else if (aim == 250)
        {
            FinalResult = 256;
        }
        else if (aim == -250)
        {
            FinalResult = -10;
        }
        else if (aim == 888) //当前无目标
        {
            FinalResult = 888;
        }

        if (FinalResult < 5 && FinalResult > -5)
        {
            if (AbleToGrab == 1)
                cv::putText(Fusion, "$GRAB$", Point(220, 100), cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(0, 255, 255), 4);
            else
                cv::putText(Fusion, "MOVE", Point(250, 100), cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(0, 255, 0), 2);
        }
        else if (FinalResult > 0 && FinalResult < 800)
        {
            arrowedLine(Fusion, Point(520, 100), Point(600, 100), Scalar(0, 0, 255), 4, 8, 0, 0.4);
            cv::putText(Fusion, "MOVE", Point(250, 100), cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(0, 255, 0), 2);
        }
        else if (FinalResult < 0)
        {
            arrowedLine(Fusion, Point(120, 100), Point(40, 100), Scalar(0, 0, 255), 4, 8, 0, 0.4);
            cv::putText(Fusion, "MOVE", Point(250, 100), cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(0, 255, 0), 2);
        }
        else if (FinalResult == 888)
        {
            FinalResult = 0;
            AbleToGrab = 0;
            cv::putText(Fusion, "NO TARGET", Point(250, 100), cv::FONT_HERSHEY_COMPLEX, 2, cv::Scalar(0, 255, 0), 2);
        }
        imshow("Fusion", Fusion);

        //        video.write(c3hScaledDepth);
        //        video1.write(rgb);

        //------------------------------------------------------------------
        data_send[0].data_float = FinalResult;               //数据装载
        data_send[1].data_float = LeftDvalue - ReduceValue;  //数据装载
        data_send[2].data_float = RightDvalue - ReduceValue; //数据装载

        cout << "//" << data_send[0].data_float << endl; //唯一COUT
        cout << "//" << data_send[1].data_float << endl;
        cout << "//" << data_send[2].data_float << endl;

        sendXYZ(fd2car, data_send, AbleToGrab, 1, cnt); //发送数据
        cnt = (cnt + 1) % 255;                          //累加
        //------------------------------------------------------------------
    }

    //    video.release();
    //    video1.release();
    capRGB.release();
    streamDepth.destroy(); // 关闭数据流
    Orbbec.close();        // 关闭设备
    OpenNI::shutdown();    // 最后关闭OpenNI

    return 0;
}
