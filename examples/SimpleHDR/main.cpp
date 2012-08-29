/**
 * SimpleHDR
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
    video.LoadConfig();
    video.SetHDRRegister(false);
    video.SetMetaDataFlags( META_ALL_AND_ABS );
    video.CreateShutterMaps();
    video.SetAllFeaturesAuto();
    video.PrintCameraReport();
    
    unsigned char* img = new unsigned char[video.SizeBytes()];

    /*-----------------------------------------------------------------------
     *  GUI
     *-----------------------------------------------------------------------*/    
   
    VideoPixelFormat vid_fmt = VideoFormatFromString(video.PixFormat());
    const unsigned w = video.Width();
    const unsigned h = video.Height();
    
    // Create Glut window
    const int panel_width = 200;
    double scale = 1.25;
    CreateGlutWindowAndBind("SimpleHDR", w*scale + panel_width, h*scale);
    int win = glutGetWindow();

    // Create viewport for video with fixed aspect
    View& d_panel = CreatePanel("ui.")
    .SetBounds(0.0, 1.0, 0.0, Attach::Pix(panel_width));
    
    // Create viewport for video with fixed aspect
    View& vVideo = Display("Video")
    .SetBounds(0.0, 1.0, Attach::Pix(panel_width), 1.0)
    .SetAspect((float)w/h);
    
    // OpenGl Texture for video frame
    GlTexture texVideo(w,h,GL_RGBA8);
    
    /*-----------------------------------------------------------------------
     *  KEYBOARD SHORCUTS
     *-----------------------------------------------------------------------*/ 
    
    RegisterKeyPressCallback( 'h', SetVarFunctor<bool>("ui.HDR Mode", true));                 // hdr mode 
    RegisterKeyPressCallback( 'n', SetVarFunctor<bool>("ui.HDR Mode", false));                // normal mode
    RegisterKeyPressCallback( 32 , SetVarFunctor<bool>("ui.Record", true));                   // start/stop recording
    RegisterKeyPressCallback( 'c', SetVarFunctor<bool>("ui.Capture Frame", true));            // grab single image
    RegisterKeyPressCallback( 'm', SetVarFunctor<bool>("ui.Manual Camera Settings", true));   // manual on
    RegisterKeyPressCallback( 'a', SetVarFunctor<bool>("ui.Manual Camera Settings", false));  // manual off
    RegisterKeyPressCallback( 'e', SetVarFunctor<bool>("ui.Automatic Exposure Control", true));  // AEC on

    /*-----------------------------------------------------------------------
     *  CONTROL PANEL
     *-----------------------------------------------------------------------*/ 
    
    // camera brand/model info
    static Var<string> vendor("ui.Vendor", video.GetCameraVendor());
    static Var<string> camera("ui.Camera", video.GetCameraModel());
    
    // capture options
    static Var<bool> record("ui.Record",false,false);
    static Var<bool> hdr("ui.HDR Mode",false,true);
    static Var<bool> AEC("ui.Automatic Exposure Control",false,true);
    
    // recording info
    static Var<int> recorded_frames("ui.Recorded Frames", 0);
    static Var<int> recorded_time("ui.Recorded (secs)", 0);
    
    // single frame
    static Var<bool> capture("ui.Capture Frame",false,false);
    static Var<bool> capture_hdr("ui.Capture HDR Frame",false,false);

    // manual control
    static Var<bool> manual("ui.Manual Camera Settings",false,true);

    // camera settings
    static Var<float> shutter("ui.Shutter (s)", video.GetFeatureValue(DC1394_FEATURE_SHUTTER),
                               video.GetFeatureValueMin(DC1394_FEATURE_SHUTTER),
                               video.GetFeatureValueMax(DC1394_FEATURE_SHUTTER), true);

    static Var<float> exposure("ui.Exposure (EV)", video.GetFeatureValue(DC1394_FEATURE_EXPOSURE),
                                video.GetFeatureValueMin(DC1394_FEATURE_EXPOSURE),
                                video.GetFeatureValueMax(DC1394_FEATURE_EXPOSURE),false);    

    static Var<float> brightness("ui.Brightness (%)", video.GetFeatureValue(DC1394_FEATURE_BRIGHTNESS),
                                 video.GetFeatureValueMin(DC1394_FEATURE_BRIGHTNESS),
                                 video.GetFeatureValueMax(DC1394_FEATURE_BRIGHTNESS),false); 
    
    static Var<float> gain("ui.Gain (dB)", video.GetFeatureValue(DC1394_FEATURE_GAIN),
                           video.GetFeatureValueMin(DC1394_FEATURE_GAIN),
                           video.GetFeatureValueMax(DC1394_FEATURE_GAIN),false);    
    
    static Var<float> gamma("ui.Gamma", video.GetFeatureValue(DC1394_FEATURE_GAMMA),
                            video.GetFeatureValueMin(DC1394_FEATURE_GAMMA),
                            video.GetFeatureValueMax(DC1394_FEATURE_GAMMA),false);    
    
    static Var<float> saturation("ui.Saturation (%)", video.GetFeatureValue(DC1394_FEATURE_SATURATION),
                                 video.GetFeatureValueMin(DC1394_FEATURE_SATURATION),
                                 video.GetFeatureValueMax(DC1394_FEATURE_SATURATION),false);  
    
    static Var<float> hue("ui.Hue (deg)", video.GetFeatureValue(DC1394_FEATURE_HUE),
                        video.GetFeatureValueMin(DC1394_FEATURE_HUE),
                        video.GetFeatureValueMax(DC1394_FEATURE_HUE),false);  

    static Var<int> sharpness("ui.Sharpness",video.GetFeatureQuant(DC1394_FEATURE_SHARPNESS),
                              video.GetFeatureQuantMin(DC1394_FEATURE_SHARPNESS),
                              video.GetFeatureQuantMax(DC1394_FEATURE_SHARPNESS),false);  
    
    static Var<int> whitebalance_B_U("ui.White Balance (Blue/U)",video.GetWhiteBalanceBlueU(),
                                     video.GetFeatureQuantMin(DC1394_FEATURE_WHITE_BALANCE),
                                     video.GetFeatureQuantMax(DC1394_FEATURE_WHITE_BALANCE),false);  
    
    static Var<int> whitebalance_R_V("ui.White Balance (Red/V)",video.GetWhiteBalanceRedV(),
                                     video.GetFeatureQuantMin(DC1394_FEATURE_WHITE_BALANCE),
                                     video.GetFeatureQuantMax(DC1394_FEATURE_WHITE_BALANCE),false);  
    
    // loaded options
    static Var<string> response("ui.Response:", video.GetConfigValue("HDR_RESPONSE_CALIBRATION"));
    static Var<string> tmo("ui.TMO", video.GetConfigValue("HDR_TMO"));
  
    /*-----------------------------------------------------------------------
     *  CAPTURE LOOP
     *-----------------------------------------------------------------------*/     
    
    bool save = false;    
    bool under_over = true;
    time_t start, end;
    uint32_t hdr_shutter[3];    
    uint32_t aec_shutter[3];    

    // loop until quit (e.g ESC key)
    for(int frame_number = 0; !ShouldQuit(); ++frame_number)
    {     
        // Screen refresh
        if(HasResized())
            DisplayBase().ActivateScissorAndClear();

        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        
        /*-----------------------------------------------------------------------
         *  CONTROL LOGIC
         *-----------------------------------------------------------------------*/
        
        // HDR MODE
        
        // switch under over flag
        under_over = under_over ? false : true ;
        
        // shutter settings when AEC mode activated   
        if(Pushed(AEC.var->meta_gui_changed)){
            
            AEC ? cout << "[AEC]: AEC enabled" << endl : cout << "[AEC]: AEC disabled" << endl;  
            
            // copy, don't modify original hdr shutter values 
            for(int i = 0; i < 4; i++){
                aec_shutter[i] = hdr_shutter[i];
            }
            
            if(!AEC){
                video.SetHDRShutterFlags(hdr_shutter[0], hdr_shutter[2], hdr_shutter[0], hdr_shutter[2]); 
            }
        } 
        
        // will only modify values if HDR mode is on
        if (hdr && AEC){
            
            // calculate new shutter values
            if(under_over){
                aec_shutter[0] = video.AEC(img, video.GetShutterMapAbs(aec_shutter[0]), under_over);
                cout << "under shutter: " << video.GetShutterMapAbs(aec_shutter[0]) << endl;
            } else {
                aec_shutter[2] = video.AEC(img, video.GetShutterMapAbs(aec_shutter[2]), under_over);
                cout << "over shutter: " << video.GetShutterMapAbs(aec_shutter[2]) << endl;
            }
            
            // set new shutter values
            video.SetHDRShutterFlags(aec_shutter[0], aec_shutter[2], aec_shutter[0], aec_shutter[2]); 
        } 
        
        // checks if hdr mode has been switched and sets register on
        if(Pushed(hdr.var->meta_gui_changed)){
            
            if( hdr ){
                cout << "[HDR]: HDR mode enabled" << endl;        
                video.SetFeatureAuto(DC1394_FEATURE_SHUTTER);
                float EV = video.GetFeatureValue(DC1394_FEATURE_EXPOSURE);
                
                cout << "[HDR]: HDR mode bracket range set at: " << endl;
                for (int i = -1; i <= 1 ; i++){
                    video.SetFeatureValue(DC1394_FEATURE_EXPOSURE, EV+i);
                    sleep(1);
                    cout << "> " << i << ": " << EV+i << " EV" << endl;
                    hdr_shutter[i+1] = video.GetFeatureQuant(DC1394_FEATURE_SHUTTER);
                } 

                // set shutter values
                video.SetHDRShutterFlags(hdr_shutter[0], hdr_shutter[2], hdr_shutter[0], hdr_shutter[2]); 
                video.SetHDRRegister(true);
                
            } else {
                cout << "[HDR]: HDR mode disabled" << endl;    
                video.SetHDRRegister(false);
            }
        
        }
           
        // MANUAL SETTINGS

        if ( manual && !hdr ) { 
            
            /* 
             * exposure & shutter inter-linked, therefore need special logic control
             *  if: exposure val is changed, shutter val is changed (camera works out new value)
             *  else: shutter val is set to gui value 
             */

            if(Pushed(exposure.var->meta_gui_changed)){
                video.SetFeatureValue(DC1394_FEATURE_EXPOSURE, exposure); 
                video.SetFeatureAuto(DC1394_FEATURE_SHUTTER);
                shutter.operator=(video.GetFeatureValue(DC1394_FEATURE_SHUTTER));
            }
            else{
                video.SetFeatureValue(DC1394_FEATURE_SHUTTER, shutter);
            }
            
        }

        if ( manual ){    
            video.SetFeatureValue(DC1394_FEATURE_BRIGHTNESS, brightness);
            video.SetFeatureValue(DC1394_FEATURE_GAIN, gain);
            video.SetFeatureValue(DC1394_FEATURE_GAMMA, gamma); 
            video.SetFeatureValue(DC1394_FEATURE_SATURATION, saturation);
            video.SetFeatureValue(DC1394_FEATURE_HUE, hue); 
            video.SetFeatureQuant(DC1394_FEATURE_SHARPNESS, sharpness);
            video.SetWhiteBalance(whitebalance_B_U, whitebalance_R_V);
        } 

        // resets all features to automatic mode and update trackbars
        else if( !manual && !hdr ) { 
            
            // set auto mode for all features and reset gamma and hue
            video.SetAllFeaturesAuto(); 
            video.ResetBrightness();
            video.ResetHue();
            video.ResetGamma();
            
            // update all trackbars upon switching back to automatic mode
            exposure.operator=(video.GetFeatureValue(DC1394_FEATURE_EXPOSURE));
            shutter.operator=(video.GetFeatureValue(DC1394_FEATURE_SHUTTER));
            brightness.operator=(video.GetFeatureValue(DC1394_FEATURE_BRIGHTNESS));
            gain.operator=(video.GetFeatureValue(DC1394_FEATURE_GAIN));
            gamma.operator=(video.GetFeatureValue(DC1394_FEATURE_GAMMA));
            saturation.operator=(video.GetFeatureValue(DC1394_FEATURE_SATURATION));
            hue.operator=(video.GetFeatureValue(DC1394_FEATURE_HUE));
            sharpness.operator=(video.GetFeatureQuant(DC1394_FEATURE_SHARPNESS));
            whitebalance_B_U.operator=(video.GetWhiteBalanceBlueU());
            whitebalance_R_V.operator=(video.GetWhiteBalanceRedV());

        }
        
        if( Pushed(capture) ) { video.SaveSingleFrame(img); } 
        
        if( Pushed(capture_hdr) ){
            
            // see if response function has already been generated
            if (!video.CheckResponseFunction()) {
                cout << "[HDR]: No response function found, generating one" << endl;
                video.GetResponseFunction();
            }
            video.SetAllFeaturesAuto();
            sleep(1);
            video.SetFeatureAuto(DC1394_FEATURE_SHUTTER);
            float EV = video.GetFeatureValue(DC1394_FEATURE_EXPOSURE);
            for (int i = -1; i <= 1 ; i++){
                video.SetFeatureValue(DC1394_FEATURE_EXPOSURE, EV+i);
                sleep(1);
                cout << "[HDR]: image " << i+1 << " @ " << EV+i << "EV" << endl;
                hdr_shutter[i+1] = video.GetFeatureQuant(DC1394_FEATURE_SHUTTER);
            } 
            
            cout << "[HDR]: EV range calibrated" << endl;
            
            // capture HDR images
            video.CaptureHDRFrame(img, 3, hdr_shutter);
            
        } 

        /*-----------------------------------------------------------------------
         *  Save options
         *-----------------------------------------------------------------------*/    
        
        // start/stop recording
        if ( save && Pushed(record) ){ 
            
            save = false; // set save flag to false
            
            if(hdr){          
                cout << "[VIDEO]: Processing HDR video" << endl;
                boost::thread(&FirewireVideo::SaveHDRVideo, &video, frame_number);    
            } else {
                // refactor
                cout << "[VIDEO]: Processing video" << endl;
                char time_stamp[32];
                char command[128];
                char output[1024];
                char *video_format;
                
                // set output video format from config or if not loaded, to default (mpeg)
                if (video.CheckConfigLoaded()){
                    video_format = (char*) video.GetConfigValue("NORMAL_VIDEO_FORMAT").c_str();
                } else {
                    video_format = (char *) "mpeg" ;
                }
                // get time stamp for file name
                video.GetTimeStamp(time_stamp);
                
                sprintf(output, "%s.%s", time_stamp, video_format);
                
                // create command string: convert video, remove files and then echo completed - should be thread safe this way
                sprintf(command, "convert -quality 100 ./video/ppm/*.ppm ./video/%s \
                        && rm -rf ./video/ppm/  \
                        && echo '[VIDEO]: Video saved to ./video/%s'", 
                        output, output); 
                
                // run video conversion
                 boost::thread(system,command);  
                //boost::thread(&FirewireVideo::SaveVideo, &video);
            }
         
        }

        if ( !save && Pushed(record) ){ 
            cout << "[VIDEO]: Recording video" << endl;
            save = true; 
            time (&start); // get current time
            frame_number = 0; 
            recorded_frames.operator=(frame_number);
            recorded_time.operator=(0);
        }

        // save mode
        if ( save && hdr ){
            video.RecordFramesOneShot(frame_number, img, true, hdr); 
            recorded_frames.operator=(frame_number);
            time (&end);
            recorded_time.operator=(difftime(end, start));
        } 
        else if ( save ){
            video.RecordFrames(frame_number, img, true, false, hdr); 
            recorded_frames.operator=(frame_number);
            time (&end);
            recorded_time.operator=(difftime(end, start));
        } 
        else{
            video.GrabOneShot(img);
        }
       
        /*-----------------------------------------------------------------------
         *  Refresh screen
         *-----------------------------------------------------------------------*/    
        
        texVideo.Upload(img, vid_fmt.channels==1 ? GL_LUMINANCE:GL_RGB, GL_UNSIGNED_BYTE);
        // Activate video viewport and render texture
        vVideo.ActivateScissorAndClear();
        texVideo.RenderToViewportFlipY();
        // Swap back buffer with front and process window events via GLUT
        d_panel.Render();
        glutSetWindow(win);
        FinishGlutFrame();

    }

    delete[] img;

    return 0;
}
