#include <opencv2\opencv.hpp>
#include <iostream>
using namespace std;
using namespace cv;

vector<int> Median(vector<Vec4i>);
Point slot(Point, Point, Point, Point);

int main()
{
	Mat src = imread("11.jpg");
	Mat dst ;
	src.copyTo(dst);
	cvtColor(dst, dst, CV_BGR2GRAY);
	medianBlur(dst, dst, 5);//使用中值滤波器,保护边缘信息
	imshow("blur", dst);

	Canny(dst, dst, 150, 250, 3);
	imshow("canny", dst);

	int colNumber = dst.cols * dst.channels();
	for (int i = 0; i < dst.rows; i++)
	{
		uchar* data = dst.ptr<uchar>(i);
		for (int j = 0; j < colNumber; j++)
		{
			if (i >= dst.rows / 2)
			{
				if (j >= ((dst.rows - i)*(dst.cols / 2) / dst.rows) * 3 
					&&j <= colNumber + 100 - ((dst.rows - i)*(dst.cols / 2) / dst.rows) * 3)
				{
					continue;
				}
			}
			data[j] = 0;
		}
	}//兴趣区域OK
	imshow("roi", dst);

	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));		
	dilate(dst, dst, element);
	imshow("dilate", dst);//加重边缘

	/*vector<Vec2f> lines;
	HoughLines(dst, lines, 1, CV_PI / 180, 100, 100, 50);
	for (size_t i = 0; i < lines.size(); i++)//将求得的线条画出来
	{
		float rho = lines[i][0], theta = lines[i][1];
		Point pt1, pt2;
		double a = cos(theta), b = sin(theta);
		double x0 = a*rho, y0 = b*rho;
		pt1.x = cvRound(x0 + 1000 * (-b));
		pt1.y = cvRound(y0 + 1000 * (a));
		pt2.x = cvRound(x0 - 1000 * (-b));
		pt2.y = cvRound(y0 - 1000 * (a));
		line(src, pt1, pt2, Scalar(0, 0, 255), 3, CV_AA);
	}*/

	vector<Vec4i> possible_Lines;
	HoughLinesP(dst, possible_Lines, 1, CV_PI / 180, 150, 100, 50);
	cout << possible_Lines.size();
	
	//每条线用4个元素表示，即直线的两个端点的4个坐标值 ;rho和theta分别为距离和角度的分辨率
	//threshold为阈值，
	//minLineLength为最小直线长度;maxLineGap为最大直线间隙如果这个间隙大于该值，则被认为是两条线段，否则是一条。
	for (size_t i = 0; i < possible_Lines.size(); i++)
	{
		//line(src, Point(possible_Lines[i][0], possible_Lines[i][1]),
			//Point(possible_Lines[i][2], possible_Lines[i][3]), Scalar(0, 0, 255), 2, 8);
		
		cout << "["<<possible_Lines[i][0]<<","<< possible_Lines[i][1]<<"]" 
			<< "["<<possible_Lines[i][2]<<","<<possible_Lines[i][3] << "]";
		printf("%f",(possible_Lines[i][3] - possible_Lines[i][1])*0.1 * 10 / (possible_Lines[i][2] - possible_Lines[i][0]));
		cout << endl;
	}
	//进行直线聚类，中心线
	vector<Vec4i> point1,point2;
	//point1[0] = possible_Lines[0];
	point1.push_back(possible_Lines[0]);
	float f1 = (possible_Lines[0][3] - possible_Lines[0][1])*0.1 * 10 / (possible_Lines[0][2] - possible_Lines[0][0]);
	float f2 = 0;
	for (size_t i = 1; i < possible_Lines.size(); i++)
	{
		f2 = (possible_Lines[i][3] - possible_Lines[i][1])*0.1 * 10 / (possible_Lines[i][2] - possible_Lines[i][0]);
		if ((f2-f1)<0.2 && (f2 - f1)>-0.2)
		{
			cout << i<<endl;
			point1.push_back( possible_Lines[i]);
		}
		else
		{
			point2.push_back(possible_Lines[i]);
		}
	}
	vector<int> s1(Median(point1));
	vector<int> s2(Median(point2));

	line(src, Point(s1[0], s1[1]),
		Point(s1[2], s1[3]), Scalar(0, 0, 255), 2, 8);
	line(src, Point(s2[0], s2[1]),
		Point(s2[2], s2[3]), Scalar(0, 0, 255), 2, 8);

	Point X=slot(Point(s1[0], s1[1]), Point(s1[2], s1[3]), Point(s2[0], s2[1]), Point(s2[2], s2[3]) );

	circle(src, X, 2, Scalar(0, 0, 255), 2, 8);
	line(src, X, Point(s1[2], s1[3]), Scalar(0, 0, 255), 2, 8);
	line(src, X, Point(s2[2], s2[3]), Scalar(0, 0, 255), 2, 8);

	imwrite("test1.jpg", src);
	imshow("haha", src);
	waitKey(0);
	return 0;
}

Point slot(Point point1, Point point2, Point point3, Point point4)
{
	float k1 = (point2.y - point1.y)*0.1*10 / (point2.x - point1.x);
	float b1 = point1.y-point1.x*k1;
	float k2 = (point4.y - point3.y)*0.1 * 10 / (point4.x - point3.x);
	float b2 = point3.y- point3.x*k2;
	float x = (b2 - b1) / (k1 - k2);
	float y = x*k1 + b1;
	Point X=Point(x, y);
	return X;
}


int median(vector<int> s)
{
	if (s.size()%2==0)
	{
		return (s[s.size() / 2] + s[s.size() / 2 - 1]) / 2;
	}
	else
	{
		return s[s.size() / 2];
	}
}

vector<int> Median(vector<Vec4i> point)
{
	vector<int> s1, s2, s3, s4, s5;

	for (size_t i = 1; i < point.size(); i++)
	{
		s1.push_back(point[i][0]);
		s2.push_back(point[i][1]);
		s3.push_back(point[i][2]);
		s4.push_back(point[i][3]);
		//cout << "test" << s1[0] << endl;
	}
	sort(s1.begin(), s1.end());
	sort(s2.begin(), s2.end());
	sort(s3.begin(), s3.end());
	sort(s4.begin(), s4.end());

	s5.push_back(median(s1));
	s5.push_back(median(s2));
	s5.push_back(median(s3));
	s5.push_back(median(s4));
	return s5;
}



/*for (size_t i = 1; i < point2.size(); i++)
	{
		s5.push_back(point2[i][0]);
		s6.push_back(point2[i][1]);
		s7.push_back(point2[i][2]);
		s8.push_back(point2[i][3]);
	}*/