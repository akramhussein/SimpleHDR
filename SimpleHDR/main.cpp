/**
 * @author  Akram Hussein
 * Copyright (C) 2012  Akram Hussein
 *                     Imperial College London
 **/

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


struct CustomType
{
    string y;
};

std::ostream& operator<< (std::ostream& os, const CustomType& o){
    os << o.y;
    return os;
}

std::istream& operator>> (std::istream& is, CustomType& o){
    is >> o.y;
    return is;
}

int main( int argc, char* argv[] )
{

    /*-----------------------------------------------------------------------
     *  SETUP SOURCE
     *-----------------------------------------------------------------------*/    
    
    FirewireVideo video = FirewireVideo();     
    video.SetAutoAll();
    video.PrintCameraReport();

    unsigned char* img = new unsigned char[video.SizeBytes()];
    
    VideoPixelFormat vid_fmt = VideoFormatFromString(video.PixFormat());
    const unsigned w = video.Width();
    const unsigned h = video.Height();
    
    /*-----------------------------------------------------------------------
     *  GUI
     *-----------------------------------------------------------------------*/    
    
    // Create Glut window
    const int panel_width = 200;
    pangolin::CreateGlutWindowAndBind("SimpleHDR",w + panel_width,h);
    
    // Create viewport for video with fixed aspect
    View& d_panel = pangolin::CreatePanel("ui.")
    .SetBounds(0.0, 1.0, 0.0, Attach::Pix(panel_width));
    
    // Create viewport for video with fixed aspect
    View& vVideo = Display("Video")
    .SetBounds(0.0, 1.0, Attach::Pix(panel_width), 1.0)
    .SetAspect((float)w/h);
    
    // OpenGl Texture for video frame
    GlTexture texVideo(w,h,GL_RGBA8);
    
    //keyboard shortcuts
    pangolin::RegisterKeyPressCallback( 'h', SetVarFunctor<bool>("ui.HDR", true));
    pangolin::RegisterKeyPressCallback( 'n', SetVarFunctor<bool>("ui.HDR", false));
    pangolin::RegisterKeyPressCallback( 's', SetVarFunctor<bool>("ui.Record", true));
    pangolin::RegisterKeyPressCallback( 'm', SetVarFunctor<bool>("ui.Manual Camera Settings", true));
    pangolin::RegisterKeyPressCallback( 'a', SetVarFunctor<bool>("ui.Manual Camera Settings", false));
    pangolin::RegisterKeyPressCallback( 'r', SetVarFunctor<bool>("ui.Reset Camera Settings", true));
    pangolin::RegisterKeyPressCallback( 'f', SetVarFunctor<bool>("ui.Reset Frame Count", true));
    
    // options
    static Var<bool> record("ui.Record",false,false);
    static Var<bool> hdr("ui.HDR",false,true);
    //static Var<bool> AEC("ui.Automatic Exposure Control",false,true);
    //static Var<bool> motion("ui.Motion Correction",false,true);
    static Var<bool> manual("ui.Manual Camera Settings",false,true);

    // camera settings
    //static Var<float> shutter("ui.Shutter",video.GetShutterTimeQuant(),video.GetShutterTimeQuantMin()+numeric_limits<double>::epsilon(),video.GetShutterTimeQuantMax(), false);
    static Var<float> shutter("ui.Shutter (s)",video.GetShutterTime(),video.GetShutterTimeMin(),video.GetShutterTimeMax(), true);
    static Var<float> exposure("ui.Exposure (EV)",video.GetExposure(),video.GetExposureMin(),video.GetExposureMax(), false);      //faulty  
    static Var<float> brightness("ui.Brightness (%)",video.GetBrightness(),video.GetBrightnessMin(),video.GetBrightnessMax(), false); 
    static Var<float> gain("ui.Gain (dB)",video.GetGain(),video.GetGainMin(),video.GetGainMax(), false);
    static Var<float> gamma("ui.Gamma",video.GetGamma(),video.GetGammaMin(),video.GetGammaMax(), false);
    static Var<float> saturation("ui.Saturation",video.GetSaturation(),video.GetSaturationMin(),video.GetSaturationMax(), false);
    static Var<int> hue("ui.Hue (deg)",video.GetHueQuant(),video.GetHueQuantMin(),video.GetHueQuantMax(), false);
    static Var<int> sharpness("ui.Sharpness",video.GetSharpnessQuant(),video.GetSharpnessQuantMin(),video.GetSharpnessQuantMax(), false);
    //static Var<float> framerate("ui.Framerate (fps)",0,0,100);
    //static Var<float> tilt("ui.Tilt",0,0,100);
    //static Var<float> pan("ui.Pan",0,0,100);
     
    static Var<bool> reset("ui.Reset Camera Settings",false,false);
    
    // info
    static Var<int> current_frame("ui.Current Frame",0);
    static Var<bool> reset_frame("ui.Reset Frame Count",false,false);
    
    /*-----------------------------------------------------------------------
     *  CAPTURE
     *-----------------------------------------------------------------------*/    
      
    bool over_exposed = true;
    bool save = false;
    
    for(int frame_number=0; !pangolin::ShouldQuit(); ++frame_number)
    {
        if(pangolin::HasResized())
            DisplayBase().ActivateScissorAndClear();
        
        current_frame.operator=(frame_number);
        
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        
        if(pangolin::Pushed(reset)){
            shutter.Reset();
            exposure.Reset();
            brightness.Reset();
            gain.Reset();
            gamma.Reset();
            saturation.Reset();
            hue.Reset();
            sharpness.Reset();
        }
        
        if( hdr ) {
            if (over_exposed){
                //cout << "Over" << endl;
                video.SetShutterTimeQuant(150);
                over_exposed = false;
                //cout << video.GetShutterTimeQuant() << endl;
            }
            else {
                //cout << "Under" << endl;
                video.SetShutterTimeQuant(100);
                over_exposed = true;
                //cout << video.GetShutterTimeQuant() << endl;
            }
        }
        else{
            video.SetAutoAll();
        }
        
        if (manual){ 
            video.SetShutterTime(shutter);
            video.SetExposure(exposure);
            video.SetBrightness(brightness);
            video.SetGain(gain);
            video.SetGamma(gamma);
            video.SetSaturation(saturation);
            video.SetHueQuant(hue);
            video.SetSharpnessQuant(sharpness);
        }

        if(pangolin::Pushed(reset_frame)){ frame_number = 0; }

        
        // start/stop save
        if (!save && pangolin::Pushed(record)){ save = true; }
        if (save && pangolin::Pushed(record)){ save = false; }

        // save mode
        if (save && hdr){
            // wait 1/30th of a second to sync change in settings and display
            boost::this_thread::sleep(boost::posix_time::seconds(1/30));
            video.SaveFrame(frame_number, img, true, true);  

        } 
        else if (save){
            video.SaveFrame(frame_number, img, true, true);  
        }        
        else {
            video.GrabOneShot(img);
        }
        
        texVideo.Upload(img, vid_fmt.channels==1 ? GL_LUMINANCE:GL_RGB, GL_UNSIGNED_BYTE);
        // Activate video viewport and render texture
        vVideo.ActivateScissorAndClear();
        texVideo.RenderToViewportFlipY();
        // Swap back buffer with front and process window events via GLUT
        d_panel.Render();
        pangolin::FinishGlutFrame();
        
    }

    delete[] img;

    return 0;
}



