#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <cmath>

using namespace std;

static const float BIG_FLOAT = std::numeric_limits<float>::max(); 
static const uchar BIG_INT = std::numeric_limits<uchar>::max(); 
inline float DISTANCE(float x1, float y1, float x2, float y2) {
    if (x2 == -1 || y2 == -1)
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

    // cv::IMREAD_UNCHANGED
    // CV_16FC(4)
    cv::Mat rawimage = cv::imread( argv[1],  cv::IMREAD_UNCHANGED);
    cv::Mat_<cv::Vec4b> image = rawimage;
    
    int ncols = image.cols;
    int nrows = image.rows;
    if ( !image.data )
    {
        printf("No image data \n");
        return -1;
    }

    cv::Mat_<cv::Vec2i> closest = cv::Mat(nrows, ncols, CV_32SC(2));
    cv::Mat_<float> distances = cv::Mat(nrows, ncols, CV_32F);
    


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


    auto distmap_update_pixel = [&](int y, int x, const int TEMPL[2])
    {
        int cy = y + TEMPL[0];
        int cx = x + TEMPL[1];
        float newdist = DISTANCE(y,x,closest(cy,cx)[0],closest(cy,cx)[1]);

        if (newdist < distances(y,x) )
        {
            image(y,x)[0] = image(cy,cx)[0];
            image(y,x)[1] = image(cy,cx)[1];
            image(y,x)[2] = image(cy,cx)[2];
            image(y,x)[3] = image(cy,cx)[3];

            closest(y,x)[0] = closest(cy,cx)[0]; 
            closest(y,x)[1] = closest(cy,cx)[1];
            distances(y,x) = newdist;
        }   
    };

    
    // in-place diffuse
    auto diffuse = [&](float shrink_factor = 1, float radius_override = -1)
    {
        for( int linear_x = 0; linear_x < image.cols; linear_x++)
        {
            int x = shuffled_cols[linear_x];
            for( int linear_y = 0; linear_y < image.rows; linear_y++ )
            {
                int y = shuffled_rows[linear_y];
                // 3 channels
                for( int chan = 0; chan < 3; chan++)
                {
                    float truerad;
                    if (radius_override != -1) truerad = radius_override;
                    else truerad = distances(y,x) * 0.92 * shrink_factor;
                    float newvalue = 0;
                    float count = 0;
                    // look around
                    for( int kappa = 0; kappa < ndiffspots; kappa++)
                    {
                        int cy = int( y + DIFF_UNIT_VECS[kappa][0]*truerad ); 
                        int cx = int( x + DIFF_UNIT_VECS[kappa][1]*truerad );
                        if (cy >= 0 && cy < image.rows && cx >= 0 && cx < image.cols)
                        {
                            newvalue += float(image(cy,cx)[chan]);
                            count++;
                        }
                    }
                    newvalue = newvalue/count;
                    image(y,x)[chan] = int(newvalue);
                }
            }
        }
    };

    // // not in-place
    // auto take_snapshot = [&](){
    //     snapshot = image.clone();
    //     cv::Mat_<cv::Vec4b> _snapshot = snapshot;
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
    //                 float truerad = distances(y,x) * 0.92;
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
    //                 image(y,x)[chan] = int(newvalue);
    //             }
    //         }
    //     }
    //     snapshot = _snapshot;
    // };


    ///////////////// 
    // MAIN
    ///////////////// 

    // init
    // find max alpha
    float max_alpha = 0;;
    int tempcols = image.cols;
    int temprows = image.rows;
    if (image.isContinuous())
    {
        tempcols *= temprows;
        temprows = 1;
    }
    uchar* p;
    for(int i = 0; i < temprows; ++i)
    {
        p = image.ptr<uchar>(i);
        for (int j = 0; j < tempcols; ++j)
        {
            // if (float(p[j*4 + 3]) == 255.) { max_alpha = 255.; break; }
            if (float(p[j*4 + 3]) > max_alpha) { max_alpha = float(p[j*4 + 3]); }
        }
    }
    std::cout <<setprecision(10)<< "max alpha " << max_alpha << "\n";

    // initialize pixels as either boundary (fixed) or to-be-painted-over
    for( int y = 0; y < image.rows; y++)
    {
        for( int x = 0; x < image.cols; x++ )
        {
            // std::cout << float(image(y,x)[3]) << " ";
            if ( float(image(y,x)[3]) >= max_alpha )
            {
                // image(y,x)[3] = max_alpha;
                // normal init
                closest(y,x)[0] = y;
                closest(y,x)[1] = x;
                distances(y,x) = 0;
            }
            else
            {
                distances(y,x) = BIG_FLOAT;
                closest(y,x)[0] = -1;
                closest(y,x)[1] = -1;
            }
        }
    }

    // RIGHT SWEEP
    for( int x = 1; x < image.cols; x++)
    {
        // edge
        int edgey1 = 0;
        for( int k = 1; k < 3; k++) distmap_update_pixel(edgey1,x,TEMPL_RIGHT[k]);

        // bulk
        for( int y = 1; y <= image.rows-2; y++ )
        {
            for( int k = 0; k < 3; k++) distmap_update_pixel(y,x,TEMPL_RIGHT[k]);
        }
        // edge
        int edgey2 = image.rows-1;
        for( int k = 0; k < 2; k++) distmap_update_pixel(edgey2,x,TEMPL_RIGHT[k]);
    }


    // LEFT SWEEP
    for( int x = image.cols-2; x >= 0; x--)
    {
        // edge
        int edgey1 = 0;
        for( int k = 1; k < 3; k++) distmap_update_pixel(edgey1,x,TEMPL_LEFT[k]);
        // bulk
        for( int y = 1; y <= image.rows-2; y++ )
        {
            for( int k = 0; k < 3; k++) distmap_update_pixel(y,x,TEMPL_LEFT[k]);
        }
        // edge
        int edgey2 = image.rows-1;
        for( int k = 0; k < 2; k++) distmap_update_pixel(edgey2,x,TEMPL_LEFT[k]);
    }

    // DOWN SWEEP
    for( int y = 1; y < image.rows; y++)
    {
        // left edge
        int x = 0;
        for( int k = 1; k < 3; k++) distmap_update_pixel(y,x,TEMPL_DOWN[k]);
        // bulk
        for( int x = 1; x <= image.cols-2; x++ )
        {
            for( int k = 0; k < 3; k++) distmap_update_pixel(y,x,TEMPL_DOWN[k]);
        }
        // right edge
        x = image.cols-1;
        for( int k = 0; k < 2; k++) distmap_update_pixel(y,x,TEMPL_DOWN[k]);
    }

    // UP SWEEP
    for( int y = image.rows-2; y >= 0; y--)
    {
        // left edge
        int x = 0;
        for( int k = 1; k < 3; k++) distmap_update_pixel(y,x,TEMPL_UP[k]);
        // bulk
        for( int x = 1; x <= image.cols-2; x++ )
        {
            for( int k = 0; k < 3; k++) distmap_update_pixel(y,x,TEMPL_UP[k]);
        }
        // right edge
        x = image.cols-1;
        for( int k = 0; k < 2; k++) distmap_update_pixel(y,x,TEMPL_UP[k]);
    }

    // DIFFUSION STEPS

    int n_steps_before = 4;
    int n_steps_shrinking = 6;
    int n_steps_after = 0;


    int step = 1;
    for (   ; step <= n_steps_before; step++)
        diffuse();
    for (   ; step <= n_steps_before + n_steps_shrinking; step++) 
        diffuse( (1- step/float(n_steps_shrinking)) );
    for ( int j = step; j < step + n_steps_after; j++)
        diffuse( (1- step/float(n_steps_shrinking)) );

    // diffuse(3., 3.);
    // diffuse(1., 1.);
    



 

    // std::cerr     <<"rows "<< image.rows << ", cols "<<image.cols<<std::endl;





    cv::imwrite( "cxxout.png", image );
    cv::namedWindow("Display Image", cv::WINDOW_AUTOSIZE );
    cv::imshow("Display Image", image);
    cv::waitKey(0);
    cv::destroyAllWindows();

    return 0;
}



