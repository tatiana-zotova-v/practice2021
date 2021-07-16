#include "recognition.h"

const float radian = 57.3;
enum Object
{
	PUCK,
	GOAL,
};

cv::Mat sfml2opencv(sf::Image* img)
{
	cv::Size size(img->getSize().x, img->getSize().y);
	cv::Mat mat(size, CV_8UC4, (void*)img->getPixelsPtr(), cv::Mat::AUTO_STEP);
	cv::cvtColor(mat, mat, cv::COLOR_RGBA2BGR);
	return mat.clone();
}

cv::Mat GetThresholdedMat(cv::Mat* mat/*, Object toFind*/)
{
	int hueFrom = 40;
	int hueTo = 80;

	int saturationFrom = 130;
	int saturationTo = 255;

	int valueFrom = 180;
	int valueTo = 255;
	/*
	if (toFind == PUCK)
	{
		hueFrom = 40;
		hueTo = 80;

		valueFrom = 180;
	}
	else if (toFind == GOAL)
	{
		hueFrom = 110;
		hueTo = 130;

		valueFrom = 90;
	}
	else
	{
		//throw exception?
	}
	*/
	cv::Mat thresholded;
	cv::cvtColor(*mat, thresholded, cv::COLOR_BGR2HSV);

	cv::inRange(
		thresholded,
		cv::Scalar(hueFrom, saturationFrom, valueFrom),
		cv::Scalar(hueTo, saturationTo, valueTo),
		thresholded
	);

	cv::erode(
		thresholded,
		thresholded,
		cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5))
	);

	cv::dilate(
		thresholded,
		thresholded,
		cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5))
	);

	cv::dilate(
		thresholded,
		thresholded,
		cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5))
	);

	cv::erode(
		thresholded,
		thresholded,
		cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5))
	);

	return thresholded;
}

sf::Image GetCountorsPic(cv::Mat* thresholded
					   , cv::Mat* img
					   , std::vector<std::vector<cv::Point>>& contours)
{
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(
		*thresholded,
		contours,
		hierarchy,
		cv::RETR_TREE,
		cv::CHAIN_APPROX_SIMPLE,
		cv::Point(0, 0)
	);

	if (hierarchy.size() > 0)
	{
		for (int idx = 0; idx >= 0; idx = hierarchy[idx][0])
		{
			cv::Scalar color(0, 0, 255);
			drawContours(*img, contours, idx, color, 1, 8, hierarchy);
		}
	}

	cv::cvtColor(*img, *img, cv::COLOR_BGR2RGBA);
	sf::Image image;
	image.create(img->cols, img->rows, img->ptr());

	return image;
}

float GetCentralPoint(const std::vector<std::vector<cv::Point>>& contours
					, sf::Image* img)
{
	cv::Rect rect = cv::boundingRect(contours[0]);
	return static_cast<float>(rect.x + rect.width / 2) / static_cast<float>(img->getSize().x);
}
/*
float DefineGreen(const sf::Image& img) {
	cv::Mat img_mat = sfml2opencv(img);
	cv::Mat thresholded = GetThresholdedMat(img_mat);

	std::vector<std::vector<cv::Point>> contours;

	sf::Image result = GetCountorsPic(thresholded, img_mat, contours);
	std::cout << "size = " << contours.size() << std::endl;

	if(contours.size() == 1)
	{
		float point = GetCentralPoint(contours, img);
	}
	else if (contours.size() == 0)
	{
		//объекта нет в кадре
	} 
	else if (contours.size() > 1)
	{
		//какие такие не один объектов ты нашол... throw exception?
	}

	return 0;	
}
*/
float GetAngle(float pointX)
{
	if (pointX == 0.5)
	{
		return 0;
	}
	else
	{
		pointX -= 0.5;
		pointX *= 0.6;
		pointX /= 0.3;
		return std::atanf(pointX);
	}
}

float Radian2Degrees(float angle)
{
	return angle * radian;
}