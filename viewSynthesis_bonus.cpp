#include "bmp.h"		//	Simple .bmp library
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

#define Baseline 30.0
#define Focal_Length 100
#define Image_Width 35.0
#define Image_Height 35.0
#define Resolution_Row 512
#define Resolution_Col 512
#define View_Grid_Row 9
#define View_Grid_Col 9

struct Point3d
{
	double x;
	double y;
	double z;
	Point3d(double x_, double y_, double z_) :x(x_), y(y_), z(z_) {}
};

struct Point2d
{
	double x;
	double y;
	Point2d(double x_, double y_) :x(x_), y(y_) {}
};


int main(int argc, char** argv)
{
	if(argc < 5 || argc > 6)
	{
		cout << "Arguments prompt: viewSynthesis.exe <LF_dir> <X Y Z> OR: viewSynthesis.exe <LF_dir> <X Y Z> <focal_length>" << endl;
		return 0;
	}
	string LFDir = argv[1];
	double Vx = stod(argv[2]), Vy = stod(argv[3]), Vz = stod(argv[4]);//position of the viewpoint
	double targetFocalLen = 100;//target focal length
	if(argc == 6)
	{
		targetFocalLen = stod(argv[5]);
	}
	

	vector<Bitmap> viewImageList;
	//! loading light field views
	for (int i = 0; i < View_Grid_Col*View_Grid_Row; i++)
	{
		char name[128];
		sprintf(name, "/cam%03d.bmp", i+1);
		string filePath = LFDir + name;
		Bitmap view_i(filePath.c_str());
		viewImageList.push_back(view_i);
	}

	Bitmap targetView(Resolution_Col, Resolution_Row);
	cout << "Synthesizing image from viewpoint: (" << Vx << "," << Vy << "," << Vz << ") with focal length: " << targetFocalLen << endl;
	//! resample pixels of the target view one by one
	for (int r = 0; r < Resolution_Row; r++)
	{
		for (int c = 0; c < Resolution_Col; c++)
		{
			Point3d rayRGB(0, 0, 0);
			//! resample the pixel value of this ray: TODO
            
            if(targetFocalLen == Focal_Length)//no need to bilinear interpolate pixels at one viewpoint
            {
                int x, y;
                double a, b;
                
                double X, Y;
                X = Vx + (Vz*(2*Image_Width*c-Image_Width*(Resolution_Col-1)))/(double)(2*Resolution_Col)/(double)(targetFocalLen);
                Y = Vy + (Vz*(2*Image_Height*r-Image_Height*(Resolution_Row-1)))/(double)(2*Resolution_Row)/(double)(targetFocalLen);
                
                if(X < (Baseline*(View_Grid_Col-1)/2.0) && X>=-(Baseline*(View_Grid_Col-1)/2.0) && Y < =(Baseline*(View_Grid_Row-1)/2.0) && Y > -(Baseline*(View_Grid_Row-1)/2.0 ))
                {
                    
                    x = (int)(X + (Baseline*(View_Grid_Col-1)/2.0))/Baseline;
                    y = (int)((Baseline*(View_Grid_Row-1)/2.0) - Y)/Baseline;

                    a = (X + (Baseline*(View_Grid_Col-1)/2.0) - (x * Baseline))/(double)Baseline;
                    b = ((Baseline*(View_Grid_Row-1)/2.0) - Y - (y * Baseline))/(double)Baseline;
      
                    unsigned char red1, green1, blue1;
                    unsigned char red2, green2, blue2;
                    unsigned char red3, green3, blue3;
                    unsigned char red4, green4, blue4;
      
                    viewImageList[View_Grid_Col * y + x].getColor(c, r, red1, green1, blue1);
                    viewImageList[View_Grid_Col * y + (x+1)].getColor(c, r, red2, green2, blue2);
                    viewImageList[View_Grid_Col * (y+1) + x].getColor(c, r, red3, green3, blue3);
                    viewImageList[View_Grid_Col * (y+1) + (x+1)].getColor(c, r, red4, green4, blue4);
      
                    rayRGB.x = (double)((1-b) * ((1-a) * red1 + a * red2) + b * ((1-a) * red3 + a * red4));
                    rayRGB.y = (double)((1-b) * ((1-a) * green1 + a * green2) + b * ((1-a) * green3 + a * green4));
                    rayRGB.z = (double)((1-b) * ((1-a) * blue1 + a * blue2) + b * ((1-a) * blue3 + a * blue4));
                }
            }
            else
            {
                int x, y;
                double a, b, X, Y;
                //the position of the intersection
                X = Vx + (Vz*(2*Image_Width*c-Image_Width*(Resolution_Col-1)))/(double)(2*Resolution_Col)/(double)(targetFocalLen);
                Y = Vy + (Vz*(2*Image_Height*r-Image_Height*(Resolution_Row-1)))/(double)(2*Resolution_Row)/(double)(targetFocalLen);
                              
                x = (int)(X + (Baseline*(View_Grid_Col-1)/2.0))/Baseline;
                y = (int)((Baseline*(View_Grid_Row-1)/2.0) - Y)/Baseline;

                // alpha and beta of the position of the intersection
                a = (X + (Baseline*(View_Grid_Col-1)/2.0) - (x * Baseline))/(double)Baseline;
                b = ((Baseline*(View_Grid_Row-1)/2.0) - Y - (y * Baseline))/(double)Baseline;
                
                int ci, ri;
                double u, v, alpha, beta;
                u = (double)(2*Image_Width*c-Image_Width*(Resolution_Col-1))*Focal_Length/(double)(2*Resolution_Col)/(double)targetFocalLen + Image_Width/2.0;// the position of the pixel
                v = (double)(2*Image_Height*r-Image_Height*(Resolution_Row-1))*Focal_Length/(double)(2*Resolution_Row)/(double)targetFocalLen + Image_Height/2.0;

                ci = (int)((double)((2*Resolution_Col) * u - Image_Width))/(double)(2*Image_Width);
                ri = (int)((double)((2*Resolution_Row) * v - Image_Height))/(double)(2*Image_Height);
                
                alpha = ((2*Resolution_Col) * u - ((2*Image_Width) * ci + Image_Width)) / (double)(2*Image_Width);
                beta = 1 - ((2*Resolution_Row) * v - (2*Image_Height * ri + Image_Height)) / (double)(2*Image_Height);
                
                if(X < (Baseline*(View_Grid_Col-1)/2.0) && X>=-(Baseline*(View_Grid_Col-1)/2.0) && Y < =(Baseline*(View_Grid_Row-1)/2.0) && Y > -(Baseline*(View_Grid_Row-1)/2.0 ) && ci>=0 && ci<(Resolution_Col-1) && ri>=0 && ri<(Resolution_Row-1))
                {
                    unsigned char red1, green1, blue1;// four rays of neighbour viewpoints
                    unsigned char red2, green2, blue2;
                    unsigned char red3, green3, blue3;
                    unsigned char red4, green4, blue4;
                    
                    for(int i = 0; i<2;i++)
                    {
                        for(int j = 0; j<2; j++)
                        {
                            unsigned char r1, g1, b1;
                            unsigned char r2, g2, b2;
                            unsigned char r3, g3, b3;
                            unsigned char r4, g4, b4;
                            
                            viewImageList[View_Grid_Col * (y+i) + (x+j)].getColor(ci, ri+1, r1, g1, b1);
                            viewImageList[View_Grid_Col * (y+i) + (x+j)].getColor(ci+1, ri+1, r2, g2, b2);
                            viewImageList[View_Grid_Col * (y+i) + (x+j)].getColor(ci, ri, r3, g3, b3);
                            viewImageList[View_Grid_Col * (y+i) + (x+j)].getColor(ci+1, ri, r4, g4, b4);
                            
                            if(i == 0 && j ==0)
                            {
                                red1 = (double)((1-beta) * ((1-alpha) * r1 + alpha * r2) + beta * ((1-alpha) * r3 + alpha * r4));
                                green1 = (double)((1-beta) * ((1-alpha) * g1 + alpha * g2) + beta * ((1-alpha) * g3 + alpha * g4));
                                blue1 = (double)((1-beta) * ((1-alpha) * b1 + alpha * b2) + beta * ((1-alpha) * b3 + alpha * b4));
                            }
                            else if(i == 0 && j == 1)
                            {
                                red2 = (double)((1-beta) * ((1-alpha) * r1 + alpha * r2) + beta * ((1-alpha) * r3 + alpha * r4));
                                green2 = (double)((1-beta) * ((1-alpha) * g1 + alpha * g2) + beta * ((1-alpha) * g3 + alpha * g4));
                                blue2 = (double)((1-beta) * ((1-alpha) * b1 + alpha * b2) + beta * ((1-alpha) * b3 + alpha * b4));
                            }
                            else if(i ==1 && j == 0)
                            {
                                red3 = (double)((1-beta) * ((1-alpha) * r1 + alpha * r2) + beta * ((1-alpha) * r3 + alpha * r4));
                                green3 = (double)((1-beta) * ((1-alpha) * g1 + alpha * g2) + beta * ((1-alpha) * g3 + alpha * g4));
                                blue3 = (double)((1-beta) * ((1-alpha) * b1 + alpha * b2) + beta * ((1-alpha) * b3 + alpha * b4));
                            }
                            else
                            {
                                red4 = (double)((1-beta) * ((1-alpha) * r1 + alpha * r2) + beta * ((1-alpha) * r3 + alpha * r4));
                                green4 = (double)((1-beta) * ((1-alpha) * g1 + alpha * g2) + beta * ((1-alpha) * g3 + alpha * g4));
                                blue4 = (double)((1-beta) * ((1-alpha) * b1 + alpha * b2) + beta * ((1-alpha) * b3 + alpha * b4));
                            }
                        }
                    }
                    
                    rayRGB.x = (double)((1-b) * ((1-a) * red1 + a * red2) + b * ((1-a) * red3 + a * red4));
                    rayRGB.y = (double)((1-b) * ((1-a) * green1 + a * green2) + b * ((1-a) * green3 + a * green4));
                    rayRGB.z = (double)((1-b) * ((1-a) * blue1 + a * blue2) + b * ((1-a) * blue3 + a * blue4));
                }
            }
            
            //! record the resampled pixel value
			targetView.setColor(c, r, (unsigned char) rayRGB.x, (unsigned char) rayRGB.y, (unsigned char) rayRGB.z);
		}
	}
    
	string savePath = "newView.bmp";
	targetView.save(savePath.c_str());
	cout << "Result saved!" << endl;
    
    printf("do you want to generate a html with 3D display? [y/n]\n");
    char input;
    scanf("%c", &input);
    if(input == 'c')
    {
        for(int new_X = -120; new_X<120; new_X += 10)
        {
            for(int new_Y = -110; new_Y<=120; new_Y += 10)
            {
                Bitmap newTargetView(Resolution_Col, Resolution_Row);
                
                for (int r = 0; r < Resolution_Row; r++)
                {
                    for (int c = 0; c < Resolution_Col; c++)
                    {
                        Point3d rayRGB(0, 0, 0);
                        //! resample the pixel value of this ray: TODO

                        int x, y;
                        double a, b;
                            
                        x = (int)(new_X + 120)/Baseline;
                        y = (int)(120 - new_Y)/Baseline;

                        a = (new_X + 120 - (x * Baseline))/(double)Baseline;
                        b = (120 - new_Y - (y * Baseline))/(double)Baseline;
                  
                        unsigned char red1, green1, blue1;
                        unsigned char red2, green2, blue2;
                        unsigned char red3, green3, blue3;
                        unsigned char red4, green4, blue4;
                  
                        viewImageList[9 * y + x].getColor(c, r, red1, green1, blue1);
                        viewImageList[9 * y + (x+1)].getColor(c, r, red2, green2, blue2);
                        viewImageList[9 * (y+1) + x].getColor(c, r, red3, green3, blue3);
                        viewImageList[9 * (y+1) + (x+1)].getColor(c, r, red4, green4, blue4);
                  
                        rayRGB.x = (double)((1-b) * ((1-a) * red1 + a * red2) + b * ((1-a) * red3 + a * red4));
                        rayRGB.y = (double)((1-b) * ((1-a) * green1 + a * green2) + b * ((1-a) * green3 + a * green4));
                        rayRGB.z = (double)((1-b) * ((1-a) * blue1 + a * blue2) + b * ((1-a) * blue3 + a * blue4));
                        
                        newTargetView.setColor(c, r, (unsigned char) rayRGB.x, (unsigned char) rayRGB.y, (unsigned char) rayRGB.z);
                    
                    //! record the resampled pixel value
                    }
                }
                
                string savePath = "views/" + std::to_string(24 * (new_X+120)/10 + (new_Y+110)/10) + ".bmp";
                newTargetView.save(savePath.c_str());
                
            }
        }
      
        FILE * html_file = fopen("synthetic_views.html", "w");
        fprintf(html_file, "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"utf-8\">\n<title>synthetic views</title>\n<script src=\"https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js\">\n</script>\n<script>\n");
        for(int k=0; k<576; k++)
        {
            string jquery = "$(document).ready(function(){$(\"#button"+std::to_string(k)+"\").hover(function(){$(\"#"+std::to_string(k)+"\").css(\"display\",\"block\");},function(){$(\"#"+std::to_string(k)+"\").css(\"display\",\"none\");});});";
            fprintf(html_file, "%s",jquery.c_str());
        }
        fprintf(html_file, "</script>\n</head>\n<body>\n<pre>\n");
        for(int n = 23; n>=0; n--)
        {
            for(int m =0; m < 24;m++)
            {
                string button = "<a id=\"button" + std::to_string(m * 24 + n) + "\" style=\"background-color:rgb(127,127,127);\">&nbsp;&nbsp;</a>";
                fprintf(html_file,"%s", button.c_str());
            }
            fprintf(html_file, "\n");
        }
        fprintf(html_file, "</pre>\n");
        for(int k = 0; k<576; k++)
        {
            string img = "<img id=\"" + std::to_string(k) + "\" src=\"views\\" + std::to_string(k) + ".bmp\" style=\"display:none;\"></img>\n";
            fprintf(html_file, "%s", img.c_str());
        }
        fprintf(html_file, "</body>\n</html>\n");
    }
    
            

	return 0;
}
