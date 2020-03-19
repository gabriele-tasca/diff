#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <cmath>

static const float BIG_FLOAT = std::numeric_limits<float>::max(); 
static const uchar BIG_INT = std::numeric_limits<uchar>::max(); 
inline float DISTANCE(float x1, float y1, float x2, float y2) {
    if (x2 == -1 && y2 == -1)
    {
        // std::cout << "meno uno" << std::endl;
        return BIG_FLOAT;
    }
    return sqrt( pow(x1-x2, 2) + pow(y1-y2, 2)  );
}


int main(int argc, char** argv )
{
    ///////////////// 
    // VARIABLES
    ///////////////// 


    if ( argc != 2 )
    {
        printf("usage: DisplayImage.out <Image_Path>\n");
        return -1;
    }

    cv::Mat image;
    image = cv::imread( argv[1], cv::IMREAD_UNCHANGED );
    int ncols = image.cols;
    int nrows = image.rows;
    if ( !image.data )
    {
        printf("No image data \n");
        return -1;
    }


    cv::Mat closest = cv::Mat(ncols, nrows, CV_32SC(2));
    cv::Mat distances = cv::Mat(ncols, nrows, CV_32F);
    
    

    auto shuffled_rows = std::vector<int>(image.rows);
    auto shuffled_cols = std::vector<int>(image.cols);
    for (int z = 0; z < image.rows; z++) shuffled_rows[z] = z;
    for (int z = 0; z < image.cols; z++) shuffled_cols[z] = z;
    std::random_shuffle(shuffled_rows.begin(),shuffled_rows.end());
    std::random_shuffle(shuffled_cols.begin(),shuffled_cols.end());

    static const int TEMPL_DOWN [3][2] = {{-1,-1}, {-1,0}, {-1,1}};
    static const int TEMPL_UP [3][2] = {{1,-1}, {1,0},{1,1}};
    static const int TEMPL_RIGHT [3][2] = {{-1,-1}, {0,-1},{1,-1}};
    static const int TEMPL_LEFT [3][2] = {{-1,1}, {0,1},{1,1}};

    //UPDATE THE 8 
    static const float DIFF_UNIT_VECS [8][2] = {
        {0,1},
        {0,-1},
        {1,0},
        {-1,0},

        {0.7071067811865475, 0.7071067811865475},
        {0.7071067811865475, -0.7071067811865475},
        {-0.7071067811865475, 0.7071067811865475},
        {-0.7071067811865475, -0.7071067811865475},

    };
    static const int ndiffspots = 8;
    //UPDATE THE 8 



    ///////////////// 
    // FUNCTIONS
    ///////////////// 


    auto distmap_update_pixel = [&](int y, int x, const int TEMPL[3][2], int k)
    {
        int cy = y + TEMPL[k][0];
        int cx = x + TEMPL[k][1];
        float newdist = DISTANCE(y,x,closest.at<cv::Vec2i>(cy,cx)[0],closest.at<cv::Vec2i>(cy,cx)[1]);
        if (newdist < distances.at<float>(y,x)  )
        {
            image.at<cv::Vec4b>(y,x)[0] = image.at<cv::Vec4b>(cy,cx)[0];
            image.at<cv::Vec4b>(y,x)[1] = image.at<cv::Vec4b>(cy,cx)[1];
            image.at<cv::Vec4b>(y,x)[2] = image.at<cv::Vec4b>(cy,cx)[2];
            image.at<cv::Vec4b>(y,x)[3] = image.at<cv::Vec4b>(cy,cx)[3];

            closest.at<cv::Vec2i>(y,x)[0] = closest.at<cv::Vec2i>(cy,cx)[0]; 
            closest.at<cv::Vec2i>(y,x)[1] = closest.at<cv::Vec2i>(cy,cx)[1];
            distances.at<float>(y,x) = newdist;
        }
    };

    
    // in-place
    auto diffuse = [&](float shrink_factor = 1)
    {
        for( int linear_x = 0; linear_x < image.cols; linear_x++)
        {
            int x = shuffled_cols[linear_x];
            for( int linear_y = 0; linear_y < image.rows; linear_y++ )
            {
                int y = shuffled_cols[linear_y];
                // 3 channels
                for( int chan = 0; chan < 3; chan++)
                {
                    float truerad = distances.at<float>(y,x) * 0.92 * shrink_factor;
                    float newvalue = 0;
                    float count = 0;
                    // look around
                    for( int kappa = 0; kappa < ndiffspots; kappa++)
                    {
                        int cy = int( y + DIFF_UNIT_VECS[kappa][0]*truerad ); 
                        int cx = int( x + DIFF_UNIT_VECS[kappa][1]*truerad );
                        if (cy >= 0 && cy < image.rows && cx >= 0 && cx < image.cols)
                        {
                            newvalue += float(image.at<cv::Vec4b>(cy,cx)[chan]);
                            count++;
                        }
                    }
                    newvalue = newvalue/count;
                    image.at<cv::Vec4b>(y,x)[chan] = int(newvalue);
                }
            }
        }
    };

    // // not in-place
    // auto take_snapshot = [&](){
    //     snapshot = image.clone();
    //     cv::Mat_<cv::Vec4b> _snapshot = snapshot;
    //     // std::cout << "printerino" << std::endl;
    //     // cv::namedWindow("Display nug", cv::WINDOW_AUTOSIZE );
    //     // cv::imshow("Display nug", snapshot);
    //     // std::cout << "printerino2" << std::endl;
    // };
    // auto diffuse2 = [&]()
    // {
    //     snapshot = image.clone();
    //     cv::Mat_<cv::Vec4b> _snapshot = snapshot;

    //     for( int x = 0; x < image.cols; x++)
    //     {
    //         for( int y = 0; y < image.rows; y++ )
    //         {
    //             // 3 channels
    //             for( int chan = 0; chan < 3; chan++)
    //             {
    //                 float truerad = _distances(y,x) * 0.92;
    //                 float newvalue = 0;
    //                 float count = 0;
    //                 // look around
    //                 for( int kappa = 0; kappa < ndiffspots; kappa++)
    //                 {
    //                     int cy = int( y + DIFF_UNIT_VECS[kappa][0]*truerad ); 
    //                     int cx = int( x + DIFF_UNIT_VECS[kappa][1]*truerad );
    //                     if (cy >= 0 && cy < image.rows && cx >= 0 && cx < image.cols)
    //                     {
    //                         newvalue += float(_snapshot(cy,cx)[chan]);
    //                         count++;
    //                     }
    //                 }
    //                 newvalue = newvalue/count;
    //                 _image(y,x)[chan] = int(newvalue);
    //             }
    //         }
    //     }
    //     snapshot = _snapshot;
    // };


    ///////////////// 
    // MAIN
    ///////////////// 

    // init 
    for( int y = 0; y < image.rows; y++)
    {
        for( int x = 0; x < image.cols; x++ )
        {
            if (image.at<cv::Vec4b>(y,x)[3] != 0)
            {
                closest.at<cv::Vec2i>(y,x)[0] = y;
                closest.at<cv::Vec2i>(y,x)[1] = x;
                distances.at<float>(y,x) = 0;

            }
            else
            {
                closest.at<cv::Vec2i>(y,x)[0] = -1;
                closest.at<cv::Vec2i>(y,x)[1] = -1;
                distances.at<float>(y,x) = BIG_FLOAT;
            }

        }
    }

    // DISTANCE MAP SWEEPS
    // RIGHT SWEEP
    for( int x = 1; x < image.cols; x++)
    {
        // edge
        int y = 0;
        for( int k = 1; k < 3; k++) distmap_update_pixel(y,x,TEMPL_RIGHT,k);
        // bulk
        for( int y = 1; y < image.rows -1; y++ )
        {
            for( int k = 0; k < 3; k++) distmap_update_pixel(y,x,TEMPL_RIGHT,k);
        }
        // edge
        y = image.cols-1;
        for( int k = 0; k < 2; k++) distmap_update_pixel(y,x,TEMPL_RIGHT,k);
    }

    // LEFT SWEEP
    for( int x = image.cols-1; x >= 1; x--)
    {
        // edge
        int y = 0;
        for( int k = 1; k < 3; k++) distmap_update_pixel(y,x,TEMPL_LEFT,k);
        // bulk
        for( int y = 1; y < image.rows -1; y++ )
        {
            for( int k = 0; k < 3; k++) distmap_update_pixel(y,x,TEMPL_LEFT,k);
        }
        // edge
        y = image.cols-1;
        for( int k = 0; k < 2; k++) distmap_update_pixel(y,x,TEMPL_LEFT,k);
    }

    // DOWN SWEEP
    for( int y = 1; y < image.rows; y++)
    {
        // left edge
        int x = 0;
        for( int k = 1; k < 3; k++) distmap_update_pixel(y,x,TEMPL_DOWN,k);
        // bulk
        for( int x = 1; x < image.cols -1; x++ )
        {
            for( int k = 0; k < 3; k++) distmap_update_pixel(y,x,TEMPL_DOWN,k);
        }
        // right edge
        x = image.cols-1;
        for( int k = 0; k < 2; k++) distmap_update_pixel(y,x,TEMPL_DOWN,k);
    }

    // UP SWEEP
    for( int y = image.rows-1; y >= 1; y--)
    {
        // left edge
        int x = 0;
        for( int k = 1; k < 3; k++) distmap_update_pixel(y,x,TEMPL_UP,k);
        // bulk
        for( int x = 1; x < image.cols -1; x++ )
        {
            for( int k = 0; k < 3; k++) distmap_update_pixel(y,x,TEMPL_UP,k);
        }
        // right edge
        x = image.cols-1;
        for( int k = 0; k < 2; k++) distmap_update_pixel(y,x,TEMPL_UP,k);
    }

    // DIFFUSION STEPS
    int n_steps_before = 4;
    int n_steps_shrinking = 4;
    int step = 1;
    for (   ; step <= n_steps_before; step++)
        diffuse();
    for (   ; step <= n_steps_before + n_steps_shrinking; step++) 
        diffuse( (1- step/float(n_steps_shrinking)) );
    











    

 






    cv::imwrite( "cxxout.png", image );
    cv::namedWindow("Display Image", cv::WINDOW_AUTOSIZE );
    cv::imshow("Display Image", image);
    cv::waitKey(0);
    cv::destroyAllWindows();

    return 0;
}



