#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include "opencv2/features2d.hpp"
#include <iostream>
#include <vector>
#include <fstream>

using namespace std;
using namespace cv;
const float inlier_threshold = 2.5f; // Distance threshold to identify inliers
const float nn_match_ratio = 0.8f;   // Nearest neighbor matching ratio

class maxims
{
public:
    double per;
    string name;
    maxims(double p,string n)
    {
        per=p;
        name=n;
    }
};
bool myfunction (maxims i,maxims j) { return (i.per>j.per); }
int Plant(Mat &src,Mat &image,int l,int u,double change,double percent)
{
    Mat hsv;
    int x=0;
    //resize(src,image,Size(500,500));
    cvtColor(image,hsv,COLOR_BGR2HSV);
    Scalar lower(l,15,0),upper(u,255,255);
    inRange(hsv,lower,upper,hsv);

    Size s(image.rows/64,image.cols/64);
    double im_area=image.rows*image.cols;
    if(im_area<100000)
        s=Size(10,10);
    Mat kernel = getStructuringElement(MORPH_ELLIPSE,s);
    //morphologyEx(hsv,hsv,MORPH_OPEN,kernel);
    erode(hsv,hsv,kernel);
    cvtColor(hsv,hsv,COLOR_GRAY2BGR);
    Mat image2;
    bitwise_and(image,hsv,image2);
    Mat gray;
    cvtColor(image2,gray,COLOR_BGR2GRAY);
    Canny(gray,gray,change*10,change*30);
    morphologyEx(gray,gray,MORPH_CLOSE,kernel);
    morphologyEx(gray,gray,MORPH_OPEN,kernel);
    vector<vector<Point>> contours;
    findContours(gray,contours,CV_RETR_TREE,CV_CHAIN_APPROX_NONE);
    double area=0;
    int k=-1;
    vector<Point> bounding_contour;
    for(int i=0;i<contours.size();i++)
    {
        double d=contourArea(contours[i]);
        Moments mu = moments(contours[i]);
        Point centroid = Point (mu.m10/mu.m00 , mu.m01/mu.m00);
        if(d>im_area*percent)
            {
                area+=d;
                bounding_contour.insert(bounding_contour.end(),contours[i].begin(),contours[i].end());
            }
        else if(d>im_area*0.01)
        {
            x=-1;
        }
    }
    Rect r=boundingRect(bounding_contour);
    if(r.width*r.height>=im_area*percent)
    {
        x=1;
    }
    //rectangle(image,Point(r.x,r.y),Point(r.x+r.width,r.y+r.height),Scalar(0,0,255));
    resize(image,image,src.size());
    if(r.area()!=0)
        image = Mat(image,r);
    return x;
}

Mat fouriers(Mat I)
{

    Mat padded;                            //expand input image to optimal size
    int m = getOptimalDFTSize( I.rows );
    int n = getOptimalDFTSize( I.cols ); // on the border add zero values
    copyMakeBorder(I, padded, 0, m - I.rows, 0, n - I.cols, BORDER_CONSTANT, Scalar::all(0));

    Mat planes[] = {Mat_<float>(padded), Mat::zeros(padded.size(), CV_32F)};
    Mat complexI;
    merge(planes, 2, complexI);         // Add to the expanded another plane with zeros

    dft(complexI, complexI);            // this way the result may fit in the source matrix

    // compute the magnitude and switch to logarithmic scale
    // => log(1 + sqrt(Re(DFT(I))^2 + Im(DFT(I))^2))
    split(complexI, planes); // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
    Mat magI,phI;
    magnitude(planes[0], planes[1], magI);// planes[0] = magnitude
    //Mat magI = planes[0];

    magI += Scalar::all(1);                    // switch to logarithmic scale
    log(magI, magI);

    // crop the spectrum, if it has an odd number of rows or columns
    magI = magI(Rect(0, 0, magI.cols & -2, magI.rows & -2));

    // rearrange the quadrants of Fourier image  so that the origin is at the image center
    int cx = magI.cols/2;
    int cy = magI.rows/2;

    Mat q0(magI, Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant
    Mat q1(magI, Rect(cx, 0, cx, cy));  // Top-Right
    Mat q2(magI, Rect(0, cy, cx, cy));  // Bottom-Left
    Mat q3(magI, Rect(cx, cy, cx, cy)); // Bottom-Right

    Mat tmp;                           // swap quadrants (Top-Left with Bottom-Right)
    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);

    q1.copyTo(tmp);                    // swap quadrant (Top-Right with Bottom-Left)
    q2.copyTo(q1);
    tmp.copyTo(q2);

    normalize(magI, magI, 0, 1, CV_MINMAX); // Transform the matrix with float values into a
                                            // viewable image form (float between values 0 and 1).

    return magI;
}
float func(Mat src)
{
     float data[3][3];
     double sum_max=0;
     float t;
     Mat kernel(3,3,CV_32FC1,&data),tmp;
     for(float theta=0;theta<CV_PI*2;theta+=0.01)
     {
         for(int i=-1;i<2;i++)
         {
             for(int j=-1;j<2;j++)
             {
                 data[i+1][j+1]=i*sin(theta)+j*cos(theta);
             }
         }
         filter2D(src,tmp,-1,kernel);
         Scalar x=sum(tmp);
         if(x[0]>sum_max)
         {sum_max=x[0];
          t=theta;
         }
     }
      for(int i=-1;i<2;i++)
     for(int j=-1;j<2;j++)
             {
                 data[i+1][j+1]=i*sin(t)+j*cos(t);
             }
     filter2D(src,tmp,-1,kernel);
     imshow("",tmp);
     return t;

}

