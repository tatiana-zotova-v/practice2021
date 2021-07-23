#include "recognition.h"

const float radian = 57.3;

cv::Mat sfml2opencv(const sf::Image& img)
{
	cv::Size size(img.getSize().x, img.getSize().y);
	cv::Mat mat(size, CV_8UC4, (void*)img.getPixelsPtr(), cv::Mat::AUTO_STEP);
	cv::cvtColor(mat, mat, cv::COLOR_RGBA2BGR);
	return mat.clone();
}

cv::Mat GetThresholdedMat(cv::Mat& mat, FindingStates state)
{
	int hueFrom = 0;
	int hueTo = 180;

	int saturationFrom = 0;
	int saturationTo = 255;

	int valueFrom = 0;
	int valueTo = 255;

	if (state == FindingStates::FINDING_PUCK)
	{
		hueFrom = 50;
		hueTo = 70;
		saturationFrom = 110;
		saturationTo = 255;
		valueFrom = 100;
		valueTo = 255;
	}
	else if (state == FindingStates::FINDING_GOAL)
	{
		hueFrom = 110;
		hueTo = 130;
		saturationFrom = 130;
		saturationTo = 255;
		valueFrom = 80;
		valueTo = 255;
	}

	cv::Mat thresholded;
	cv::cvtColor(mat, thresholded, cv::COLOR_BGR2HSV);

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

sf::Image GetCountorsPic(const cv::Mat& thresholded, cv::Mat& img, std::vector<std::vector<cv::Point>>& contours)
{
	std::vector<cv::Vec4i> hierarchy;
	cv::findContours(
		thresholded,
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
			drawContours(img, contours, idx, color, 1, 8, hierarchy);
		}
	}

	cv::cvtColor(img, img, cv::COLOR_BGR2RGBA);
	sf::Image image;
	image.create(img.cols, img.rows, img.ptr());

	return image;
}

cv::Point2f GetCentralPoint(const std::vector<std::vector<cv::Point>>& contours, const sf::Vector2u& imgSize)
{
	cv::Rect2f rect = cv::boundingRect(contours[0]);
	cv::Point2f central(
		static_cast<float>(rect.x + rect.width / 2) / static_cast<float>(imgSize.x)
	  , static_cast<float>(rect.y + rect.height / 2) / static_cast<float>(imgSize.y)
	);
	return central;
}

sf::Image DefineObj(const sf::Image& img, std::vector<std::vector<cv::Point>>& contours, FindingStates state)
{
	cv::Mat img_mat = sfml2opencv(img);
	cv::Mat thresholded = GetThresholdedMat(img_mat, state);
	sf::Image contoured = GetCountorsPic(thresholded, img_mat, contours);
	return contoured;
}

int Round(float num)
{
	float integer = floorf(num);
	if ((num - integer) < 0.5)
	{
		return (num > 0)
			? static_cast<int>(floorf(num))
			: static_cast<int>(ceilf(num));
	}
	else
	{
		return (num < 0)
			? static_cast<int>(floorf(num))
			: static_cast<int>(ceilf(num));
	}
}

int GetAngle(float pointX)
{
	pointX -= 0.5;
	pointX *= 0.44;
	pointX /= 0.22;
	float angle = Radian2Degrees(std::atanf(pointX));
	return Round(angle);
}

float Radian2Degrees(float angle)
{
	return angle * radian;
}