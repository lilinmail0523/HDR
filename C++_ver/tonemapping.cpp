#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <iostream>

#include "tonemapping.h"

//Reinhard tone mapping
//http://www.cmap.polytechnique.fr/~peyre/cours/x2005signal/hdr_photographic.pdf

//parameter take from https ://github.com/felipegb94/hdr_imaging/blob/master/reinhardLocal.m



double RGB2Luminus(float R, float G, float B) {
	return (double)0.27 * R + 0.67 * G + 0.06 * B;
}



//gamma correction = pow((i/255), (1/gamma)) * 255
cv::Mat GammaCorrection(cv::Mat img, double gamma) {

	cv::Mat res = cv::Mat::zeros(img.rows, img.cols, CV_8UC3);

	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			for (int ch = 0; ch < 3; ch++) {
				res.at<cv::Vec3b>(i, j)[ch] = cv::saturate_cast<uchar>(pow(img.at<cv::Vec3f>(i, j)[ch] / 255.0, 1 / gamma) * 255.0);;
			}
		}

	}
	return res;


}


cv::Mat Local(cv::Mat hdr, cv::Mat Lm, double a, double phi, double eps) {

	double alpha = 1 / (2 * sqrt(2));
	int ScaleLevel = 9;

	std::vector<cv::Mat> V1;

	V1.push_back(Lm);
	for (int i = 0; i < ScaleLevel; i++) {
		double s = pow(1.6, i);
		double sigma = alpha * s;
		cv::Mat blur;
		cv::GaussianBlur(Lm, blur, cv::Size(0, 0), sigma, sigma, cv::BORDER_REPLICATE);
		V1.push_back(blur);
	}

	std::vector<cv::Mat>VS;
	for (int scale = 0; scale < ScaleLevel; scale++) {
		double s = pow(1.6, scale);
		cv::Mat vs(Lm.rows, Lm.cols, CV_64F);
		for (int i = 0; i < Lm.rows; i++) {
			for (int j = 0; j < Lm.cols; j++) {
				double blur = V1[scale + 1].at<double>(i, j);
				double blurprev = V1[scale].at<double>(i, j);

				vs.at<double>(i, j) = abs((blur - blurprev) / (pow(2, phi) * a / (s * s) + blurprev));

			}
		}
		VS.push_back(vs);
	}
	std::vector<std::vector<int>> smax(Lm.rows, std::vector<int>(Lm.cols, 0));
	for (int i = 0; i < Lm.rows; i++) {
		for (int j = 0; j < Lm.cols; j++) {
			for (int scale = 0; scale < ScaleLevel; scale++) {
				if (VS[scale].at<double>(i, j) > eps && scale > 0) {
					smax[i][j] = scale - 1;
					break;
				}
			}
		}
	}


	cv::Mat ldr = cv::Mat::zeros(hdr.rows, hdr.cols, CV_32FC3);

	for (int i = 0; i < hdr.rows; i++) {
		for (int j = 0; j < hdr.cols; j++) {
			double Lumin_Lw = RGB2Luminus(hdr.at<cv::Vec3f>(i, j)[0], hdr.at<cv::Vec3f>(i, j)[1], hdr.at<cv::Vec3f>(i, j)[2]);
			double pix = Lm.at<double>(i, j);
			int Lsmax = smax[i][j];
			for (int ch = 0; ch < 3; ch++) {
				ldr.at<cv::Vec3f>(i, j)[ch] = (float)hdr.at<cv::Vec3f>(i, j)[ch] * (pix / (Lumin_Lw * (1 + V1[Lsmax].at<double>(i, j)))) * 255.0;

			}
		}
	}



	return GammaCorrection(ldr);


}


cv::Mat Global(cv::Mat hdr, cv::Mat Lm) {
	double min = 0, max = 0;
	double* minp = &min, * maxp = &max;
	cv::minMaxIdx(Lm, minp, maxp);

	cv::Mat ldr = cv::Mat::zeros(hdr.rows, hdr.cols, CV_32FC3);
	for (int i = 0; i < hdr.rows; i++) {
		for (int j = 0; j < hdr.cols; j++) {
			double Lumin_Lw = RGB2Luminus(hdr.at<cv::Vec3f>(i, j)[0], hdr.at<cv::Vec3f>(i, j)[1], hdr.at<cv::Vec3f>(i, j)[2]);
			double pix = Lm.at<double>(i, j);
			double Ld = pix * (1 + pix / (max * max)) / (1 + pix);
			for (int ch = 0; ch < 3; ch++) {
				ldr.at<cv::Vec3f>(i, j)[ch] = (float)hdr.at<cv::Vec3f>(i, j)[ch] * (Ld / Lumin_Lw) * 255;
			}
		}
	}


	return GammaCorrection(ldr);
}






void ReinhardToneMapping(std::string HDRfilename, double delta, double a ) {
	cv::Mat hdr = cv::imread(HDRfilename, cv::IMREAD_UNCHANGED);
	double Lw_avg = 0;
	for (int i = 0; i < hdr.rows; i++) {
		for (int j = 0; j < hdr.cols; j++) {
			double Lumin_Lw = RGB2Luminus(hdr.at<cv::Vec3f>(i, j)[0], hdr.at<cv::Vec3f>(i, j)[1], hdr.at<cv::Vec3f>(i, j)[2]);
			Lw_avg += log(Lumin_Lw + delta);
		}
	}

	Lw_avg = exp(Lw_avg / (static_cast<unsigned __int64> (hdr.rows) * hdr.cols));

	cv::Mat Lm = cv::Mat::zeros(hdr.rows, hdr.cols, CV_64F);
	for (int i = 0; i < hdr.rows; i++) {
		for (int j = 0; j < hdr.cols; j++) {
			double Lumin_Lw = RGB2Luminus(hdr.at<cv::Vec3f>(i, j)[0], hdr.at<cv::Vec3f>(i, j)[1], hdr.at<cv::Vec3f>(i, j)[2]);
			Lm.at<double>(i, j) = (a / Lw_avg) * Lumin_Lw;
		}
	}



	cv::Mat global = Global(hdr, Lm);

	cv::imwrite("Global.png", global);

	std::cout << "Global tone mapping done!" << std::endl;

	cv::Mat local = Local(hdr, Lm);

	cv::imwrite("Local.png", local);

	std::cout << "Local tone mapping done!" << std::endl;


}

