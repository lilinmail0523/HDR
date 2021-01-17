#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include<tuple>

#include<time.h>

#include "hdr.h"

void float2rgbe(unsigned char rgbe[4], float red, float green, float blue) {
	float v;
	int e;
	v = red;
	if (green > v) {
		v = green;
	}
	if (blue > v) {
		v = blue;
	}
	if (v < 1e-32) {
		rgbe[0] = rgbe[1] = rgbe[2] = rgbe[3] = 0;
	}
	else {
		v = frexp(v, &e) * 256 / v;
		rgbe[0] = (unsigned int)(red * v);
		rgbe[1] = (unsigned int)(green * v);
		rgbe[2] = (unsigned int)(blue * v);
		rgbe[3] = (unsigned int)(e + 128);
	}
}

void HDRwriter(std::string filename, cv::Mat img) {
	std::ofstream hdr(filename, std::ios::binary);
	hdr << "#?RADIANCE\n";
	hdr << "FORMAT=32-bit_rle_rgbe\n\n";
	hdr << "-Y " << img.rows << " +X " << img.cols << "\n";
	unsigned char rgbe[4];
	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			float2rgbe(rgbe, img.at<cv::Vec3f>(i, j)[2], img.at<cv::Vec3f>(i, j)[1], img.at<cv::Vec3f>(i, j)[0]);
			hdr.write(reinterpret_cast<const char*> (rgbe), sizeof(rgbe));
		}
	}
	hdr.close();
}

//reference from debevec's HDR method (gsolve.m)
std::tuple <cv::Mat, cv::Mat> gsolve(std::vector<std::vector<int>>& Z, std::vector<double>& B, int l, std::vector<int>& w) {
	int n = 256;
	cv::Mat A = cv::Mat::zeros(Z.size() * Z[0].size() + n + 1, n + Z.size(), CV_64F);
	cv::Mat b = cv::Mat::zeros(A.rows, 1, CV_64F);


	int k = 0;
	for (int i = 0; i < Z.size(); i++) {
		for (int j = 0; j < Z[0].size(); j++) {
			double wij = w[Z[i][j]];
			//::cout << wij * B[j];
			A.at<double>(k, Z[i][j]) = wij;
			A.at<double>(k, n + i) = -wij;
			b.at<double>(k, 0) = wij * B[j];
			k++;

		}
	}

	A.at<double>(k, 128) = 1;
	k++;

	for (int i = 0; i < n - 2; i++) {
		A.at<double>(k, i) = (double)l * w[i + 1];
		A.at<double>(k, i + 1) = (double)-2 * l * w[i + 1];
		A.at<double>(k, i + 2) = (double)l * w[i + 1];
		k++;

	}

	cv::Mat x;
	cv::solve(A, b, x, cv::DECOMP_SVD);



	cv::Mat g = x(cv::Range(0, n), cv::Range(0, x.cols));
	cv::Mat lE = x(cv::Range(n, x.rows), cv::Range(0, x.cols));

	return { g, lE };
}

void HDR(std::vector<std::string>& Imglist, std::vector<double>& Explist) {

	clock_t start = clock();

	std::vector<cv::Mat> cImgs;
	for (std::string img : Imglist) {
		cImgs.push_back(cv::imread("Aligned_" + img, cv::IMREAD_UNCHANGED));
	}

	// sample 100 points
	int SampleSize = 100;
	std::vector<std::vector<std::vector<int>>> Z;
	for (int ch = 0; ch < 3; ch++) {
		Z.push_back(std::vector<std::vector<int>>());
		for (int i = 0; i < SampleSize; i++) {
			int x = rand() % cImgs[0].rows;
			int y = rand() % cImgs[0].cols;
			Z[ch].push_back(std::vector<int>());
			for (int j = 0; j < cImgs.size(); j++) {
				Z[ch][i].push_back(cImgs[j].at<cv::Vec3b>(x, y)[ch]);
			}
		}
	}
	//Read exposure
	std::vector<double> B(Explist.size(), 0);
	for (int ex = 0; ex < Explist.size(); ex++) {
		B[ex] = log(Explist[ex]);
	}

	//Calculate weighting function w(z)
	std::vector<int> w(256);
	for (int i = 0; i < 256; i++) {
		w[i] = (i <= 127) ? i : 255 - i;
	}
	//lamda l
	int l = 50;
	std::vector<cv::Mat> lElist;
	std::vector<cv::Mat> glist;

	for (int ch = 0; ch < 3; ch++) {
		cv::Mat g, lE;
		std::tie(g, lE) = gsolve(Z[ch], B, l, w);
		glist.push_back(g);
		lElist.push_back(lE);
	}

	cv::Mat E = cv::Mat::zeros(cImgs[0].rows, cImgs[0].cols, CV_32FC3);
	for (int ch = 0; ch < 3; ch++) {
		for (int i = 0; i < cImgs[0].rows; i++) {
			for (int j = 0; j < cImgs[0].cols; j++) {
				double w_sum = 1e-8;
				double ln_rad_sum = 0;
				for (int ind = 0; ind < cImgs.size(); ind++) {
					int pix = cImgs[ind].at<cv::Vec3b>(i, j)[ch];
					double w_pix = w[pix];
					double g_pix = glist[ch].at<double>(pix, 0);
					ln_rad_sum += w_pix * (glist[ch].at<double>(pix, 0) - B[ind]);
					w_sum += w_pix;

				}

				E.at<cv::Vec3f>(i, j)[ch] = (float)exp(ln_rad_sum / w_sum);
			}
		}
	}


	clock_t end = clock();

	std::cout << "Time of solving HDR: " << difftime(end, start) <<std::endl;

	HDRwriter("HDR.hdr", E);

}


