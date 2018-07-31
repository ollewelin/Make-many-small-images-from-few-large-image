//This code take some 0.JPG files in program root dir and tranform images to a folder with name \data\0.JPG

//#include <stdio.h>
//#include <unistd.h>
//#include <ctime>
//#include <iostream>
//#include <raspicam/raspicam_cv.h>

#include <opencv2/highgui/highgui.hpp>  // OpenCV window I/O
#include <opencv2/imgproc/imgproc.hpp> // Gaussian Blur
#include <stdio.h>
//#include <raspicam/raspicam_cv.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>        // Basic OpenCV structures (cv::Mat, Scalar)
#include <cstdlib>
#include <ctime>
#include <math.h>  // exp
#include <stdlib.h>// exit(0);
#include <iostream>


using namespace std;
using namespace cv;

//default capture width and height
//const int FRAME_WIDTH = 640;
//const int FRAME_HEIGHT = 480;
int FRAME_WIDTH = 32;
int FRAME_HEIGHT = 32;

const string WindowName = "Image viewer";

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

/* ========== resize ==============
Size size(100,100);//the dst image size,e.g.100x100
Mat dst;//dst image
Mat src;//src image
resize(src,dst,size);//resize image
*/

/* ======== Rect========
  Mat rect_part(image, Rect(rand_x_start, rand_y_start, Width, Height));//Pick a small part of image

        max_point_x_or_y = image.cols - (Width*2);//Not allowed to set start x point so the small clip out rectangel can go outside the camera source image
        rand_x_start = (int) (rand() % max_point_x_or_y);// range
        max_point_x_or_y = image.rows - (Height*2);//Not allowed to set start y point so the small clip out rectangel can go outside the camera source image
        rand_y_start = (int) (rand() % max_point_x_or_y);// range
        //     printf("rand_x_start %d\n", rand_x_start);
        //     printf("rand_y_start %d\n", rand_y_start);
        Mat cam_part2(image, Rect(rand_x_start, rand_y_start, Width*2, Height*2));//Pick a small part of image and feed autoencoder with this

*/

