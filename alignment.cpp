#include "LP.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <time.h>
using namespace std;

Mat handle(string gx_name)
{
	
    Mat finalimg;
    Mat LP = InputImage(gx_name);
    resize(LP, LP, Size(250, 100));
    LicensePlate test(LP);
    finalimg=test.LPRun();
    resize(finalimg, finalimg, Size(235, 70));
    return finalimg;
}

int main(int argc, char *argv[])
{
    Mat finalimage;
    if (argc<2)
    {
        finalimage = handle("ttt.jpg");
        imwrite("output.jpg", finalimage);
    }
    return 0;
}
