#include <sys/stat.h>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <limits>

#include <pangolin/pangolin.h>
#include <pangolin/video.h>
#include <pangolin/video/firewire.h>

#include <boost/thread/thread.hpp>

using namespace pangolin;
using namespace std;

int main( int argc, char* argv[] ){

    FirewireVideo video = FirewireVideo();     
    video.PrintCameraReport();
    video.GetResponseFunction();
    
    return 0;
}
