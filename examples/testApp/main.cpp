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
    
    FirewireVideo video = FirewireVideo();
    
    video.SetAllFeaturesAuto();
    //video.PrintCameraReport();
    video.SetMetaDataFlags( 0 );
    video.SetHDRRegister(false);
    
    video.LoadConfig();
    
    cout << video.GetConfigValue("HDR_TMO") << endl;
    cout << video.GetConfigValue("HDR_FORMAT") << endl;
    cout << video.GetConfigValue("VIDEO_FORMAT") << endl;
    
    return 0;
}

