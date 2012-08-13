/**
 * @author  Akram Hussein
 * Copyright (C) 2012  Akram Hussein
 *                     Imperial College London
 **/

#include <sys/stat.h>
#include <iostream>

#include <pangolin/pangolin.h>
#include <pangolin/video.h>
#include <pangolin/video/firewire.h>

#include <boost/thread.hpp>  

using namespace pangolin;
using namespace std;

int main( int argc, char* argv[] )
{
    /*-----------------------------------------------------------------------
     *  SETUP SOURCE
     *-----------------------------------------------------------------------*/    
    
    FirewireVideo video = FirewireVideo(0,
                                        DC1394_VIDEO_MODE_640x480_RGB8,
                                        DC1394_FRAMERATE_30,
                                        DC1394_ISO_SPEED_400,
                                        100
                                        );
    video.SetAllFeaturesAuto();
    video.SetMetaDataFlags( META_ALL );
    video.SetHDRRegister(false);
    
    /*
    video.CreateShutterMaps();
       
    video.Stop();
    */
    //unsigned char* img = new unsigned char[video.SizeBytes()];
    // float shut[4] = {0.03331, 0.008, 0.03331, 0.008};
    /*-----------------------------------------------------------------------
     *  CAPTURE
     *-----------------------------------------------------------------------*/     
    
    cout << "Camera transmission starting..." << endl;

    //video.FlushDMABuffer();
    
    uint32_t g0,g1,g2,g3;
    
    video.GetHDRGainFlags(g0, g1, g2, g3);
    
    /*
    // loop until quit
    for(int frame_number=0; frame_number < 1; ++frame_number)
    {     
        video.GrabNFramesMulti(img,10,shut);
        cout << endl;
    }
    
    delete[] img;
    */
    return 0;
}

