/*
 * Playground file for testing in Pangolin
 *
 */

#include <sys/stat.h>
#include <iostream>

#include <pangolin/pangolin.h>
#include <pangolin/video.h>
#include <pangolin/video/firewire.h>

using namespace pangolin;
using namespace std;

int main( int argc, char* argv[] )
{
    FirewireVideo video = FirewireVideo();   

    uint32_t s0,s1,s2,s3;
    uint32_t shut0,shut1,shut2,shut3;
  
    video.SetHDRRegister(true);

    cout << hex << video.GetHDRFlags() << endl;

    s0 = 10;
    s1 = 100;
    s2 = 1000;
    s3 = 1111;
    
    video.SetHDRShutterFlags(s0,s1,s2,s3);
    
    video.GetHDRShutterFlags(shut0,shut1,shut2,shut3);
    
    cout << "shut0: " << shut0 << endl;
    cout << "shut1: " << shut1 << endl;
    cout << "shut2: " << shut2 << endl;
    cout << "shut3: " << shut3 << endl;
    
    video.SetHDRRegister(false);
    
    return 0;
    
}