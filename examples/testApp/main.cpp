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
    //video.PrintCameraReport();
    video.SetMetaDataFlags( 0 );
    video.SetHDRRegister(false);
        
    float exposure_tick = (video.GetFeatureValueMax(DC1394_FEATURE_SHUTTER) - video.GetFeatureValueMin(DC1394_FEATURE_SHUTTER)) / (video.GetFeatureQuantMax(DC1394_FEATURE_SHUTTER)+1);
    
    return 0;
}

