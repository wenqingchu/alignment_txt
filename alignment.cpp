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
    //resize(LP, LP, Size(250, 100));
    LicensePlate test(LP);
    finalimg = test.LPRun();
    cout << "horizen angle: " << test.horizen_record << endl;
    cout << "vertical angle: " << test.vertical_record << endl;
    return finalimg;
}

int main(int argc, char *argv[])
{
  clock_t start = clock(); 
    Mat finalimage;
    if (argc<2)
    {
        finalimage = handle("11.jpg");
        imwrite("output.jpg", finalimage);
    }
    clock_t stop = clock();
    //cout << double(1.0*(stop-start)/CLOCKS_PER_SEC) << endl;
    return 0;
}
