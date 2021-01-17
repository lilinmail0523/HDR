#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>


#include "mtb.h"
#include "tonemapping.h"
#include "hdr.h"


int main() {
	std::string Imgfile;
	std::ifstream  Imagelist("image_list.txt");
	std::vector<std::string> imglist;

	if (!Imagelist) {
		std::cout << " Image List isn't loaded!" << std::endl;
		return 0;
	}
	else {
		while (std::getline(Imagelist, Imgfile)) {
			imglist.push_back(Imgfile);
		}
	}
	Imagelist.close();

	std::string Exposurefile;
	std::ifstream  Exposurelist("exposure.txt");
	std::vector<double> Explist;

	if (!Exposurelist) {
		std::cout << "Exposure List isn't loaded!" << std::endl;
		return 0;
	}
	else {
		while (std::getline(Exposurelist, Exposurefile)) {
			Explist.push_back(std::stod(Exposurefile));
		}
	}

	Exposurelist.close();
	std::cout << "---" << "starting alignment" << "---" << std::endl;
	mtb(imglist);
	std::cout << "---"  << "Alignment complete!" << "---" << std::endl;

	std::cout << "---" << "starting recovering response curve" << "---" << std::endl;
	HDR(imglist, Explist);
	std::cout << "---" << "Response curve reconstructed" << "---" << std::endl;


	std::cout << "---" << "starting Tone mapping" << "---" << std::endl;
	ReinhardToneMapping("HDR.hdr");
	std::cout << "Done!" << std::endl;

	return 0;
}

