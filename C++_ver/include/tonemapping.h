//Reinhard tone mapping
//http://www.cmap.polytechnique.fr/~peyre/cours/x2005signal/hdr_photographic.pdf

//parameter take from https ://github.com/felipegb94/hdr_imaging/blob/master/reinhardLocal.m

double RGB2Luminus(float R, float G, float B);

cv::Mat GammaCorrection(cv::Mat img, double gamma = 2.2);

cv::Mat Local(cv::Mat hdr, cv::Mat Lm, double a = 0.36, double phi = 12, double eps = 0.05);

cv::Mat Global(cv::Mat hdr, cv::Mat Lm);


void ReinhardToneMapping(std::string HDRfilename, double delta = 1e-5, double a = 0.36);