Mat transf(Mat src)
{
   Point2f srcTri[3];
   Point2f dstTri[3];

    float scale_r;
    float scale_r_n;
    float scale_gain=0.0f;///Change here
    float trans_x_r;
    float trans_y_r;
    float transelation_x = 0.0f;
    float transelation_y = 0.0f;
    float x_gain = 0.0f;///Change here
    float y_gain = 0.0f;///Change here
    float rot_max_min = 0.0f;///Change here
    float rot_random =0.0f;

 //   srand (static_cast <unsigned> (time(0)));//Seed the randomizer
    scale_r = (float) (rand() % 65535) / 65536;
    printf("scale_r %f\n", scale_r);
    trans_x_r = (float) (rand() % 65535) / 65536;
    printf("trans_x_r %f\n", trans_x_r);
    trans_y_r = (float) (rand() % 65535) / 65536;
    printf("trans_y_r %f\n", trans_y_r);
    rot_random = (float) (rand() % 65535) / 65536;
 //   printf("rot_random %f\n", rot_random);

    scale_r = 0.5f + scale_r;
    printf("scale_r %f\n", scale_r);
    scale_r_n = 1.0f - scale_r;


transelation_x = (trans_x_r - 0.5f) * x_gain;
transelation_y = (trans_y_r - 0.5f) * y_gain;

    scale_r = ((scale_r - 1.0f) * scale_gain) + 1.0f;
    scale_r_n = scale_r_n * scale_gain;
    printf("scale_r_n %f\n", scale_r_n);

   Mat rot_mat( 2, 3, CV_8UC1 );
   Mat warp_mat( 2, 3, CV_8UC1 );
   Mat warp_dst, warp_rotate_dst;
   /// Set the dst image the same type and size as src
   warp_dst = Mat::zeros( src.rows, src.cols, src.type() );
   /// Set your 3 points to calculate the  Affine Transform
   srcTri[0] = Point2f( 0,0 );
   srcTri[1] = Point2f( src.cols - 1, 0 );
   srcTri[2] = Point2f( 0, src.rows - 1 );

//2:1
   dstTri[0] = Point2f( src.cols*0.25, src.rows*0.25 );
   dstTri[1] = Point2f( src.cols*0.75, src.rows*0.25 );
   dstTri[2] = Point2f( src.cols*0.25, src.rows*0.75 );


   /// Get the Affine Transform
   warp_mat = getAffineTransform( srcTri, dstTri );

   /// Apply the Affine Transform just found to the src image
   warpAffine( src, warp_dst, warp_mat, warp_dst.size() );


//   dstTri[0] = Point2f( src.cols*(0.25+transelation_x), src.rows*0.25 );
//   dstTri[1] = Point2f( src.cols*(0.75+transelation_x), src.rows*0.25 );
//   dstTri[2] = Point2f( src.cols*(0.25+transelation_x), src.rows*0.75 );

//   dstTri[0] = Point2f( src.cols*(0.0+transelation_x), src.rows*(0.0+transelation_y) );
//   dstTri[1] = Point2f( src.cols*(1.0+transelation_x), src.rows*(0.0+transelation_y) );
//   dstTri[2] = Point2f( src.cols*(0.0+transelation_x), src.rows*(1.0+transelation_y) );

   dstTri[0] = Point2f( src.cols*(scale_r_n+transelation_x), src.rows*(scale_r_n+transelation_y) );
   dstTri[1] = Point2f( src.cols*(scale_r+transelation_x), src.rows*(scale_r_n+transelation_y) );
   dstTri[2] = Point2f( src.cols*(scale_r_n+transelation_x), src.rows*(scale_r+transelation_y) );

    /// Get the Affine Transform
   warp_mat = getAffineTransform( srcTri, dstTri );

   /// Do the Affine Transformation scale and translation
   warpAffine( src, warp_dst, warp_mat, warp_dst.size() );

//================== Rotation ==============================
   /** Rotating the image after Warp */
    rot_random = rot_random - 0.5f;// Make it to a random value of +/- 0.5
    rot_random = rot_random * rot_max_min;
    printf("rot_random %f\n", rot_random);

   /// Compute a rotation matrix with respect to the center of the image
   Point center = Point( warp_dst.cols/2, warp_dst.rows/2 );
   double angle = 0.0;
   double rot_scale = 1.0;
    angle = (double)rot_random;
   /// Get the rotation matrix with the specifications above
   rot_mat = getRotationMatrix2D( center, angle, rot_scale );

   /// Rotate the warped image
   warpAffine( warp_dst, warp_rotate_dst, rot_mat, warp_dst.size() );
//============= End Rotation ==============================
   imshow("warp_dst", warp_dst);
   imshow("warp_rotate_dst", warp_rotate_dst);
   imshow("src", src);

    return warp_rotate_dst;
}

#include <stdio.h>
#include <sys/stat.h>
void create_a_data_path(void)
{
printf("Create a path ./data/\n");
mkdir("./data/", 0777);
}

