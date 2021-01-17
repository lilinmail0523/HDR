
void float2rgbe(unsigned char rgbe[4], float red, float green, float blue);

void HDRwriter(std::string filename, cv::Mat img);

std::tuple <cv::Mat, cv::Mat> gsolve(std::vector<std::vector<int>>& Z, std::vector<double>& B, int l, std::vector<int>& w);

void HDR(std::vector<std::string>& Imglist, std::vector<double>& Explist);


