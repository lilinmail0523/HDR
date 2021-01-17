#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <iostream>
#include<algorithm>
#include <fstream>
#include<tuple>

#include "mtb.h"

// Image shift (dy, dx) pixels
cv::Mat ShiftImg(cv::Mat img, int dx, int dy) {
	cv::Mat shift_matrix = (cv::Mat_<double>(2, 3) << 1, 0, dx, 0, 1, dy);
	cv::warpAffine(img, img, shift_matrix, img.size());
	return img;
}

//Find median by histogram 
int FindMed(cv::Mat& Img) {
	int pix[256] = { 0 };
	int pixelNumber = Img.rows * Img.cols;
	for (int i = 0; i < Img.rows; i++) {
		for (int j = 0; j < Img.cols; j++)
			pix[Img.at<uchar>(i, j)]++;
	}
	int PixNumSum = 0;
	for (int i = 0; i < 256; i++) {
		PixNumSum += pix[i];
		if (PixNumSum > pixelNumber / 2) return i - 1;
	}
	return -1;
}

//construct the bitmap of threshold image and exclusive image
std::tuple<cv::Mat, cv::Mat> Bitmap(cv::Mat& Img) {
	int med = FindMed(Img);
	cv::Mat tb = cv::Mat::zeros(Img.rows, Img.cols, CV_8U);
	cv::Mat eb = cv::Mat::zeros(Img.rows, Img.cols, CV_8U);
	for (int i = 0; i < Img.rows; i++) {
		for (int j = 0; j < Img.cols; j++) {
			if (Img.at<uchar>(i, j) > med) {
				tb.at<uchar>(i, j) = 255;
			}
			if (abs(Img.at<uchar>(i, j) - med) >= 5) {
				eb.at<uchar>(i, j) = 255;
			}
		}
	}
	return { tb, eb };
}



std::pair<int, int> Getoffset(cv::Mat& GroundImg, cv::Mat Img2Shift, std::pair<int, int> offset) {

	int MinErr = GroundImg.rows * GroundImg.cols;
	cv::Mat Gbit, GExbit, Imgbit, ImbExbit;
	std::tie(Gbit, GExbit) = Bitmap(GroundImg);
	std::tie(Imgbit, ImbExbit) = Bitmap(Img2Shift);


	std::pair<int, int> shift = { 0,0 };
	int bit[3] = { -1,0,1 };
	for (int i : bit) {
		for (int j : bit) {
			int dx = offset.first + i;
			int dy = offset.second + j;

			cv::Mat ShtImgbit = ShiftImg(Imgbit, dx, dy);
			cv::Mat ShtImbExbit = ShiftImg(ImbExbit, dx, dy);

			cv::Mat diff;
			cv::bitwise_xor(Gbit, ShtImgbit, diff);
			cv::bitwise_and(diff, GExbit, diff);
			cv::bitwise_and(diff, ShtImbExbit, diff);

			int err = 0;
			for (int i = 0; i < diff.rows; i++) {
				for (int j = 0; j < diff.cols; j++) {
					err += diff.at<uchar>(i, j);
				}
			}
			if (i == 0 && j == 0) {
				err -= 1;
			}

			if (err < MinErr) {
				MinErr = err;
				shift.first = dx;
				shift.second = dy;
			}


		}
	}
	return shift;

}


//Pyramid of mtb
std::pair<int, int> mtbPyramid(cv::Mat& GroundImg, cv::Mat Img2Shift, int level) {
	if (level > 0) {
		cv::Mat ShrinkGroundImg, ShrinkImg2Shift;
		cv::resize(GroundImg, ShrinkGroundImg, cv::Size(), 0.5, 0.5);
		cv::resize(GroundImg, ShrinkImg2Shift, cv::Size(), 0.5, 0.5);
		std::pair<int, int> PastOffset = mtbPyramid(ShrinkGroundImg, ShrinkImg2Shift, level - 1);
		PastOffset.first *= 2;
		PastOffset.second *= 2;
		std::pair<int, int> Offset = Getoffset(GroundImg, Img2Shift, PastOffset);
		return Offset;
	}
	return { 0,0 };
}



void mtb(std::vector<std::string> & imglist) {


	std::vector<cv::Mat> cImgs;
	std::vector<cv::Mat> gImgs;
	for (std::string img : imglist) {
		gImgs.push_back(cv::imread(img, cv::IMREAD_GRAYSCALE));
		cImgs.push_back(cv::imread(img, cv::IMREAD_UNCHANGED));
	}


	for (int i = 0; i < cImgs.size(); i++) {
		std::pair<int, int> offset = mtbPyramid(gImgs[gImgs.size() / 2], gImgs[i], 4);
		std::cout << imglist[i] << "\toffset: " << offset.first << ", " << offset.second << std::endl;
		cImgs[i] = ShiftImg(cImgs[i], offset.first, offset.second);
		cv::imwrite("Aligned_" + imglist[i], cImgs[i]);
	}

}
