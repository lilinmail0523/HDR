
cv::Mat ShiftImg(cv::Mat img, int dx, int dy);

int FindMed(cv::Mat& Img);

std::tuple<cv::Mat, cv::Mat> Bitmap(cv::Mat& Img);

std::pair<int, int> Getoffset(cv::Mat& GroundImg, cv::Mat Img2Shift, std::pair<int, int> offset);

std::pair<int, int> mtbPyramid(cv::Mat& GroundImg, cv::Mat Img2Shift, int level);

void mtb(std::vector<std::string>& imglist);