int main ()
{
    //JPG File store
    int ret;
   // char ch;
   // int files_number = 2000;
    char filename_src[100];
    char filename_dst[100];
   // int verification=0;
    int transformed_nr=5;
    int nr_of_orginals=0;
  //  float Rando=0.0f;
  //  float trans_x=0;
  //  float trans_y=0;
    float scale=0;
    //-------------
    Mat color_img_ver,color_img_pos,image_pos, image_ver, cloned_img_pos , cloned_img_ver;

create_a_data_path();
const int max_DIV =100;
int div_img=0;
    printf("Enter how many orginal images you have 0...JPG \n");
    scanf("%d", &nr_of_orginals);
    printf("How many orginal images you have enter = %d\n", nr_of_orginals);
    printf("Enter how much you want devide the orginal image 1..%d\n", max_DIV);
    scanf("%d", &div_img);
    printf("div_img = %d\n", div_img);

    if(div_img < 1 || div_img > max_DIV)
{
	printf("Out of range error div_img < 1 || div_img > max_DIV\n");
exit(0);
} 
    srand (static_cast <unsigned> (time(0)));//Seed the randomizer
    
 printf("Try Open 0.JPG files and see what width and height this image have\n");
color_img_pos = imread("0.JPG", 1 );
int input_image_width = color_img_pos.cols;
int input_image_height = color_img_pos.rows;

printf("input_image_width = %d\n", input_image_width); 
printf("input_image_height = %d\n", input_image_height); 
if(input_image_height < 1 || input_image_width < 1)
{
printf("ERROR Out of range input_image_height or input_image_width <1"); 
exit(0);
}
int FRAME_WIDTH = 0;
int FRAME_HEIGHT = 0;
FRAME_WIDTH = input_image_width / div_img;
printf("FRAME_WIDTH = %d\n", FRAME_WIDTH); 
FRAME_HEIGHT = input_image_height / div_img;
printf("FRAME_HEIGHT = %d\n", FRAME_HEIGHT); 

int nr_ROW = input_image_height / FRAME_HEIGHT;
int nr_COL = input_image_width / FRAME_WIDTH;
int nr_per_orginal = nr_COL *nr_ROW;
printf("Nr of images per orginal image = %d\n", nr_per_orginal);
int random_position = 0;

    for(int org_nr=0; org_nr<nr_of_orginals; org_nr++)
    {
        sprintf(filename_src, "%d.JPG", org_nr);//Assigne a filename "pos" with index number added
        color_img_pos = imread( filename_src, 1 );
        if ( !color_img_pos.data )
        {
            printf("\n");
            printf("==================================================\n");
            printf("No image_pos data Error! Probably not find %d.JPG \n", org_nr);
            printf("==================================================\n");
            printf("\n");
            //return -1;
        }
///Size size(100,100);//the dst image size,e.g.100x100
///Mat dst;//dst image
///Mat src;//src image
///resize(src,dst,size);//resize image
///ratio Iphone photo width:hight = 1.3333..:1.0
///Size size(640,480);//the dst image size,e.g.100x100
Size size(input_image_width,input_image_height);//the dst image size,e.g.100x100

resize(color_img_pos, image_pos, size);
        ///    image_pos = color_img_pos;
            imshow("color_img_pos", color_img_pos);
            imshow("image_pos", image_pos);
          //  m1_cloned = m1.clone();
            cloned_img_pos = image_pos.clone();
        int max_point_x_or_y;
        int rand_x_start;
        int rand_y_start;
 ///       Mat cloned_image_pos_part(FRAME_WIDTH, FRAME_HEIGHT, CV_8UC3);
	int trans_nr=0;
        //for(int trans_nr=0; trans_nr<transformed_nr; trans_nr++)
        //{
	for(int y_img = 0;y_img<nr_ROW;y_img++)
	{
	for(int x_img = 0;x_img<nr_COL;x_img++)
	{
	 

///            if(trans_nr>0)
///            {
if(random_position == 1)
{
                    max_point_x_or_y = cloned_img_pos.cols - (FRAME_WIDTH);//Not allowed to set start x point so the small clip out rectangel can go outside the camera source image
                    rand_x_start = (int) (rand() % max_point_x_or_y);// range
                    max_point_x_or_y = cloned_img_pos.rows - (FRAME_HEIGHT);//Not allowed to set start y point so the small clip out rectangel can go outside the camera source image
                    rand_y_start = (int) (rand() % max_point_x_or_y);// range
                    /// image_pos = transf(cloned_img_pos);
}
else
{
rand_x_start = x_img * FRAME_WIDTH;
rand_y_start = y_img * FRAME_HEIGHT;
if(org_nr<2)
{
printf("rand_x_start = %d\n", rand_x_start);
printf("rand_y_start = %d\n", rand_y_start);
printf("trans_nr = %d\n", trans_nr);
}
}
		
                    Mat image_pos_part(cloned_img_pos, Rect(rand_x_start, rand_y_start, FRAME_WIDTH, FRAME_HEIGHT));//Pick a small part of image
           ///         cloned_image_pos_part = image_pos_part.clone();
///            }
            imshow("image_pos_part",image_pos_part);
         ///   waitKey(1000);
            cv::imwrite("temporary_file.JPG",image_pos_part);
            //--- Save JPG files ----
	    int dst_img_index = (org_nr*nr_per_orginal) + trans_nr;
            sprintf(filename_dst, "./data/%d.JPG", (dst_img_index));
            ret = rename("temporary_file.JPG", filename_dst);
            if(ret == 0)
            {
                printf("File renamed successfully");
            }
            else
            {
                printf("Error: unable to rename the file");
            }
            //---------------------
            //cout<<"image_pos saved at ./x.JPG"<<endl;
            printf("image_pos save a file: %d", dst_img_index);
            printf(".JPG\n" );
            printf("trans_nr %d\n", trans_nr);
            waitKey(1);
trans_nr++;
        }
	}
       waitKey(1);
    }
}



