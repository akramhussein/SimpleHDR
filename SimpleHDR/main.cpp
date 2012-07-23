/**
 * @author  Akram Hussein
 * Copyright (C) 2012  Akram Hussein
 *                     Imperial College London
 **/

#include <pangolin/pangolin.h>
#include <pangolin/video.h>
#include <pangolin/video/firewire.h>

#include <dc1394/dc1394.h>

#include <boost/thread/thread.hpp>

using namespace pangolin;
using namespace std;

int main( int argc, char* argv[] )
{

    // Setup Video Source
    // FirewireVideo video = FirewireVideo(); // Simplified constructor
    FirewireVideo video =  FirewireVideo(
                                          0,
                                          DC1394_VIDEO_MODE_640x480_RGB8,
                                          DC1394_FRAMERATE_30,
                                          DC1394_ISO_SPEED_400,
                                          10
                                         );
    
    VideoPixelFormat vid_fmt = VideoFormatFromString(video.PixFormat());
    const unsigned w = video.Width();
    const unsigned h = video.Height();

    video.PrintCameraReport();
    
    // Create Glut window
    pangolin::CreateGlutWindowAndBind("Main",w,h);

    // Create viewport for video with fixed aspect
    View& vVideo = Display("Video").SetAspect((float)w/h);

    // OpenGl Texture for video frame
    GlTexture texVideo(w,h,GL_RGBA8);

    unsigned char* img = new unsigned char[video.SizeBytes()];
    bool over_exposed = true;
    
    //meta_flags flags = META_ALL;
    
    //video.SetMetaDataFlags(flags);
    
    //uint32_t Current_MetaData = video.GetMetaDataFlags();
    
    //printf("Current MetaData: ", Current_MetaData);
    
    for(int frame_number=0; !pangolin::ShouldQuit(); ++frame_number)
    {
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        
        if (over_exposed){
            cout << "Over" << endl;
            video.SetExposureQuant(200);
            over_exposed = false;
            cout << video.GetExposureQuant() << endl;
        }
        else {
            cout << "Under" << endl;
            video.SetExposureQuant(800);
            over_exposed = true;
            cout << video.GetExposureQuant() << endl;
        }
        
        // wait 1/30th of a second to sync change in settings and display
        boost::this_thread::sleep(boost::posix_time::seconds(1/30));
        sleep(1/30);
        
        
        // Grab frame without saving
        video.SaveFrame(     
                           frame_number, 
                           img, 
                           true
                        ); 
        
        //cout << video.ReadShutter(img) << endl;
        
        //video.SaveOneShot(frame_number, empty_frame, img);

        texVideo.Upload(img, vid_fmt.channels==1 ? GL_LUMINANCE:GL_RGB, GL_UNSIGNED_BYTE);

        // Activate video viewport and render texture
        vVideo.Activate();
        texVideo.RenderToViewportFlipY();

        // Swap back buffer with front and process window events via GLUT
        pangolin::FinishGlutFrame();
    }

    delete[] img;
    
    return 0;
}



