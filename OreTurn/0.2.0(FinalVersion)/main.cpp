#include <iostream>
#include <opencv2/opencv.hpp>
#include "serial.h"
using namespace std;
union data_send_float
{
	float data_float;
	char data_uint8[4];
};

float getDistance(CvPoint pointO, CvPoint pointA)
{
	float distance;
	distance = powf((pointO.x - pointA.x), 2) + powf((pointO.y - pointA.y), 2);
	distance = sqrtf(distance);
	return distance;
}

double Entropy(cv::Mat img)
{
	double temp[256] = { 0.0 };

	//计算每个像素的累积值
	for (int m = 0; m < img.rows; m++)
	{
		const uchar* t = img.ptr<uchar>(m);  // 有效访问行列的方式
		for (int n = 0; n < img.cols; n++)
		{
			int i = t[n];
			temp[i] = temp[i] + 1;
		}
	}

	//计算每个像素的概率
	for (int i = 0; i < 256; i++)
	{
		temp[i] = temp[i] / (img.rows * img.cols);
	}

	double result = 0;
	//计算图像信息熵
	for (int i = 0; i < 256; i++)
	{
		if (temp[i] == 0.0)
			result = result;
		else
			result = result - temp[i] * (log(temp[i]) / log(2.0));
	}
	return result;

}

