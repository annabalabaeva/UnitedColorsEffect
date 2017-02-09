#include <iostream>
#include <string>

#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"

/** Max effect degree (procent) */
const int EFFECT_SLIDER_MAX = 100;
/** RGB image channels count */
const int N_RGB_CHANNELS = 3;



/**
 * A struct to pass data to modify image to On trackbar changed event handler.
 * @see on on_trackbar_changed(int, void* matrixes)
 */
struct EffectContext {
    cv::Mat original_img; /** Loaded image (CV_16SC3 or CV_16SC4 format) */
    cv::Mat transforming_matrix; /** Matrix modifying original image (CV_16SC3 or CV_16SC4 format) */
    cv::Mat changed_img; /** Modified image (CV_8UC3 or CV_8UC4 format) */
    int effect_degree_procent = 0; /** Current effect degree (procent) */
};

void on_trackbar_changed(int, void* matrixes);
void fill_transform_matrix(cv::Mat& transform_matrix, cv::Mat& img_original);
bool image_open(cv::Mat& img_original, std::string& file_path, int imread_flag);
bool image_save(const cv::Mat& img_changed, std::string& file_path);
void convert_slash_string(std::string& file_path);

int main(int argc, const char** argv) {
    int transform_matrix_type = CV_16SC3;
    int img_read_type         = cv::IMREAD_COLOR;
    if (argc == 4) {
        // change flags if image is RGBA
        if (strcmp(argv[3], "4") == 0) {
            img_read_type = cv::IMREAD_UNCHANGED;
            transform_matrix_type = CV_16SC4;
        }
    }
    else if (!(argc == 3)) {
        std::cout << "You should write path to input & output files." 
            << std::endl;
        return EXIT_FAILURE;
    }


    cv::Mat img_original;
    if (!image_open(img_original, static_cast<std::string>(argv[1]), img_read_type)) {
        std::cout << "Wrong input file path." << std::endl;
        return EXIT_FAILURE;
    }


    cv::Mat img_changed(img_original.rows, img_original.cols, 
                        img_original.type());


    cv::Mat transform_matrix(img_original.rows, img_original.cols, 
                             transform_matrix_type);
    fill_transform_matrix(transform_matrix, img_original);

    /* original matrix should be converted to CV_16SC format 
       to be used while calculating result image */
    cv::Mat img_original_16s;
    img_original.convertTo(img_original_16s, transform_matrix_type);

    EffectContext context;
    context.original_img = img_original_16s;
    context.transforming_matrix = transform_matrix;
    context.changed_img.create(img_changed.size(), img_changed.type());

    cv::namedWindow("United Colors Effect", CV_WINDOW_AUTOSIZE);

    std::string trackbar_name = "Effect";
    cv::createTrackbar(trackbar_name, "United Colors Effect",
                       &context.effect_degree_procent, EFFECT_SLIDER_MAX,
                       on_trackbar_changed, &context);

    cv::imshow("United Colors Effect", img_original);
    cv::waitKey(0);


    if (!image_save(img_changed, static_cast<std::string>(argv[2]))) {
        std::cout << "Can't save changed image. Check output file path." 
            << std::endl;
        return EXIT_FAILURE;
    }

    cv::destroyWindow("United Colors Effect");

    return EXIT_SUCCESS;
}


/**
 * On trackbar changed event handler.
 * @param pos integer trackbar position
 * @param img_matrixes pointer to struct with original, modifying and changed matrixes
 * @see EffectContext
 */
static void on_trackbar_changed(int pos, void* img_matrixes) {
    
    EffectContext context = *((EffectContext*)img_matrixes);
    double effect_degree_multiplier = 
        (double)context.effect_degree_procent / EFFECT_SLIDER_MAX;
    cv::Mat temp;
    temp = context.original_img +
           effect_degree_multiplier*context.transforming_matrix;
    temp.convertTo(context.changed_img, context.changed_img.type());

    cv::imshow("United Colors Effect", context.changed_img);
}


/**
 * Function compute and fill data to transform_matrix.
 * @param transform_matrix reference to transform matrix.
 * @param img_original original image. Transform matrix data is calculated 
 * using original image.
 */
void fill_transform_matrix(cv::Mat& transform_matrix, cv::Mat& img_original) {
    if (img_original.channels() == N_RGB_CHANNELS) {
        cv::MatIterator_<cv::Vec3b> it3b, end3b;
        cv::MatIterator_<cv::Vec3s> it3s;
        it3s =  transform_matrix.begin<cv::Vec3s>();
        end3b = img_original.end<cv::Vec3b>();
        for (it3b = img_original.begin<cv::Vec3b>(); it3b != end3b; ++it3b) {
            (*it3s)[0] = -(*it3b)[0];//B
            (*it3s)[1] = (*it3b)[2] - (*it3b)[1];//G
            (*it3s)[2] = 255 - (*it3b)[2];//R
            it3s++;
        }
    }
    else {
        cv::MatIterator_<cv::Vec4b> it4b, end4b;
        cv::MatIterator_<cv::Vec4s> it4s;
        it4s =  transform_matrix.begin<cv::Vec4s>();
        end4b = img_original.end<cv::Vec4b>();
        for (it4b = img_original.begin<cv::Vec4b>(); it4b != end4b; ++it4b) {
            (*it4s)[0] = -(*it4b)[0];//B
            (*it4s)[1] = (*it4b)[2] - (*it4b)[1];//G
            (*it4s)[2] = 255 - (*it4b)[2];//R
            (*it4s)[3] = 0;
            it4s++;
        }
    }
}


/**
 * Function reads image located in file_path_char with flag from imread_flag
 * and put data to imgOriginal.
 * @param img_original reference to Mat object where original image should be saved
 * @param file_path path to file with original image
 * @param imread_flag integer flag to read original image as RGB or RGBA
 * @return true if there is no error. False if there is an error 
 * while reading (e.g., wrong file path)
 */
bool image_open(cv::Mat& img_original, std::string& file_path, int imread_flag) {
    convert_slash_string(file_path);
    img_original = cv::imread(file_path, imread_flag);
    if (img_original.empty()) return false;
    else return true;
}


/**
 * Function writes image from img_changed to location specified in file_path_char.
 * @param img_changed reference to Mat object with changed image
 * @param file_path path to file where changed image should be saved
 * @return true if there is no error. False if there is an error
 * while writing (e.g., wrong file path)
 */
bool image_save(const cv::Mat& img_changed, std::string& file_path) {
    convert_slash_string(file_path);
    if (cv::imwrite(file_path, img_changed)) {
        return true;
    }
    else {
        return false;
    }
}

/**
 * Function add second back-slash to string.
 * @param file_path reference to string
 */
void convert_slash_string(std::string& file_path) {
    int i = 0;
    while (i < file_path.length()) {
        if (file_path[i] == '\\') {
            file_path.insert(i, "\\");
            i++;
        }
        i++;
    }
}