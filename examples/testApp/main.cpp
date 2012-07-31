/*
 * Playground file for testing in Pangolin
 *
 */
#include <iostream>

#include <pangolin/pangolin.h>
#include <pangolin/video.h>
#include <pangolin/video/firewire.h>


using namespace pangolin;
using namespace std;

int main( int argc, char* argv[] ){

    FirewireVideo video = FirewireVideo();     
    video.PrintCameraReport();
    video.GetResponseFunction();
    
    return 0;
}
