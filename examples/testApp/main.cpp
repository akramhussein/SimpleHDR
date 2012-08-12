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
                                        10
                                        );
    video.SetAllFeaturesAuto();
    video.CreateShutterMaps();
    video.SetMetaDataFlags( META_ALL );
    video.SetHDRRegister(false);
    video.Stop();
    
    /*
    uint32_t s0 = video.GetShutterMapQuant(0.00450313);
    uint32_t s1 = video.GetShutterMapQuant(0.00200000);
    uint32_t s2 = video.GetShutterMapQuant(0.00450313);
    uint32_t s3 = video.GetShutterMapQuant(0.00200000);
    
    video.SetHDRShutterFlags(s0,s1,s2,s3);
    */
    
    unsigned char* img = new unsigned char[video.SizeBytes()];
     
    
    /*-----------------------------------------------------------------------
     *  CAPTURE
     *-----------------------------------------------------------------------*/     

    cout << "Camera transmission starting..." << endl;
    // uint32_t ss0,ss1,ss2,ss3;
    
    //video.FlushDMABuffer();
    //video.StopForOneShot();
    video.FlushDMABuffer();
    // loop until quit
    for(int frame_number=0; frame_number <50 ; ++frame_number)
    {     
        cout << endl;
        int shut[4] = {400,1000,400,1000};
        //video.GrabNFrames(img, 4, shut);
        video.GrabNFramesMulti(img,4,shut);

        
    }
    
    delete[] img;
    
    return 0;
}