int main()
{
	cv::VideoCapture capture(0);
	cv::Mat img;

	if (!capture.isOpened())
	{
		std::cout << "Read video Failed !" << std::endl;
		return 0;
	}

	int fd2car = openPort("/dev/RMCWB");  //开串口
	char buff[10];
	bzero(buff, 10);

	int nRet = 0;
	cout << "fd2car:" << fd2car << endl;

	if (fd2car != -1)
	{
		configurePort(fd2car);
	}


	while (true)
	{
		capture >> img;
		if (img.empty())
		{
			cout << "Error: Could not load image" << endl;
			return 0;
		}

		char c = (char)cv::waitKey(1);
		if (c == 27)break;  //ESC 退出

		img = img(cv::Rect(220, 140, 300, 300));

		cv::Mat gray;
		cv::Mat dst;

		union data_send_float data_send[3];

		int AbleToSwitch = 0;
		int yaw = 0;
		int pitch = 0;
		int cnt = 0;

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
		//Mat imageContours=Mat::zeros(image.size(),CV_8UC1);
		cv::Mat Contours = cv::Mat::zeros(image.size(), CV_8UC1);  //绘制
		vector<vector<cv::Point>> hull(contours.size());  //用于存放凸包
		vector<float> length(contours.size());  //用于保存每个轮廓的长度
		vector<float> Area_contours(contours.size()), Area_hull(contours.size()), Rectangularity(contours.size()), circularity(contours.size());
		vector<vector<cv::Point2f>> DoubleRectangles;
		vector<vector<cv::Point2f>> Squares;

		float C;
		for (int i = 0; i < contours.size(); i++)
		{
			vector<cv::Point2f> TempPoint;
			int PointSize = 0;
			cv::RotatedRect Rect = minAreaRect(contours[i]);
			length[i] = arcLength(contours[i], true);  //轮廓的长度
			convexHull(cv::Mat(contours[i]), hull[i], false);  //把凸包找出来，寻找凸包函数
			Area_contours[i] = contourArea(contours[i]);   //轮廓面积
			Area_hull[i] = contourArea(hull[i]);  //凸包面积
			Rectangularity[i] = Area_contours[i] / Area_hull[i];  //矩形度
			circularity[i] = (4 * 3.1415 * Area_contours[i]) / (length[i] * length[i]);  //圆形度

			//if(contours[i].size()>50&&contours[i].size()<150)
			//abs(Rect.size.width/Rect.size.height)<1.2
			//if(abs(Rect.size.width/Rect.size.height)<1.1&&abs(Rect.size.width/Rect.size.height)>0.9&&contours[i].size()>60&&contours[i].size()<180)
			//if(abs(Rect.size.width/Rect.size.height)<1.2&&abs(Rect.size.width/Rect.size.height)>0.8&&contours[i].size()>60&&Rectangularity[i]>0.7)
			if (abs(Rect.size.width / Rect.size.height) < 1.3 && abs(Rect.size.width / Rect.size.height) > 0.7 && contours[i].size() > 80 && Rectangularity[i] > 0.6 && hierarchy[i][2] == -1)
			{
				cv::Point2f vertices[4];
				Rect.points(vertices);

				for (int j = 0; j < 4; j++)  //逐条边绘制
				{
					cv::line(Contours, vertices[j], vertices[(j + 1) % 4], cv::Scalar(255));
					cv::Point2f Temp = vertices[j];
					float MinDistance = 888;
					cv::Point2f MinPoint = cv::Point(-1, -1);
					for (int k = 0; k < contours[i].size(); k++)
					{
						float Distance;
						//cout<<contours[i][k].x<<" "<<contours[i][k].y<<endl;
						Distance = getDistance(Temp, contours[i][k]);
						if (Distance < MinDistance)
						{
							MinDistance = Distance;
							MinPoint = contours[i][k];
						}

					}
					if (MinDistance < (Rect.size.width + Rect.size.height) / 5)
					{
						circle(Contours, MinPoint, 2, cv::Scalar(255), 3, CV_AA, 0);  //中途显示
						//cv::Point2f P=Point(MinPoint.x,MinPoint.y);
						TempPoint.push_back(MinPoint);
						//cout<<TempPoint.size()<<endl;
						PointSize++;
					}

				}
				count++;
				//cout<<"Area_contours[i]:"<<Area_contours[i]<<endl;
				//cout<<"Area_hull[i]:"<<Area_hull[i]<<endl;
				//cout<<"Rectangularity[i]:"<<Rectangularity[i]<<endl;
				//cout<<"circularity[i]:"<<circularity[i]<<endl;

				for (int j = 0; j < contours[i].size(); j++)
				{
					//绘制出contours向量内所有的像素点
					cv::Point P = cv::Point(contours[i][j].x, contours[i][j].y);
					Contours.at<uchar>(P) = 255;
				}
				//imshow("Coage",temp); //轮廓
				//char a = (char)waitKey(0);
			}

			//cout<<TempPoint.size()<<endl;
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
					//cout<<"abs(a-b):"<<abs(a-b)<<"a:"<<a<<"b:"<<b<<endl;
					//cout<<PointSize<<endl;
				}
				//if(abs(((TempPoint[(MinPointNum + 1) % 3].y - TempPoint[MinPointNum].y)/(TempPoint[(MinPointNum + 1) % 3].x - TempPoint[MinPointNum].x)*(TempPoint[(MinPointNum + 2) % 3].y - TempPoint[MinPointNum].y)/(TempPoint[(MinPointNum + 2) % 3].x - TempPoint[MinPointNum].x))+1));
				//cout<<abs((TempPoint[(MinPointNum + 1) % 3].y - TempPoint[MinPointNum].y)/(TempPoint[(MinPointNum + 1) % 3].x - TempPoint[MinPointNum].x))<<endl;

				float a = getDistance(TempPoint[MinPointNum], TempPoint[(MinPointNum + 1) % 3]);
				float b = getDistance(TempPoint[MinPointNum], TempPoint[(MinPointNum + 2) % 3]);
				float c = getDistance(TempPoint[(MinPointNum + 1) % 3], TempPoint[(MinPointNum + 2) % 3]);

				if (abs(a * a + b * b - c * c) < c * c * 0.3)    //可调
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
					circle(img, Temp[0], 2, cv::Scalar(0, 0, 255), 3, CV_AA, 0);  //中途显示
					circle(img, Temp[1], 2, cv::Scalar(0, 255, 0), 3, CV_AA, 0);  //中途显示
					circle(img, Temp[2], 2, cv::Scalar(255, 0, 0), 3, CV_AA, 0);  //中途显示
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
					vector<float>TempDistances;
					TempDistances.push_back(a);
					TempDistances.push_back(b);
					TempDistances.push_back(c);
					sort(TempDistances.begin(), TempDistances.end());
					//cout << abs(TempDistances[0] * TempDistances[0] + TempDistances[1] * TempDistances[1] - TempDistances[2] * TempDistances[2]) << "::::" << (abs(TempDistances[0] * TempDistances[0] + TempDistances[1] * TempDistances[1] - TempDistances[2] * TempDistances[2])) / (TempDistances[2] * TempDistances[2]) << endl;
					if (abs(TempDistances[0] * TempDistances[0] + TempDistances[1] * TempDistances[1] - TempDistances[2] * TempDistances[2]) >
						TempDistances[2] * TempDistances[2] * 0.3)  //可调
					{
						SquareFlag = 0;
						break;
					}
				}
				if (SquareFlag)
				{
					circle(img, TempPoint[0], 2, cv::Scalar(0, 255, 255), 3, CV_AA, 0);  //中途显示
					circle(img, TempPoint[1], 2, cv::Scalar(0, 255, 255), 3, CV_AA, 0);  //中途显示
					circle(img, TempPoint[2], 2, cv::Scalar(0, 255, 255), 3, CV_AA, 0);  //中途显示
					circle(img, TempPoint[3], 2, cv::Scalar(0, 255, 255), 3, CV_AA, 0);  //中途显示
					vector<cv::Point2f> Temp;
					Temp.push_back(TempPoint[0]);
					Temp.push_back(TempPoint[1]);
					Temp.push_back(TempPoint[2]);
					Temp.push_back(TempPoint[3]);
					Squares.push_back(Temp);
				}

			}

		}    //for循环结束

		cv::Point2f KeyPoint1 = cv::Point(-1, -1);
		cv::Point2f KeyPoint2 = cv::Point(-1, -1);
		cv::Point2f MinPointA = cv::Point(-1, -1);
		cv::Point2f MinPointB = cv::Point(-1, -1);
		if (DoubleRectangles.size() > 2 && Squares.size() > 0)
		{
			float min = 888, A, B;
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

			if (abs(A * A + B * B - C * C) < C * C * 0.3)  //TempDistances
			{
				KeyPoint1 = MinPoint;
				//circle(img, KeyPoint1 , 3, cv::Scalar(106,68,20), 3, CV_AA, 0);  //中途显示
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
					//circle(img, MaxPoint , 5, cv::Scalar(0,0,0), 5, CV_AA, 0);  //中途显示
					float a = getDistance(MaxPoint, MinPointA);
					float b = getDistance(MaxPoint, MinPointB);
					//cout<<abs(a*a+b*b-C*C)<<endl;
					if (Verticalrate > abs(a * a + b * b - C * C))  //可调
					{
						Verticalrate = abs(a * a + b * b - C * C);
						BestPoint = MaxPoint;
					}
				}
				//cout<<Verticalrate<<endl;
				if (Verticalrate != 888888)
				{
					KeyPoint2 = BestPoint;
					//circle(img, KeyPoint2 , 3, cv::Scalar(106,68,20), 3, CV_AA, 0);  //中途显示
				}
			}

			float kkk = getDistance(KeyPoint1, KeyPoint2);
			//cout<< abs(kkk-C)<<endl;
			//cout<< KeyPoint1.x<<"   "<< KeyPoint2.x<<endl;
			if (KeyPoint1.x != -1 && KeyPoint2.x != -1 && abs(getDistance(KeyPoint1, KeyPoint2) - C) < 250)  //可调
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
					cout << "右下 0 -1" << endl;
					AbleToSwitch = 1;
					yaw = 0;
					pitch = -1;
				}

				if (KeyPoint2.x<TempCenter.x && KeyPoint2.y>TempCenter.y)
				{
					cout << "左下 -1 -1" << endl;
					AbleToSwitch = 1;
					yaw = -1;
					pitch = -1;
				}

				if (KeyPoint2.x < TempCenter.x && KeyPoint2.y < TempCenter.y)
				{
					cout << "左上 0 1" << endl;
					AbleToSwitch = 1;
					yaw = 0;
					pitch = 1;
				}

				if (KeyPoint2.x > TempCenter.x && KeyPoint2.y < TempCenter.y)
				{
					cout << "右上 1 -1" << endl;
					AbleToSwitch = 1;
					yaw = 1;
					pitch = -1;
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
									cout << "条码 0 2" << endl;
									AbleToSwitch = 1;
									yaw = 0;
									pitch = 2;
								}
								else
								{
									cout << "顶面 0 0" << endl;
									AbleToSwitch = 1;
									yaw = 0;
									pitch = 0;
								}
							}
						}
					}
				}
			}
		}

		//------------------------------------------------------------------
		data_send[0].data_float = yaw;  //数据装载
		data_send[1].data_float = pitch;  //数据装载
		data_send[2].data_float = 0;  //数据装载

		cout << "//" << data_send[0].data_float << endl;  //唯一cout
		cout << "//" << data_send[1].data_float << endl;
		cout << "//" << data_send[2].data_float << endl;

		sendXYZ(fd2car, data_send, AbleToSwitch, 2, cnt);  //发送数据
		cnt = (cnt + 1) % 255;  //累加
		//------------------------------------------------------------------

		imshow("Contours Image", Contours);  //轮廓
		imshow("SourceImage", img);
	}
	return 0;
}
