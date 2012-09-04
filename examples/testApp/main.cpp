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
    FirewireVideo video = FirewireVideo();
    video.SetHDRRegister(false);
    video.SetMetaDataFlags( META_ALL_AND_ABS );
    video.SetAllFeaturesAuto();
    
    unsigned char* img = new unsigned char[video.SizeBytes()];
    
    struct timeval tim;
    
    
    int tests = 1;
    int n = 5;
    double total = 0;
    
    for(int i = 1 ; i <= tests; i++){
        
        bool over = true;
        
        double old_time, new_time, sum = 0;
        
        for(int j = 1 ; j <= n ; j++){
            
            if (over){
                video.SetFeatureQuant(DC1394_FEATURE_SHUTTER, 200);
                over = false;
            } else {
                video.SetFeatureQuant(DC1394_FEATURE_SHUTTER, 600);
                over = true;
            }
            
            video.StopForOneShot();
            video.GrabOneShot(img);
            
            // print frame time gap in micro-seconds
            tim.tv_usec = video.ReadTimeStamp(img);
            new_time = tim.tv_sec+(tim.tv_usec/1000000.0);
            
            if (j != 1){
                //printf("Frame %d to %d: %.6lf micro-seconds \n", i, i-1, new_time - old_time);
                sum += (new_time - old_time);
            }
            cout << j << " " << (new_time - old_time) << endl;
            
            old_time = new_time;
        }
        
        total += ( sum / (n-1) );
        
    }
    cout << "average time gap (micro-seconds): " << total / tests << endl;
    
    return 0;
}