Mat rot(Mat& im,double thetaRad)
{
    cv::Mat rotated;
    double rskew = thetaRad* CV_PI/180;
    double nw = abs(sin(thetaRad))*im.rows+abs(cos(thetaRad))*im.cols;
    double nh = abs(cos(thetaRad))*im.rows+abs(sin(thetaRad))*im.cols;
    cv::Mat rot_mat = cv::getRotationMatrix2D(Point2d(nw*.5,nh*.5), thetaRad*180/CV_PI, 1);
    Mat pos = Mat::zeros(Size(1,3),CV_64FC1);
    pos.at<double>(0)=(nw-im.cols)*.5;
    pos.at<double>(1)=(nh-im.rows)*.5;
    Mat res = rot_mat*pos;
    rot_mat.at<double>(0,2) += res.at<double>(0);
    rot_mat.at<double>(1,2) += res.at<double>(1);
    cv::warpAffine(im, rotated, rot_mat,Size(nw,nh), cv::INTER_LANCZOS4);
    return rotated;
}
Mat Uni_hist(Mat rgb,Mat mask,int ch)
{
    Mat hsv;
    cvtColor(rgb,hsv,COLOR_BGR2HSV);
    vector<Mat> hsv_split;
    split(hsv,hsv_split);
    Mat hist;
    int histSize = 256;
    float range[] = { 0, 256 } ;
    const float* histRange = { range };

    calcHist(&hsv_split[ch],1,0,mask,hist,1,&histSize,&histRange);
    return hist;
}
double tiled_compare(Mat hist,Mat src,int tile_size,double thresh)
{
    Rect roi;
    Mat submat;
    Mat submat2;
    Mat tmp_hist;
    Mat mask;
    double d=0;
    int k=1;
    double t=1;
    for(int i=0;i<src.rows-tile_size;i+=tile_size)
    {
        for(int j=0;j<src.cols-tile_size;j+=tile_size)
        {
           roi=Rect(j,i,tile_size,tile_size);
           submat=Mat(src,roi);
           tmp_hist = Uni_hist(submat,Mat(),0);
           t=1-compareHist(hist,tmp_hist,CV_COMP_BHATTACHARYYA);
           if(t>thresh)
              {d+=t;}
              k++;
        }
    }
    d=d/k;
    return d;
}
double match(Mat img,Mat dis)
{
    double a=img.rows*img.cols;
    double b=dis.rows*dis.cols;
    resize(dis,dis,Size(50,50));
    if(a<b*8)
    {
        pyrUp(img,img);
        a=img.rows*img.cols;
    }
    Mat hist=Uni_hist(dis,Mat(),0);
    return tiled_compare(hist,img,50,0.5);
}

vector<maxims> disease(Mat img,vector<string> path)
{
    Mat dis[path.size()];
    string path_to="Diseases/";
    double d=0;
    vector<maxims> m;
    for(int i=0;i<path.size();i++)
    {
        dis[i]=imread(path_to+path[i]);
        d=match(img,dis[i]);
        if(d>0.2)
            {
                string s=path[i];
                string str;
                int j;
                for(j=0;s[j]!='_';j++)
                    str += s[j];
                m.push_back(maxims(d,str));
            }
    }
    sort(m.begin(),m.end(),myfunction);
    return m;
}
vector<maxims> similar(vector<maxims> m)
{
    for(int i=0;i<m.size();i++)
    {

    }
}
int main(int argc,char **args)
{
    Mat img=imread(argc<2?"plant.jpg":args[1]);
    int a=Plant(img,img,15,70,1,0.05);
    imshow("image",img);
    if(a)
    {system("cd Diseases && dir>>temp.txt");
    fstream f;
    f.open("Diseases/temp.txt");
    string s;
    vector<string> path;
    int i=0;
    while(f)
    {
        f>>s;
        if(s.find(".jpg")!=string::npos)
            {
                path.push_back(s);
            }
        i++;
    }
    vector<maxims> m = disease(img,path);
    cout<<"Most probabale detection : "<<m[0].name<<" with probability : "<<m[0].per;
    remove("Diseases/temp.txt");
    waitKey();
    }
    else cout<<"Does not seems like a plant";
}

