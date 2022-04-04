#include <iostream>
#include <OpenNI.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace openni;
using namespace cv;

Mat GetHistograph(const Mat grayImage)
{
	//定义求直方图的通道数目，从0开始索引
	int channels[] = { 0 };
	//定义直方图的在每一维上的大小，例如灰度图直方图的横坐标是图像的灰度值，就一维，bin的个数
	//如果直方图图像横坐标bin个数为x，纵坐标bin个数为y，则channels[]={1,2}其直方图应该为三维的，Z轴是每个bin上统计的数目
	const int histSize[] = { 256 };
	//每一维bin的变化范围
	float range[] = { 0,256 };

	//所有bin的变化范围，个数跟channels应该跟channels一致
	const float* ranges[] = { range };

	//定义直方图，这里求的是直方图数据
	Mat hist;
	//opencv中计算直方图的函数，hist大小为256*1，每行存储的统计的该行对应的灰度值的个数
	calcHist(&grayImage, 1, channels, Mat(), hist, 1, histSize, ranges, true, false);//cv中是cvCalcHist

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
		if (temp)
		{
			//由于图像坐标是以左上角为原点，所以要进行变换，使直方图图像以左下角为坐标原点
			histImage.col(i).rowRange(Range(rows - temp, rows)) = 255;
		}
	}
	//由于直方图图像列高可能很高，因此进行图像对列要进行对应的缩减，使直方图图像更直观
	Mat resizeImage;
	resize(histImage, resizeImage, Size(256, 256));
	return resizeImage;
}

Mat ConvertTo3Channels(const Mat& binImg)
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


void showdevice() {
	Array<DeviceInfo> aDeviceList;
	OpenNI::enumerateDevices(&aDeviceList);
	cout << "电脑上连接着 " << aDeviceList.getSize() << " 个设备." << endl;
	for (int i = 0; i < aDeviceList.getSize(); ++i)
	{
		cout << "设备 " << i << endl;
		const DeviceInfo& rDevInfo = aDeviceList[i];
		cout << "设备名： " << rDevInfo.getName() << endl;
		cout << "设备Id： " << rDevInfo.getUsbProductId() << endl;
		cout << "供应商名： " << rDevInfo.getVendor() << endl;
		cout << "供应商Id: " << rDevInfo.getUsbVendorId() << endl;
		cout << "设备URI: " << rDevInfo.getUri() << endl;
	}
}

void hMirrorTrans(const Mat& src, Mat& dst)
{
	dst.create(src.rows, src.cols, src.type());
	int rows = src.rows;
	int cols = src.cols;
	switch (src.channels())
	{
	case 1:   //1通道比如深度图像
		const uchar * origal;
		uchar* p;
		for (int i = 0; i < rows; i++) {
			origal = src.ptr<uchar>(i);
			p = dst.ptr<uchar>(i);
			for (int j = 0; j < cols; j++) {
				p[j] = origal[cols - 1 - j];
			}
		}
		break;
	case 3:
		const Vec3b * origal3;
		Vec3b* p3;
		for (int i = 0; i < rows; i++) {
			origal3 = src.ptr<Vec3b>(i);
			p3 = dst.ptr<Vec3b>(i);
			for (int j = 0; j < cols; j++) {
				p3[j] = origal3[cols - 1 - j];
			}
		}
		break;
	default:
		break;
	}

}

int main()
{
	//  VideoWriter video("Depth.avi", CV_FOURCC('M', 'J', 'P', 'G'), 30, Size(640,480));
	//  VideoWriter video1("RGB.avi", CV_FOURCC('M', 'J', 'P', 'G'), 30, Size(640,480));

	Status rc = STATUS_OK;
	OpenNI::initialize();// 初始化OpenNI环境
	showdevice();

	Device Orbbec;// 声明并打开Device设备。
	const char* deviceURL = openni::ANY_DEVICE;  //设备名
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

	namedWindow("Depth Image", CV_WINDOW_AUTOSIZE); // 创建OpenCV图像窗口

	int iMaxDepth = streamDepth.getMaxPixelValue(); // 获得最大深度值

	VideoFrameRef  frameDepth;  // 循环读取数据流信息并保存在VideoFrameRef中
	VideoCapture cap(2);
	string imgName;
	int index = 1;
	while (true)
	{
		Mat rgb;
		cap >> rgb;
		// 读取数据流
		rc = streamDepth.readFrame(&frameDepth);
		Mat mScaledDepth, hScaledDepth, c3hScaledDepth;
		if (rc == STATUS_OK)
		{
			// 将深度数据转换成OpenCV格式
			const Mat mImageDepth(frameDepth.getHeight(), frameDepth.getWidth(), CV_16UC1, (void*)frameDepth.getData());
			// 为了让深度图像显示的更加明显一些，将CV_16UC1 ==> CV_8U格式

			mImageDepth.convertTo(mScaledDepth, CV_8U, 255.0 / iMaxDepth);
			// 水平镜像深度图
			hMirrorTrans(mScaledDepth, hScaledDepth);
			// 显示出深度图像

			Mat Histograph = GetHistograph(hScaledDepth);

			imshow("Histograph", Histograph);

			c3hScaledDepth = ConvertTo3Channels(hScaledDepth);
			imshow("Depth Image", c3hScaledDepth);
		}
		imshow("RGB Image", rgb);
		//  video.write(c3hScaledDepth);
		//  video1.write(rgb);
		char c = (char)waitKey(1);
		if (c == 's' || c == 'S')
		{
			char str[1];
			sprintf(str, "%03d", index);
			imgName = string("RGB") + str + string(".jpg");
			imwrite(imgName, rgb);
			cout << "保存文件：" << imgName << endl;

			char str1[2];
			sprintf(str1, "%03d", index);
			imgName = string("Depth") + str + string(".jpg");
			imwrite(imgName, c3hScaledDepth);
			cout << "保存文件：" << imgName << endl;

			index++;

		}
		if (c == 27)// 终止快捷键
			break;
	}

	cap.release();
	// video.release();
	// video1.release();
	streamDepth.destroy();// 关闭数据流
	Orbbec.close(); // 关闭设备
	OpenNI::shutdown();// 最后关闭OpenNI

	return 0;
}
