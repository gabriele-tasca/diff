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


    cv::Mat closest = cv::Mat(ncols, nrows, CV_16UC(2));
    cv::Mat distances = cv::Mat(ncols, nrows, CV_32F);
    
    cv::Mat snapshot;
    snapshot = image.clone();

    cv::Mat_<cv::Vec4b> _image = image;
    cv::Mat_<cv::Vec4b> _snapshot = snapshot;
    cv::Mat_<cv::Vec2i> _closest = closest;
    cv::Mat_<float> _distances = distances;


    static const int TEMPL_DOWN [3][2] = {{-1,-1}, {-1,0}, {-1,1}};
    static const int TEMPL_UP [3][2] = {{1,-1}, {1,0},{1,1}};
    static const int TEMPL_RIGHT [3][2] = {{-1,-1}, {0,-1},{1,-1}};
    static const int TEMPL_LEFT [3][2] = {{-1,1}, {0,1},{1,1}};

    //UPDATE THE 4 
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
    //UPDATE THE 4 



    ///////////////// 
    // FUNCTIONS
    ///////////////// 


    auto update_pixel = [&](int y, int x, const int TEMPL[3][2], int k)
    {
        int cy = y + TEMPL[k][0];
        int cx = x + TEMPL[k][1];
        float newdist = DISTANCE(y,x,_closest(cy,cx)[0],_closest(cy,cx)[1]);
        if (newdist < _distances(y,x)  )
        {
            _image(y,x)[0] = _image(cy,cx)[0];
            _image(y,x)[1] = _image(cy,cx)[1];
            _image(y,x)[2] = _image(cy,cx)[2];
            _image(y,x)[3] = _image(cy,cx)[3];

            _closest(y,x)[0] = _closest(cy,cx)[0]; 
            _closest(y,x)[1] = _closest(cy,cx)[1];
            _distances(y,x) = newdist;
        }
    };

    
    // in-place
    auto diffuse = [&]()
    {
        for( int x = 0; x < image.cols; x++)
        {
            for( int y = 0; y < image.rows; y++ )
            {
                // 3 channels
                for( int chan = 0; chan < 3; chan++)
                {
                    float truerad = _distances(y,x) * 0.92;
                    float newvalue = 0;
                    float count = 0;
                    // look around
                    for( int kappa = 0; kappa < ndiffspots; kappa++)
                    {
                        int cy = int( y + DIFF_UNIT_VECS[kappa][0]*truerad ); 
                        int cx = int( x + DIFF_UNIT_VECS[kappa][1]*truerad );
                        if (cy >= 0 && cy < image.rows && cx >= 0 && cx < image.cols)
                        {
                            newvalue += float(_image(cy,cx)[chan]);
                            count++;
                        }
                    }
                    newvalue = newvalue/count;
                    _image(y,x)[chan] = int(newvalue);
                }
            }
        }
    };

    // not in-place
    auto take_snapshot = [&](){
        snapshot = image.clone();
        cv::Mat_<cv::Vec4b> _snapshot = snapshot;
        // std::cout << "printerino" << std::endl;
        // cv::namedWindow("Display nug", cv::WINDOW_AUTOSIZE );
        // cv::imshow("Display nug", snapshot);
        // std::cout << "printerino2" << std::endl;
    };
    auto diffuse2 = [&]()
    {
        snapshot = image.clone();
        cv::Mat_<cv::Vec4b> _snapshot = snapshot;

        for( int x = 0; x < image.cols; x++)
        {
            for( int y = 0; y < image.rows; y++ )
            {
                // 3 channels
                for( int chan = 0; chan < 3; chan++)
                {
                    float truerad = _distances(y,x) * 0.92;
                    float newvalue = 0;
                    float count = 0;
                    // look around
                    for( int kappa = 0; kappa < ndiffspots; kappa++)
                    {
                        int cy = int( y + DIFF_UNIT_VECS[kappa][0]*truerad ); 
                        int cx = int( x + DIFF_UNIT_VECS[kappa][1]*truerad );
                        if (cy >= 0 && cy < image.rows && cx >= 0 && cx < image.cols)
                        {
                            newvalue += float(_snapshot(cy,cx)[chan]);
                            count++;
                        }
                    }
                    newvalue = newvalue/count;
                    _image(y,x)[chan] = int(newvalue);
                }
            
            
            }
        }

        snapshot = _snapshot;
        
    };


    ///////////////// 
    // MAIN
    ///////////////// 

    // init 
    for( int y = 0; y < image.rows; y++)
    {
        for( int x = 0; x < image.cols; x++ )
        {
            if (_image(y,x)[3] != 0)
            {
                _closest(y,x)[0] = y;
                _closest(y,x)[1] = x;
                _distances(y,x) = 0;

            }
            else
            {
                _distances(y,x) = BIG_FLOAT;
                _closest(y,x)[0] = -1;
                _closest(y,x)[1] = -1;
            }

        }
    }

    // RIGHT SWEEP
    for( int x = 1; x < image.cols; x++)
    {
        // edge
        int y = 0;
        for( int k = 1; k < 3; k++) update_pixel(y,x,TEMPL_RIGHT,k);
        // bulk
        for( int y = 1; y < image.rows -1; y++ )
        {
            for( int k = 0; k < 3; k++) update_pixel(y,x,TEMPL_RIGHT,k);
        }
        // edge
        y = image.cols-1;
        for( int k = 0; k < 2; k++) update_pixel(y,x,TEMPL_RIGHT,k);
    }

    // LEFT SWEEP
    for( int x = image.cols-1; x >= 1; x--)
    {
        // edge
        int y = 0;
        for( int k = 1; k < 3; k++) update_pixel(y,x,TEMPL_LEFT,k);
        // bulk
        for( int y = 1; y < image.rows -1; y++ )
        {
            for( int k = 0; k < 3; k++) update_pixel(y,x,TEMPL_LEFT,k);
        }
        // edge
        y = image.cols-1;
        for( int k = 0; k < 2; k++) update_pixel(y,x,TEMPL_LEFT,k);
    }

    // DOWN SWEEP
    for( int y = 1; y < image.rows; y++)
    {
        // left edge
        int x = 0;
        for( int k = 1; k < 3; k++) update_pixel(y,x,TEMPL_DOWN,k);
        // bulk
        for( int x = 1; x < image.cols -1; x++ )
        {
            for( int k = 0; k < 3; k++) update_pixel(y,x,TEMPL_DOWN,k);
        }
        // right edge
        x = image.cols-1;
        for( int k = 0; k < 2; k++) update_pixel(y,x,TEMPL_DOWN,k);
    }

    // UP SWEEP
    for( int y = image.rows-1; y >= 1; y--)
    {
        // left edge
        int x = 0;
        for( int k = 1; k < 3; k++) update_pixel(y,x,TEMPL_UP,k);
        // bulk
        for( int x = 1; x < image.cols -1; x++ )
        {
            for( int k = 0; k < 3; k++) update_pixel(y,x,TEMPL_UP,k);
        }
        // right edge
        x = image.cols-1;
        for( int k = 0; k < 2; k++) update_pixel(y,x,TEMPL_UP,k);
    }


    diffuse();
    diffuse();
    diffuse();
    diffuse();
    diffuse();
    diffuse();
    diffuse();
    diffuse();









    image = _image;
    closest = _closest;
    distances = _distances;
    

 






    cv::imwrite( "cxxout.png", image );
    cv::namedWindow("Display Image", cv::WINDOW_AUTOSIZE );
    cv::imshow("Display Image", image);
    cv::waitKey(0);
    cv::destroyAllWindows();

    return 0;
}



