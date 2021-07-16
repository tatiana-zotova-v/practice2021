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
extern enum Object;

cv::Mat sfml2opencv(sf::Image* img);
cv::Mat GetThresholdedMat(cv::Mat* mat/*, Object toFind*/);
sf::Image GetCountorsPic(cv::Mat* thresholded
					   , cv::Mat* img
					   , std::vector<std::vector<cv::Point>>& contours);
float GetCentralPoint(const std::vector<std::vector<cv::Point>>& contours
					, sf::Image* img);
//float DefineGreen(const sf::Image& img);
float GetAngle(float pointX);
float Radian2Degrees(float angle);

#endif // RECOGNITION_H
