#pragma once
#ifndef RECOGNITION_H
#define RECOGNITION_H

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <SFML/Graphics.hpp>
#include <cmath>

extern const float radian;

enum class FindingStates
{
	FINDING_PUCK,
	FINDING_GOAL,
	FOUND_ALL,
};

cv::Mat sfml2opencv(const sf::Image& img);
cv::Mat GetThresholdedMat(cv::Mat& mat, FindingStates state);
sf::Image GetCountorsPic(const cv::Mat& thresholded, cv::Mat& img, std::vector<std::vector<cv::Point>>& contours);
cv::Point2f GetCentralPoint(const std::vector<std::vector<cv::Point>>& contours, const sf::Vector2u& imgSize);
sf::Image DefineObj(const sf::Image& img, std::vector<std::vector<cv::Point>>& contours, FindingStates state);
int Round(float num);
int GetAngle(float pointX);
float Radian2Degrees(float angle);

#endif // RECOGNITION_H
