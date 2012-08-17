    /* This file is part of the Pangolin Project.
    * http://github.com/stevenlovegrove/Pangolin
    *
    * Copyright (c) 2011 Steven Lovegrove
    *
    * Permission is hereby granted, free of charge, to any person
    * obtaining a copy of this software and associated documentation
    * files (the "Software"), to deal in the Software without
    * restriction, including without limitation the rights to use,
    * copy, modify, merge, publish, distribute, sublicense, and/or sell
    * copies of the Software, and to permit persons to whom the
    * Software is furnished to do so, subject to the following
    * conditions:
    *
    * The above copyright notice and this permission notice shall be
    * included in all copies or substantial portions of the Software.
    *
    * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
    * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    * OTHER DEALINGS IN THE SOFTWARE.
    */

    #include "firewire.h"
    #include "image.h"

    using namespace std;

    namespace pangolin
    {

    void FirewireVideo::init_camera(
    uint64_t guid, int dma_frames,
    dc1394speed_t iso_speed,
    dc1394video_mode_t video_mode,
    dc1394framerate_t framerate
    ) {

    if(video_mode>=DC1394_VIDEO_MODE_FORMAT7_0)
      throw VideoException("[DC1394 ERROR]: format7 modes need to be initialized through the constructor that allows for specifying the roi");

    camera = dc1394_camera_new (d, guid);
    if (!camera)
        throw VideoException("[DC1394 ERROR]: Failed to initialize camera");

    // Attempt to stop camera if it is already running
    dc1394switch_t is_iso_on = DC1394_OFF;
    dc1394_video_get_transmission(camera, &is_iso_on);
    if (is_iso_on==DC1394_ON) {
        dc1394_video_set_transmission(camera, DC1394_OFF);
    }

    err = dc1394_feature_get_all(camera, &features);  
    if (err != DC1394_SUCCESS) {
        throw VideoException("[DC1394 ERROR]: Could not get camera feature set");
    }

    cout << "[INFO]: Using camera with GUID " << camera->guid << endl;

    //-----------------------------------------------------------------------
    //  setup capture
    //-----------------------------------------------------------------------

    if( iso_speed >= DC1394_ISO_SPEED_800)
    {
        err=dc1394_video_set_operation_mode(camera, DC1394_OPERATION_MODE_1394B);
        if( err != DC1394_SUCCESS )
            throw VideoException("[DC1394 ERROR]: Could not set DC1394_OPERATION_MODE_1394B");
    }

    err=dc1394_video_set_iso_speed(camera, iso_speed);
    if( err != DC1394_SUCCESS )
        throw VideoException("[DC1394 ERROR]: Could not set iso speed");

    err=dc1394_video_set_mode(camera, video_mode);
    if( err != DC1394_SUCCESS )
        throw VideoException("[DC1394 ERROR]: Could not set video mode");

    err=dc1394_video_set_framerate(camera, framerate);
    if( err != DC1394_SUCCESS )
        throw VideoException("[DC1394 ERROR]: Could not set framerate");

    err=dc1394_capture_setup(camera,dma_frames, DC1394_CAPTURE_FLAGS_DEFAULT);
    if( err != DC1394_SUCCESS )
        throw VideoException("[DC1394 ERROR]: Could not setup camera - check settings");

    //-----------------------------------------------------------------------
    //  initialise width and height from mode
    //-----------------------------------------------------------------------
    dc1394_get_image_size_from_video_mode(camera, video_mode, &width, &height);

    Start();
    }


    // Note:
    // the following was tested on a IIDC camera over USB therefore might not work as
    // well on a camera over proper firewire transport
    void FirewireVideo::init_format7_camera(
    uint64_t guid, int dma_frames,
    dc1394speed_t iso_speed,
    dc1394video_mode_t video_mode,
    int framerate,
    uint32_t width, uint32_t height,
    uint32_t left, uint32_t top, bool reset_at_boot
    ) {

    if(video_mode< DC1394_VIDEO_MODE_FORMAT7_0)
        throw VideoException("[DC1394 ERROR]: roi can be specified only for format7 modes");

    camera = dc1394_camera_new (d, guid);
    if (!camera)
        throw VideoException("[DC1394 ERROR]: Failed to initialize camera");

    // Attempt to stop camera if it is already running
    dc1394switch_t is_iso_on = DC1394_OFF;
    dc1394_video_get_transmission(camera, &is_iso_on);
    if (is_iso_on==DC1394_ON) {
        dc1394_video_set_transmission(camera, DC1394_OFF);
    }

    cout << "[INFO]: Using camera with GUID " << camera->guid << endl;

    if(reset_at_boot){
      dc1394_camera_reset(camera);
    }

    //-----------------------------------------------------------------------
    //  setup mode and roi
    //-----------------------------------------------------------------------

    if(iso_speed >= DC1394_ISO_SPEED_800)
    {
        err=dc1394_video_set_operation_mode(camera, DC1394_OPERATION_MODE_1394B);
        if( err != DC1394_SUCCESS )
           throw VideoException("[DC1394 ERROR]: Could not set DC1394_OPERATION_MODE_1394B");
    }

    err=dc1394_video_set_iso_speed(camera, iso_speed);
    if( err != DC1394_SUCCESS )
        throw VideoException("[DC1394 ERROR]: Could not set iso speed");

    // check that the required mode is actually supported
    dc1394format7mode_t format7_info;

    err = dc1394_format7_get_mode_info(camera, video_mode, &format7_info);
    if( err != DC1394_SUCCESS )
      throw VideoException("[DC1394 ERROR]: Could not get format7 mode info");

    // safely set the video mode
    err=dc1394_video_set_mode(camera, video_mode);
    if( err != DC1394_SUCCESS )
        throw VideoException("Could not set format7 video mode");

    // set position to 0,0 so that setting any size within min and max is a valid command
    err = dc1394_format7_set_image_position(camera, video_mode,0,0);
    if( err != DC1394_SUCCESS )
      throw VideoException("[DC1394 ERROR]: Could not set format7 image position");

    // work out the desired image size
    width = nearest_value(width, format7_info.unit_pos_x, 0, format7_info.max_size_x - left);
    height = nearest_value(height, format7_info.unit_pos_y, 0, format7_info.max_size_y - top);

    // set size
    err = dc1394_format7_set_image_size(camera,video_mode,width,height);
    if( err != DC1394_SUCCESS )
      throw VideoException("[DC1394 ERROR]: Could not set format7 size");

    // get the info again since many parameters depend on image size
    err = dc1394_format7_get_mode_info(camera, video_mode, &format7_info);
    if( err != DC1394_SUCCESS )
      throw VideoException("[DC1394 ERROR]: Could not get format7 mode info");

    // work out position of roi
    left = nearest_value(left, format7_info.unit_size_x, format7_info.unit_size_x, format7_info.max_size_x - width);
    top = nearest_value(top, format7_info.unit_size_y, format7_info.unit_size_y, format7_info.max_size_y - height);

    // set roi position
    err = dc1394_format7_set_image_position(camera,video_mode,left,top);
    if( err != DC1394_SUCCESS )
      throw VideoException("[DC1394 ERROR]: Could not set format7 size");

    this->width = width;
    this->height = height;
    this->top = top;
    this->left = left;

    cout<<"roi: "<<left<<" "<<top<<" "<<width<<" "<<height<<"  ";


    //-----------------------------------------------------------------------
    //  setup frame rate
    //-----------------------------------------------------------------------

    if((framerate == MAX_FR)||(framerate == EXT_TRIG)){

      err = dc1394_format7_set_packet_size(camera,video_mode, format7_info.max_packet_size);
      if( err != DC1394_SUCCESS )
        throw VideoException("[DC1394 ERROR]: Could not set format7 packet size");

    } else {

      // setting packet size to get the desired frame rate according to the libdc docs
      // does not do the trick, so for now we support only max frame rate

        throw VideoException("[DC1394 ERROR]: In format 7 only max frame rate is currently supported");
      //      uint32_t depth;
      //      err = dc1394_format7_get_data_depth(camera, video_mode, &depth);
      //      if( err != DC1394_SUCCESS )
      //        throw VideoException("Could not get format7 depth");
      //
      //      // the following is straight from the libdc docs
      //      double bus_period = bus_period_from_iso_speed(iso_speed);
      //
      //      // work out the max number of packets that the bus can deliver
      //      int num_packets = (int) (1.0/(bus_period*framerate) + 0.5);
      //
      //      if((num_packets > 4095)||(num_packets < 0))
      //        throw VideoException("number of format7 packets out of range");
      //
      //      // work out what the packet size should be for the requested size and framerate
      //      uint32_t packet_size = (width*964*depth + (num_packets*8) - 1)/(num_packets*8);
      //      packet_size = nearest_value(packet_size,format7_info.unit_packet_size,format7_info.unit_packet_size,format7_info.max_packet_size);
      //
      //      if(packet_size > format7_info.max_packet_size){
      //        throw VideoException("format7 requested frame rate and size exceed bus bandwidth");
      //      }
      //
      //      err=dc1394_format7_set_packet_size(camera,video_mode, packet_size);
      //      if( err != DC1394_SUCCESS ){
      //        throw VideoException("Could not set format7 packet size");
      //      }
    }

    // ask the camera what is the resulting framerate (this assume that such a rate is actually
    // allowed by the shutter time)
    err = dc1394_feature_set_power(camera,DC1394_FEATURE_FRAME_RATE,DC1394_OFF);
    if( err != DC1394_SUCCESS )
        throw VideoException("[DC1394 ERROR]: Could not turn off frame rate");

    float value;
    err=dc1394_feature_get_absolute_value(camera,DC1394_FEATURE_FRAME_RATE,&value);
    if( err != DC1394_SUCCESS )
        throw VideoException("[DC1394 ERROR]: Could not get framerate");

    cout<<" framerate(shutter permitting):"<<value<<endl;

    //-----------------------------------------------------------------------
    //  setup capture
    //-----------------------------------------------------------------------

    err=dc1394_capture_setup(camera,dma_frames, DC1394_CAPTURE_FLAGS_DEFAULT);
    if( err != DC1394_SUCCESS )
        throw VideoException("[DC1394 ERROR]: Could not setup camera - check settings");

    Start();

    }


    std::string Dc1394ColorCodingToString(dc1394color_coding_t coding)
    {
    switch(coding)
    {
        case DC1394_COLOR_CODING_RGB8 :    return "RGB24";
        case DC1394_COLOR_CODING_MONO8 :   return "GRAY8";

    //        case DC1394_COLOR_CODING_MONO16 :  return "GRAY16LE";
    //        case DC1394_COLOR_CODING_RGB16 :   return "RGB48LE";
    //        case DC1394_COLOR_CODING_MONO16S : return "GRAY16BE";
    //        case DC1394_COLOR_CODING_RGB16S :  return "RGB48BE";

    //        case DC1394_COLOR_CODING_YUV411 :  return "YUV411P";
    //        case DC1394_COLOR_CODING_YUV422 :  return "YUV422P";
    //        case DC1394_COLOR_CODING_YUV444 :  return "YUV444P";
    //        case DC1394_COLOR_CODING_RAW8 :    return "RAW8";
    //        case DC1394_COLOR_CODING_RAW16 :   return "RAW16";
        default:
            throw VideoException("[DC1394 ERROR]: Unknown colour coding");
    }
    }

    dc1394color_coding_t Dc1394ColorCodingFromString(std::string coding)
    {
    if(     !coding.compare("RGB24"))    return DC1394_COLOR_CODING_RGB8;
    else if(!coding.compare("GRAY8"))    return DC1394_COLOR_CODING_MONO8;

    //    else if(!coding.compare("GRAY16LE")) return DC1394_COLOR_CODING_MONO16;
    //    else if(!coding.compare("RGB48LE"))  return DC1394_COLOR_CODING_RGB16;
    //    else if(!coding.compare("GRAY16BE")) return DC1394_COLOR_CODING_MONO16S;
    //    else if(!coding.compare("RGB48BE"))  return DC1394_COLOR_CODING_RGB16S;

    
    //    else if(!coding.compare("YUV411P"))  return DC1394_COLOR_CODING_YUV411;
    //    else if(!coding.compare("YUV422P"))  return DC1394_COLOR_CODING_YUV422;
    //    else if(!coding.compare("YUV444P"))  return DC1394_COLOR_CODING_YUV444;
    //    else if(!coding.compare("RAW8"))     return DC1394_COLOR_CODING_RAW8;
    //    else if(!coding.compare("RAW16"))    return DC1394_COLOR_CODING_RAW16;
    throw VideoException("Unknown colour coding");
    }

    void Dc1394ModeDetails(dc1394video_mode_t mode, unsigned& w, unsigned& h, string& format )
    {
    switch( mode )
    {
    // RGB Modes
    case DC1394_VIDEO_MODE_1024x768_RGB8:
    w=1024;    h=768;    format = "RGB24";
    break;
    case DC1394_VIDEO_MODE_640x480_RGB8:
    w=640;    h=480;    format = "RGB24";
    break;
    case DC1394_VIDEO_MODE_800x600_RGB8:
    w=800;    h=600;    format = "RGB24";
    break;
    case DC1394_VIDEO_MODE_1280x960_RGB8:
    w=1280;    h=960;    format = "RGB24";
    break;
    case DC1394_VIDEO_MODE_1600x1200_RGB8:
    w=1600;    h=1200;    format = "RGB24";
    break;

    // Greyscale modes
    case DC1394_VIDEO_MODE_640x480_MONO8:
    w=640;    h=480;    format = "GRAY8";
    break;
    case DC1394_VIDEO_MODE_800x600_MONO8:
    w=800;    h=600;    format = "GRAY8";
    break;
    case DC1394_VIDEO_MODE_1024x768_MONO8:
    w=1024;    h=768;    format = "GRAY8";
    break;
    case DC1394_VIDEO_MODE_1280x960_MONO8:
    w=1280;    h=960;    format = "GRAY8";
    break;
    case DC1394_VIDEO_MODE_1600x1200_MONO8:
    w=1600;    h=1200;    format = "GRAY8";
    break;
    case DC1394_VIDEO_MODE_640x480_MONO16:
    w=640;    h=480;    format = "GRAY16";
    break;
    case DC1394_VIDEO_MODE_800x600_MONO16:
    w=800;    h=600;    format = "GRAY16";
    break;
    case DC1394_VIDEO_MODE_1024x768_MONO16:
    w=1024;    h=768;    format = "GRAY16";
    break;
    case DC1394_VIDEO_MODE_1280x960_MONO16:
    w=1280;    h=960;    format = "GRAY16";
    break;
    case DC1394_VIDEO_MODE_1600x1200_MONO16:
    w=1600;    h=1200;    format = "GRAY16";
    break;

    // Chrome modes
    case DC1394_VIDEO_MODE_640x480_YUV411:
    w=640;    h=480;    format = "YUV411P";
    break;
    case DC1394_VIDEO_MODE_160x120_YUV444:
    w=160;    h=120;    format = "YUV444P";
    break;
    case DC1394_VIDEO_MODE_320x240_YUV422:
    w=320;    h=240;    format = "YUV422P";
    break;
    case DC1394_VIDEO_MODE_640x480_YUV422:
    w=640;    h=480;    format = "YUV422P";
    break;
    case DC1394_VIDEO_MODE_800x600_YUV422:
    w=800;    h=600;    format = "YUV422P";
    break;
    case DC1394_VIDEO_MODE_1024x768_YUV422:
    w=1024;    h=768;    format = "YUV422P";
    break;
    case DC1394_VIDEO_MODE_1600x1200_YUV422:
    w=1600;    h=1200;    format = "YUV422P";
    break;
    case DC1394_VIDEO_MODE_1280x960_YUV422:
    w=1280;    h=960;    format = "YUV422P";
    break;
    default:
      throw VideoException("[DC1394 ERROR]: Unknown colour coding");
    }
    }

    FirewireVideo::~FirewireVideo()
    {
        Stop();
        
        if(shutter_lookup_table) delete shutter_lookup_table;
        
        // Close camera
        dc1394_video_set_transmission(camera, DC1394_OFF);
        dc1394_capture_stop(camera);
        dc1394_camera_free(camera);
        dc1394_free (d);
    }

        
    std::string FirewireVideo::PixFormat() const
    {
    dc1394video_mode_t video_mode;
    dc1394color_coding_t color_coding;
    dc1394_video_get_mode(camera,&video_mode);
    dc1394_get_color_coding_from_video_mode(camera,video_mode,&color_coding);
    return Dc1394ColorCodingToString(color_coding);
    }

    size_t FirewireVideo::SizeBytes() const
    {
    return (Width() * Height() * VideoFormatFromString(PixFormat()).bpp) / 8;
    }

    void FirewireVideo::Start()
    {
    if( !running )
    {
        err=dc1394_video_set_transmission(camera, DC1394_ON);
        if( err != DC1394_SUCCESS )
            throw VideoException("[DC1394 ERROR]: Could not start camera iso transmission");
        running = true;
    }
    }

    void FirewireVideo::Stop()
    {

    cout << "[INFO]: Stopping camera transmission" << endl;

    if( running )
    {
        // Stop transmission
        err = dc1394_video_set_transmission(camera,DC1394_OFF);
        if( err != DC1394_SUCCESS )
            throw VideoException("[DC1394 ERROR]: Could not stop the camera");
        running = false;
    }
    }

    bool FirewireVideo::CheckMultiShotCapable(){
        if (camera->multi_shot_capable >0 ) return true;
        else return false;
    }
    
    void FirewireVideo::SetMultiShotOn(int num_frames){
        
        err = dc1394_video_set_multi_shot(camera, num_frames, DC1394_ON);
        if( err != DC1394_SUCCESS )
            throw VideoException("[DC1394 ERROR]: Could not turn on multi-shot mode");
        
    }
    
    void FirewireVideo::SetMultiShotOff(){
        
        err = dc1394_video_set_multi_shot(camera, 0, DC1394_OFF);
        if( err != DC1394_SUCCESS )
            throw VideoException("[DC1394 ERROR]: Could not turn off multi-shot mode");
        
    }
        
    void FirewireVideo::StopForOneShot()
    {
        if( running )
        {
            // Stop transmission
            err = dc1394_video_set_transmission(camera,DC1394_OFF);
            if( err != DC1394_SUCCESS )
                throw VideoException("[DC1394 ERROR]: Could not stop the camera");
            running = false;
        }
        //Call to eliminate spurious frames
        err = dc1394_video_set_one_shot(camera, DC1394_OFF);
        if( err != DC1394_SUCCESS )
            throw VideoException("[DC1394 ERROR]: Could not set one shot to OFF");
        FlushDMABuffer();
    }

    bool FirewireVideo::CheckOneShotCapable() {
        if (camera->one_shot_capable == DC1394_TRUE) return true;
        else return false;
    }

    bool FirewireVideo::GrabOneShot(unsigned char* image) {
        
        dc1394_video_set_one_shot(camera, DC1394_ON);
        dc1394video_frame_t *frame;
        
        dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, &frame);   
        if( frame )
        {
            memcpy(image,frame->image,frame->image_bytes);
            dc1394_capture_enqueue(camera,frame);
            
            //print time stamps
            /*
            time_t time_ms = (frame->timestamp);
            time_t time_secs = (frame->timestamp)/1000000;
            printf("Epoch: %ld Timestamp: %s", time_ms, ctime(&time_secs));
            */
            //dc1394_video_set_one_shot(camera, DC1394_OFF);
            //cout << "frames behind: " << frame->frames_behind << endl;
            //CreatePixIntensityMap(*frame);
                    
            return true;
        }
        return false;
    }
        
    void FirewireVideo::GrabNFramesMulti(unsigned char *image, int n, float shut[]){
        
        // frame array
        dc1394video_frame_t *frame[n];
        dc1394video_frame_t *discarded_frame;
        
        SetHDRShutterFlags(
                           GetShutterMapQuant(shut[0]),
                           GetShutterMapQuant(shut[1]),
                           GetShutterMapQuant(shut[2]),
                           GetShutterMapQuant(shut[3])
                           );
                         
        SetHDRRegister(true);
        Start();
        SetMultiShotOn(n);
        
        //discard first frame -- not working
        dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, &discarded_frame);
        dc1394_capture_enqueue(camera, discarded_frame);
        
        for(int i = 0; i < n ; i++){
          dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, &frame[i]);  
        }
        
        SetMultiShotOff();

        for(int i = 0; i < n; i++){
            if(frame[i]){
                //memcpy(image,frame[i]->image,frame[i]->image_bytes);
                //cout << "Shutter: " << ReadShutter(image) << endl;
                boost::thread(&FirewireVideo::SaveFile, this, i, *frame[i], "hdr-frames", true);
                dc1394_capture_enqueue(camera, frame[i]);
            }
        }
    
    }
        
        
    //! image updating won't work for now
    void FirewireVideo::GrabNFrames(unsigned char *image, int n, int shut[]){
        
        dc1394video_frame_t *f1;
        dc1394video_frame_t *f2;
        dc1394video_frame_t *f3;
        dc1394video_frame_t *f4;
        
        // turn off HDR register mode just in case
        
        SetHDRRegister(false);
        /*
        uint32_t under = GetShutterMapQuant(0.00450313);
        uint32_t over = GetShutterMapQuant(0.00200000);

        SetHDRShutterFlags(over,under,over,under);
        */
        //steps 2-3 - stop transmission and flush dma buffer
        StopForOneShot();  
        
        // 4.Grab N frames
        for(int i = 0 ; i < n; i++){
            //cout << shut[i] << endl;
            //set shutter value
            //SetFeatureQuant(DC1394_FEATURE_SHUTTER, shut[i]);
            //sleep(2/30);

            // turn on one shot
            dc1394_video_set_one_shot(camera, DC1394_ON);
            //sleep(2/30);
            SetFeatureQuant(DC1394_FEATURE_SHUTTER, shut[0]);
            cout << "get shutter: " << GetFeatureQuant(DC1394_FEATURE_SHUTTER) << endl;
            //sleep(2/30);
            dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, &f1); 

            dc1394_video_set_one_shot(camera, DC1394_ON);
            //sleep(2/30);
            SetFeatureQuant(DC1394_FEATURE_SHUTTER, shut[1]);
             cout << "get shutter: " << GetFeatureQuant(DC1394_FEATURE_SHUTTER) << endl;
            //sleep(2/30);
            dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, &f2); 

            dc1394_video_set_one_shot(camera, DC1394_ON);
            //sleep(2/30);
            SetFeatureQuant(DC1394_FEATURE_SHUTTER, shut[2]);
             cout << "get shutter: " << GetFeatureQuant(DC1394_FEATURE_SHUTTER) << endl;
            //sleep(2/30);
            dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, &f3); 

            dc1394_video_set_one_shot(camera, DC1394_ON);
            //sleep(2/30);
            SetFeatureQuant(DC1394_FEATURE_SHUTTER, shut[3]);
             cout << "get shutter: " << GetFeatureQuant(DC1394_FEATURE_SHUTTER) << endl;
            //sleep(2/30);
            dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, &f4); 
            
            if( f1 )
            {
                memcpy(image,f1->image,f1->image_bytes);
                cout << "Shutter: " << ReadShutter(image) << endl;
                cout << "Time stamp: " << ReadTimeStamp(image) << endl;
            }
            
            if( f2 )
            {
                memcpy(image,f2->image,f2->image_bytes);
                cout << "Shutter: " << ReadShutter(image) << endl;
                cout << "Time stamp: " << ReadTimeStamp(image) << endl;
            }            
            
            if( f3 )
            {
                memcpy(image,f3->image,f3->image_bytes);
                cout << "Shutter: " << ReadShutter(image) << endl;
                cout << "Time stamp: " << ReadTimeStamp(image) << endl;
            }
            
            if( f4 )
            {
                memcpy(image,f4->image,f4->image_bytes);
                cout << "Shutter: " << ReadShutter(image) << endl;
                cout << "Time stamp: " << ReadTimeStamp(image) << endl;
            }
            
            // empty frames
            // FlushDMABuffer();
            dc1394_capture_enqueue(camera, f1);
            dc1394_capture_enqueue(camera, f2);
            dc1394_capture_enqueue(camera, f3);
            dc1394_capture_enqueue(camera, f4);
            /*
            if( frame )
            {
                memcpy(image,frame->image,frame->image_bytes);
                cout << "Shutter: " << ReadShutter(image) << endl;
                cout << "Time stamp: " << ReadTimeStamp(image) << endl;
                
                dc1394_capture_enqueue(camera, frame);
            }
             */
        }
        
        // 6. restart transmission
        Start();
     
    }
        
        
    void FirewireVideo::FlushDMABuffer()
    {
        cout << "[INFO]: Flushing camera DMA buffer" << endl;
        Stop();
        
        dc1394video_frame_t *frame;
        int discarded_frames = 0;

        while( true ) {
            
            if( dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_POLL, &frame) != DC1394_SUCCESS){
                throw VideoException("[DC1394 ERROR]: Could not dequeue frame");
            } 
            if (!frame) { break; }
                if( dc1394_capture_enqueue(camera, frame) != DC1394_SUCCESS){
                    throw VideoException("[DC1394 ERROR]: Could not enqueue frame");   
                }
            discarded_frames++;
        }
 
        cout << "[INFO]: Flushed frames: " << discarded_frames << endl;
    }
       
    FirewireVideo::FirewireVideo(
    Guid guid,
    dc1394video_mode_t video_mode,
    dc1394framerate_t framerate,
    dc1394speed_t iso_speed,
    int dma_buffers
    ) :running(false),top(0),left(0)
    {
    d = dc1394_new ();
    if (!d)
        throw VideoException("[DC1394 ERROR]: Failed to get 1394 bus");
    shutter_lookup_table = 0;
    init_camera(guid.guid,dma_buffers,iso_speed,video_mode,framerate);
    }

    FirewireVideo::FirewireVideo(
    Guid guid,
    dc1394video_mode_t video_mode,
    int framerate,
    uint32_t width, uint32_t height,
    uint32_t left, uint32_t top,
    dc1394speed_t iso_speed,
    int dma_buffers, bool reset_at_boot
    ) :running(false)
    {
    d = dc1394_new ();
    if (!d)
        throw VideoException("[DC1394 ERROR]: Failed to get 1394 bus");
    shutter_lookup_table = 0;
    init_format7_camera(guid.guid,dma_buffers,iso_speed,video_mode,framerate,width,height,left,top, reset_at_boot);
    }

    FirewireVideo::FirewireVideo(
                                    unsigned deviceid,
                                    dc1394video_mode_t video_mode,
                                    dc1394framerate_t framerate,
                                    dc1394speed_t iso_speed,
                                    int dma_buffers
                                ) :running(false),top(0),left(0)
        {
        d = dc1394_new ();
        if (!d)
            throw VideoException("[DC1394 ERROR]: Failed to get 1394 bus");

        err=dc1394_camera_enumerate (d, &list);
        if( err != DC1394_SUCCESS )
            throw VideoException("[DC1394 ERROR]: Failed to enumerate cameras");

        if (list->num == 0)
            throw VideoException("[DC1394 ERROR]: No cameras found");

        if( deviceid >= list->num )
            throw VideoException("[DC1394 ERROR]: Invalid camera index");

        const uint64_t guid = list->ids[deviceid].guid;

        dc1394_camera_free_list (list);
        shutter_lookup_table = 0;
        init_camera(guid,dma_buffers,iso_speed,video_mode,framerate);

    }

    FirewireVideo::FirewireVideo(
    unsigned deviceid,
    dc1394video_mode_t video_mode,
    int framerate,
    uint32_t width, uint32_t height,
    uint32_t left, uint32_t top,
    dc1394speed_t iso_speed,
    int dma_buffers, bool reset_at_boot
    ) :running(false)
    {
    d = dc1394_new ();
    if (!d)
        throw VideoException("[DC1394 ERROR]: Failed to get 1394 bus");

    err=dc1394_camera_enumerate (d, &list);
    if( err != DC1394_SUCCESS )
        throw VideoException("[DC1394 ERROR]: Failed to enumerate cameras");

    if (list->num == 0)
        throw VideoException("[DC1394 ERROR]: No cameras found");

    if( deviceid >= list->num )
        throw VideoException("[DC1394 ERROR]: Invalid camera index");

    const uint64_t guid = list->ids[deviceid].guid;

    dc1394_camera_free_list (list);
    shutter_lookup_table = 0;
    init_format7_camera(guid,dma_buffers,iso_speed,video_mode,framerate,width,height,left,top, reset_at_boot);

    }
        
    /*-----------------------------------------------------------------------
     *  FRAME GRAB
     *-----------------------------------------------------------------------*/
    
    bool FirewireVideo::GrabNext( unsigned char* image, bool wait )
    {
    const dc1394capture_policy_t policy =
            wait ? DC1394_CAPTURE_POLICY_WAIT : DC1394_CAPTURE_POLICY_POLL;

    dc1394video_frame_t *frame;
    dc1394_capture_dequeue(camera, policy, &frame);
    if( frame )
    {
        memcpy(image,frame->image,frame->image_bytes);
        dc1394_capture_enqueue(camera,frame);
        return true;
    }
    return false;
    }

    bool FirewireVideo::GrabNewest( unsigned char* image, bool wait )
    {
    dc1394video_frame_t *f;
    dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_POLL, &f);

    if( f ) {
        while( true )
        {
            dc1394video_frame_t *nf;
            dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_POLL, &nf);
            if( nf )
            {
                err=dc1394_capture_enqueue(camera,f);
                f = nf;
            }else{
                break;
            }
        }
        memcpy(image,f->image,f->image_bytes);
        err=dc1394_capture_enqueue(camera,f);
        return true;
    }else if(wait){
        return GrabNext(image,true);
    }
    return false;
    }

    FirewireFrame FirewireVideo::GetNext(bool wait)
    {
    const dc1394capture_policy_t policy =
            wait ? DC1394_CAPTURE_POLICY_WAIT : DC1394_CAPTURE_POLICY_POLL;

    dc1394video_frame_t *frame;
    dc1394_capture_dequeue(camera, policy, &frame);
    return FirewireFrame(frame);
    }

    FirewireFrame FirewireVideo::GetNewest(bool wait)
    {
    dc1394video_frame_t *f;
    dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_POLL, &f);

    if( f ) {
        while( true )
        {
            dc1394video_frame_t *nf;
            dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_POLL, &nf);
            if( nf )
            {
                err=dc1394_capture_enqueue(camera,f);
                f = nf;
            }else{
                break;
            }
        }
        return FirewireFrame(f);
    }else if(wait){
        return GetNext(true);
    }
    return FirewireFrame(0);
    }

    void FirewireVideo::PutFrame(FirewireFrame& f)
    {
    if( f.frame )
    {
        dc1394_capture_enqueue(camera,f.frame);
        f.frame = 0;
    }
    }
    /*-----------------------------------------------------------------------
     *  FEATURE CONTROL
     *-----------------------------------------------------------------------*/    
    
    // move to header file 
    dc1394feature_t feature[] = {
        DC1394_FEATURE_BRIGHTNESS,
        DC1394_FEATURE_EXPOSURE,
        DC1394_FEATURE_SHARPNESS,
        DC1394_FEATURE_WHITE_BALANCE,
        DC1394_FEATURE_HUE,
        DC1394_FEATURE_SATURATION,
        DC1394_FEATURE_GAMMA,
        DC1394_FEATURE_SHUTTER,
        DC1394_FEATURE_GAIN,
        DC1394_FEATURE_IRIS,
        DC1394_FEATURE_FOCUS,
        DC1394_FEATURE_TEMPERATURE,
        DC1394_FEATURE_TRIGGER,
        DC1394_FEATURE_TRIGGER_DELAY,
        DC1394_FEATURE_WHITE_SHADING,
        DC1394_FEATURE_FRAME_RATE,
        DC1394_FEATURE_ZOOM,
        DC1394_FEATURE_PAN,
        DC1394_FEATURE_TILT,
        DC1394_FEATURE_OPTICAL_FILTER,
        DC1394_FEATURE_CAPTURE_SIZE,
        DC1394_FEATURE_CAPTURE_QUALITY
    };
        
    void FirewireVideo::SetAllFeaturesAuto()
    {
                
    dc1394feature_modes_t modes;
        
    for (int i = 0; i <= 22; i++){
        
        if(feature[i] == DC1394_FEATURE_TRIGGER || feature[i] == DC1394_FEATURE_TRIGGER_DELAY ){
            dc1394_feature_set_power(camera, DC1394_FEATURE_TRIGGER, DC1394_OFF);       // these tend to break things so i've left them alone
            dc1394_feature_set_power(camera, DC1394_FEATURE_TRIGGER_DELAY, DC1394_OFF); // these tend to break things so i've left them alone
            break;
        }
        
        err = dc1394_feature_get_modes(camera, feature[i], &modes);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not get modes for feature ", dc1394_feature_get_string(feature[i]));
        }
        
  
        if (modes.modes[1] == DC1394_FEATURE_MODE_AUTO){
            
            err = dc1394_feature_set_power(camera, feature[i], DC1394_ON);
            if (err != DC1394_SUCCESS) {
                break;
            }
            
            err = dc1394_feature_set_mode(camera, feature[i], DC1394_FEATURE_MODE_AUTO);
            if (err != DC1394_SUCCESS) {
                break;
            }
            
        }
    }
        
        ResetGamma();
        ResetHue();
        
    }
        
    void FirewireVideo::SetAllFeaturesManual(){
       
        dc1394feature_modes_t modes;

        for (int i = 0; i <= 22; i++){
            
            if(feature[i] == DC1394_FEATURE_TRIGGER || feature[i] == DC1394_FEATURE_TRIGGER_DELAY ){
                dc1394_feature_set_power(camera, DC1394_FEATURE_TRIGGER, DC1394_OFF);       // these tend to break things so i've left them alone
                dc1394_feature_set_power(camera, DC1394_FEATURE_TRIGGER_DELAY, DC1394_OFF); // these tend to break things so i've left them alone
                break;
            }
            
            err = dc1394_feature_get_modes(camera, feature[i], &modes);
            if (err != DC1394_SUCCESS) {
                break;
            }
            
        
            if (modes.modes[0] == DC1394_FEATURE_MODE_MANUAL){
                
                err = dc1394_feature_set_power(camera, feature[i], DC1394_ON);
                if (err != DC1394_SUCCESS) {
                    break;
                }
                
                err = dc1394_feature_set_mode(camera, feature[i], DC1394_FEATURE_MODE_MANUAL);
                if (err != DC1394_SUCCESS) {
                    break;
                }
                
            }
        }

        
    }
        
    void FirewireVideo::SetFeatureAuto(dc1394feature_t feature){
        
        err = dc1394_feature_set_power(camera, feature, DC1394_ON);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not turn on ", dc1394_feature_get_string(feature));
        }
        
        err = dc1394_feature_set_mode(camera, feature, DC1394_FEATURE_MODE_AUTO);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not set auto mode for ", dc1394_feature_get_string(feature));
        }
        
    }

    void FirewireVideo::SetFeatureManual(dc1394feature_t feature){
        
        err = dc1394_feature_set_power(camera, feature, DC1394_ON);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not turn on ", dc1394_feature_get_string(feature));
        }
        
        err = dc1394_feature_set_mode(camera, feature, DC1394_FEATURE_MODE_MANUAL);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not set manual mode for ", dc1394_feature_get_string(feature));
        }
        
    }

    void FirewireVideo::SetFeatureOn(dc1394feature_t feature){
        
        err = dc1394_feature_set_power(camera, feature, DC1394_ON);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not turn on ", dc1394_feature_get_string(feature));
        }
        
    }

    void FirewireVideo::SetFeatureOff(dc1394feature_t feature){
        
        err = dc1394_feature_set_power(camera, feature, DC1394_OFF);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not turn off ", dc1394_feature_get_string(feature));
        }
        
    }

    void FirewireVideo::SetFeatureValue(dc1394feature_t feature, float value){
        
        SetFeatureOn(feature);
        
        err = dc1394_feature_set_mode(camera, feature, DC1394_FEATURE_MODE_MANUAL);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not set manual mode for ", dc1394_feature_get_string(feature));
        }
        
        err = dc1394_feature_set_absolute_control(camera, feature, DC1394_ON);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not turn absolute mode off for ", dc1394_feature_get_string(feature));
        }
             
        err = dc1394_feature_set_absolute_value(camera, feature, value);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could set absolute value for ", dc1394_feature_get_string(feature));
        }
        
    }

    void FirewireVideo::SetFeatureQuant(dc1394feature_t feature, int value){
        
        err = dc1394_feature_set_mode(camera, feature, DC1394_FEATURE_MODE_MANUAL);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not set manual mode for ", dc1394_feature_get_string(feature));
        }
        
        err = dc1394_feature_set_absolute_control(camera, feature, DC1394_OFF);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not turn absolute mode off for ", dc1394_feature_get_string(feature));
        }
        
        err = dc1394_feature_set_value(camera, feature, value);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could set quantised value for ", dc1394_feature_get_string(feature));
        }
        
    }

    bool FirewireVideo::GetFeaturePower(dc1394feature_t feature){
        
        dc1394switch_t pwr;
        
        err = dc1394_feature_get_power(camera, feature, &pwr);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could check if power on/off for feature ", dc1394_feature_get_string(feature));
        }
        
        if(pwr == 1){
            return true;
        }
        return false;
    }
        
    int FirewireVideo::GetFeatureMode(dc1394feature_t feature) const{
        
        dc1394feature_mode_t mode;
        
        err =  dc1394_feature_get_mode(camera, feature, &mode);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not check mode of feature ", dc1394_feature_get_string(feature));
        }
        
        if (mode == 737){
            return 0; // auto mode
        } 
        
        return 1; // manual mode
    }
            
    float FirewireVideo::GetFeatureValue(dc1394feature_t feature) const {
       
        float value;
        
        err = dc1394_feature_get_absolute_value(camera, feature, &value);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could set absolute value for ", dc1394_feature_get_string(feature));
        }

        return value;
        
    }

    int FirewireVideo::GetFeatureQuant(dc1394feature_t feature) const {
        
        uint32_t value;
        
        err = dc1394_feature_get_value(camera, feature, &value);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not get quantised value for ", dc1394_feature_get_string(feature));
        }
        
        return value;
        
    }

    float FirewireVideo::GetFeatureValueMax(dc1394feature_t feature) const {
        
        float min, max;
       
        err = dc1394_feature_get_absolute_boundaries(camera, feature, &min, &max);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could get max absolute value for ", dc1394_feature_get_string(feature));
        }
        
        return max;
        
    }

    float FirewireVideo::GetFeatureValueMin(dc1394feature_t feature) const {
        
        float min, max;
        
        err = dc1394_feature_get_absolute_boundaries(camera, feature, &min, &max);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could get min absolute value for ", dc1394_feature_get_string(feature));
        }
        
        return min;
        
    }

    int FirewireVideo::GetFeatureQuantMax(dc1394feature_t feature) const {
        
        uint32_t min, max;
        
        err =  dc1394_feature_get_boundaries(camera, feature, &min, &max);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could get max quantised value for ", dc1394_feature_get_string(feature));
        }
        
        return max;
        
    }
    
    int FirewireVideo::GetFeatureQuantMin(dc1394feature_t feature) const {
        
        uint32_t min, max;
        
        err =  dc1394_feature_get_boundaries(camera, feature, &min, &max);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could get min quantised value for ", dc1394_feature_get_string(feature));
        }
        
        return min;
        
    }
                                           
    /*-----------------------------------------------------------------------
     *  WHITE BALANCE
     *-----------------------------------------------------------------------*/
        
    void FirewireVideo::SetSingleAutoWhiteBalance(){

        err = dc1394_feature_set_mode(camera, DC1394_FEATURE_WHITE_BALANCE, DC1394_FEATURE_MODE_ONE_PUSH_AUTO);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not set manual white balance mode");
        }
    }
    
        
    void FirewireVideo::SetWhiteBalance(unsigned int Blue_U_val, unsigned int Red_V_val){

        err = dc1394_feature_set_mode(camera, DC1394_FEATURE_WHITE_BALANCE, DC1394_FEATURE_MODE_MANUAL);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not set manual white balance mode");
        }

        err = dc1394_feature_whitebalance_set_value(camera, Blue_U_val, Red_V_val);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not set white balance value");
        }

    }

    void FirewireVideo::GetWhiteBalance(unsigned int *Blue_U_val, unsigned int *Red_V_val) {

    err = dc1394_feature_whitebalance_get_value(camera,Blue_U_val, Red_V_val );
    if( err != DC1394_SUCCESS )
        throw VideoException("[DC1394 ERROR]: Failed to read white balance");
    }    
    
        
    int FirewireVideo::GetWhiteBalanceBlueU()
    {
        uint32_t Blue_U_val, Red_V_val;
        
        err = dc1394_feature_whitebalance_get_value( camera, &Blue_U_val, &Red_V_val );
        if( err != DC1394_SUCCESS )
            throw VideoException("[DC1394 ERROR]: Failed to read white balance");

        return Blue_U_val;
    }
        
    int FirewireVideo::GetWhiteBalanceRedV()
    {
        uint32_t Blue_U_val, Red_V_val;
        
        err = dc1394_feature_whitebalance_get_value( camera, &Blue_U_val, &Red_V_val );
        if( err != DC1394_SUCCESS )
            throw VideoException("[DC1394 ERROR]: Failed to read white balance");
        
        return Red_V_val;
    }

    void FirewireVideo::ResetBrightness()
    {
        err = dc1394_feature_set_power(camera, DC1394_FEATURE_BRIGHTNESS, DC1394_ON);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not set brightness feature on");
        }
        
        err = dc1394_feature_set_absolute_control(camera, DC1394_FEATURE_BRIGHTNESS, DC1394_OFF);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not set absolute control for brightness off");
        }
        
        err = dc1394_feature_set_value(camera, DC1394_FEATURE_BRIGHTNESS, 0);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not set absolute gamma value to 0");
        }
        
    } 
    
    void FirewireVideo::ResetGamma()
    {
        err = dc1394_feature_set_power(camera, DC1394_FEATURE_GAMMA, DC1394_ON);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not set gamma feature on");
        }
        
        err = dc1394_feature_set_absolute_control(camera, DC1394_FEATURE_GAMMA, DC1394_ON);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not set absolute control for gamma");
        }
        
        err = dc1394_feature_set_absolute_value(camera, DC1394_FEATURE_GAMMA, 1.0);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not set absolute gamma value to 1.0");
        }
        
    } 

    void FirewireVideo::ResetHue()
    {
        err = dc1394_feature_set_power(camera, DC1394_FEATURE_HUE, DC1394_ON);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not set hue feature on");
        }
        
        err = dc1394_feature_set_absolute_control(camera, DC1394_FEATURE_HUE, DC1394_OFF);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not set absolute control for hue off");
        }
        
        err = dc1394_feature_set_value(camera, DC1394_FEATURE_HUE, 2048);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not set absolute hue value to 2048 i.e. 0");
        }
        
    } 
        
    /*-----------------------------------------------------------------------
     *  TRIGGERS
     *-----------------------------------------------------------------------*/

    void FirewireVideo::SetInternalTrigger() 
    {
        err = dc1394_external_trigger_set_power(camera, DC1394_OFF);
        if ( err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not set internal trigger mode");
        }
        
    }

    void FirewireVideo::SetExternalTrigger(dc1394trigger_mode_t mode, dc1394trigger_polarity_t polarity, dc1394trigger_source_t source)
    {
    err = dc1394_external_trigger_set_polarity(camera, polarity);
    if (err != DC1394_SUCCESS) {
        throw VideoException("[DC1394 ERROR]: Could not set external trigger polarity");
    }

    err = dc1394_external_trigger_set_mode(camera, mode);
    if (err != DC1394_SUCCESS) {
        throw VideoException("[DC1394 ERROR]: Could not set external trigger mode");
    }

    err = dc1394_external_trigger_set_source(camera, source);
    if (err != DC1394_SUCCESS) {
        throw VideoException("[DC1394 ERROR]: Could not set external trigger source");
    }

    err = dc1394_external_trigger_set_power(camera, DC1394_ON);
    if (err != DC1394_SUCCESS) {
        throw VideoException("[DC1394 ERROR]: Could not set external trigger power");
    }
}

    /*-----------------------------------------------------------------------
     *  REGISTERS - META DATA, HDR & LOOPUP TABLE ETC
     *-----------------------------------------------------------------------*/
        
    void FirewireVideo::SetMetaDataFlags( int flags ) 
    {
        meta_data_flags = 0x80000000 | flags;
        
        err = dc1394_set_control_register(camera, 0x12f8, meta_data_flags);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not set meta data flags");
        }
    }
   
    uint32_t FirewireVideo::GetMetaDataFlags() 
    {
        uint32_t flags;
        err = dc1394_get_control_register(camera, 0x12f8, &flags);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not get meta data flags");
        }
        return flags;
    }

    void FirewireVideo::SetHDRRegister(bool power){

        // flip hdr bit on (6th bit)
        uint32_t hdr_flags;
        //uint32_t hdr_flags = 0x80000000 | 33554432;
        
        if( power ) {
            cout << "[HDR]: HDR register enabled" << endl;
            hdr_flags = 0x82000000;
            hdr_register = true;

        } else{
            cout << "[HDR]: HDR register disabled" << endl;
            hdr_flags = 0x8000000;
            hdr_register = false;
            // reset shutter flags
            SetHDRShutterFlags(0,0,0,0); 
        }
        
        err = dc1394_set_control_register(camera, 0x1800, hdr_flags);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not set hdr flags");
        }
        
    }
     
    uint32_t FirewireVideo::GetHDRFlags() 
    {
        uint32_t flags;
        err = dc1394_get_control_register(camera, 0x1800, &flags);
        if (err != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not get hdr flags");
        }
        return flags;
    }
    
    void FirewireVideo::SetHDRShutterFlags(uint32_t shut0, 
                                           uint32_t shut1, 
                                           uint32_t shut2, 
                                           uint32_t shut3) 
    {

        if (dc1394_set_control_register(camera, 0x1820, 0x8000000 | shut0) != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not set hdr shutter0 flags");
        }
        //cout << "[HDR]: Shutter 0 set to: " << shut0 << endl;
        if (dc1394_set_control_register(camera, 0x1840, 0x8000000 | shut1) != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not set hdr shutter1 flags");
        }
        //cout << "[HDR]: Shutter 1 set to: " << shut1 << endl;
        if (dc1394_set_control_register(camera, 0x1860, 0x8000000 | shut2) != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not set hdr shutter2 flags");
        }
        //cout << "[HDR]: Shutter 2 set to: " << shut2 << endl;
        if (dc1394_set_control_register(camera, 0x1880, 0x8000000 | shut3) != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not set hdr shutter3 flags");
        }
        //cout << "[HDR]: Shutter 3 set to: " << shut3 << endl;
    }

    void FirewireVideo::GetHDRShutterFlags(uint32_t &shut0, 
                                           uint32_t &shut1, 
                                           uint32_t &shut2, 
                                           uint32_t &shut3) 
    {
        if (dc1394_get_control_register(camera, 0x1820, &shut0) != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not get hdr shutter0 flags");
        }
        if (dc1394_get_control_register(camera, 0x1840, &shut1) != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not get hdr shutter1 flags");
        }
        if (dc1394_get_control_register(camera, 0x1860, &shut2) != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not get hdr shutter2 flags");
        }
        if (dc1394_get_control_register(camera, 0x1880, &shut3) != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not get hdr shutter3 flags");
        }
        /*
        cout << "Shutter Values" << endl;
        cout << GetShutterMapAbs(shut0 - 2181038080) << endl;
        cout << GetShutterMapAbs(shut1 - 2181038080) << endl;
        cout << GetShutterMapAbs(shut2 - 2181038080) << endl;
        cout << GetShutterMapAbs(shut3 - 2181038080) << endl;
         */
    }
        
    void FirewireVideo::SetHDRGainFlags(uint32_t gain0, 
                                        uint32_t gain1, 
                                        uint32_t gain2, 
                                        uint32_t gain3) 
    {
        if (dc1394_set_control_register(camera, 0x1824, 0x8000000 | gain0) != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not set hdr gain0 flags");
        }
        if (dc1394_set_control_register(camera, 0x1844, 0x8000000 | gain1) != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not set hdr gain1 flags");
        }
        if (dc1394_set_control_register(camera, 0x1864, 0x8000000 | gain2) != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not set hdr gain2 flags");
        }
        if (dc1394_set_control_register(camera, 0x1884, 0x8000000 | gain3) != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not set hdr gain3 flags");
        }
    }

        
    void FirewireVideo::GetHDRGainFlags(uint32_t &gain0, 
                                        uint32_t &gain1, 
                                        uint32_t &gain2, 
                                        uint32_t &gain3) 
    {
        // gain default is 178
        
        if (dc1394_get_control_register(camera, 0x1824, &gain0) != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not get hdr gain0 flags");
        }
        if (dc1394_get_control_register(camera, 0x1844, &gain1) != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not get hdr gain1 flags");
        }
        if (dc1394_get_control_register(camera, 0x1864, &gain2) != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not get hdr gain2 flags");
        }
        if (dc1394_get_control_register(camera, 0x1884, &gain3) != DC1394_SUCCESS) {
            throw VideoException("[DC1394 ERROR]: Could not get hdr gain3 flags");
        }
        /*
        cout << "Gain flags" << endl;
        cout << gain0 - 2181038080 << endl;
        cout << gain1 - 2181038080 << endl;
        cout << gain2 - 2181038080 << endl; 
        cout << gain3 - 2181038080 << endl;
        */
    }

    void FirewireVideo::ReadMetaData( unsigned char *image, MetaData *metaData ) {
        
    uint8_t* data = (uint8_t*)image;

    int offset = 0;

    metaData->flags = meta_data_flags;

    if(meta_data_flags & META_ABS) {
        metaData->abs_on = true;
    }
    
    if(meta_data_flags & META_TIMESTAMP) {
    metaData->timestamp = (data+4*offset)[3] + ((data+4*offset)[2] << 8) + ((data+4*offset)[1] << 16) + ((data+4*offset)[0] << 24);
    offset++;
    }
    if(meta_data_flags & META_GAIN) {
        metaData->gain = (data+4*offset)[3] + (((data+4*offset)[2]&0xf) << 8);
        offset++;
    }
    if(meta_data_flags & META_SHUTTER) {
        metaData->shutterQuant = (data+4*offset)[3] + (((data+4*offset)[2]) << 8) + ((data+4*offset)[1] << 16);
        offset++;

        // convert quantized value to absolute value from shutter map
        if(!shutter_abs_map.empty()) metaData->shutterAbs = GetShutterMapAbs(metaData->shutterQuant);
        
        //Convert quantized value to absolute value from lookup table
        //if(shutter_lookup_table) metaData->shutterAbs = shutter_lookup_table[metaData->shutterQuant];
    }
    if(meta_data_flags & META_BRIGHTNESS) {
        metaData->brightness = (data+4*offset)[3] + (((data+4*offset)[2]&0xf) << 8);
        offset++;
    }
    if(meta_data_flags & META_EXPOSURE) {
        metaData->auto_exposure = (data+4*offset)[3] + (((data+4*offset)[2]&0xf) << 8);
        offset++;
    }
    if(meta_data_flags & META_WHITE_BALANCE) {
        metaData->whitebalance_v_r = (data+4*offset)[3] + (((data+4*offset)[2]&0xF ) << 8);
        metaData->whitebalance_u_b = (data+4*offset)[1] + (((data+4*offset)[2]&0xF0) << 8);
        //TODO: Add other white balance
        offset++;
    }
    if(meta_data_flags & META_FRAME_COUNTER) {
        metaData->frame_count = (data+4*offset)[3] + ((data+4*offset)[2] << 8) + ((data+4*offset)[1] << 16) + ((data+4*offset)[0] << 24);
        offset++;
    }
    if(meta_data_flags & META_STROBE) {
        //TODO: add strobe functionality
        offset++;
    }
    if(meta_data_flags & META_GPIO_PIN_STATE) {
        //TODO: Add GPIO functionality
        offset++;
    }
    if(meta_data_flags & META_ROI_POSITION) {
        //TODO: Add ROI functionality
        offset++;
    }

    }

    float FirewireVideo::ReadShutter( unsigned char *image ) {
        
        uint8_t* data = (uint8_t*)image;

        int offset = 0;
        float ret=0;

        if(meta_data_flags & META_TIMESTAMP) {
            offset++;
        }
        if(meta_data_flags & META_GAIN) {
            offset++;
        }
        if(meta_data_flags & META_SHUTTER) {
            int shutterQuant = (data+4*offset)[3] + (((data+4*offset)[2]) << 8) + ((data+4*offset)[1] << 16);
            // convert quantized value to absolute value from lookup table
            //if(shutter_lookup_table) ret = shutter_lookup_table[shutterQuant];
            
            // convert quantized value to absolute value from shutter map
            if(!shutter_abs_map.empty()) ret = GetShutterMapAbs(shutterQuant);
                }
        return ret;
    }
   
    uint32_t FirewireVideo::ReadTimeStamp( unsigned char *image ){
        
        uint8_t* data = (uint8_t*)image;
        
        int offset = 0;

        if(meta_data_flags & META_TIMESTAMP) {
           return(data+4*offset)[3] + ((data+4*offset)[2] << 8) + ((data+4*offset)[1] << 16) + ((data+4*offset)[0] << 24);
        }
        else return 0;
    }
        
    void FirewireVideo::CreateShutterLookupTable() {
        cout << "[INFO]: Creating Shutter Lookup Table" << endl;
        shutter_lookup_table = new float[4096];
        for (int i=0; i<4096; i++) {
            SetFeatureQuant(DC1394_FEATURE_SHUTTER, i);
            shutter_lookup_table[i] = GetFeatureValue(DC1394_FEATURE_SHUTTER);
        }
        cout << "[INFO]: Shutter Lookup Table Created" << endl;
    }
        
    void FirewireVideo::CreateShutterMaps() {
        cout << "[INFO]: Creating Shutter Lookup Maps : <quant,abs> and <abs,quant>" << endl;
        float shutter;
        int max_shutter = GetFeatureQuantMax(DC1394_FEATURE_SHUTTER);
        for (int i=0 ; i <= max_shutter ; i++) {
            SetFeatureQuant(DC1394_FEATURE_SHUTTER, i);
            shutter = GetFeatureValue(DC1394_FEATURE_SHUTTER);
            shutter_abs_map[i] = shutter;
            shutter_quant_map[shutter] = i;
        }
        cout << "[INFO]: Shutter Shutter Maps Created" << endl;
    }
    
    int FirewireVideo::GetShutterMapQuant(float val){    
        return shutter_quant_map.lower_bound(val)->second;
        // return shutter_quant_map.upper_bound(val)->second;
    }
        
    float FirewireVideo::GetShutterMapAbs(int val){
        return shutter_abs_map.lower_bound(val)->second;
        //return shutter_abs_map.upper_bound(val)->second;
    }
        
    void FirewireVideo::PrintShutterMapAbs(){
        map<int,float>::iterator pos;
        for(pos = shutter_abs_map.begin(); pos != shutter_abs_map.end() ; pos++){
            cout << "Int: " << pos->first << " Float: " << pos->second << endl;
        }
    }
        
    void FirewireVideo::PrintShutterMapQuant(){
        map<float,int>::iterator pos;
        for(pos = shutter_quant_map.begin(); pos != shutter_quant_map.end(); pos++){
            cout << "Float: " << pos->first << " Int: " << pos->second << endl;
        }
    }
        
    /*-----------------------------------------------------------------------
     *   RECORDING/SAVING
     *-----------------------------------------------------------------------*/
        
    bool FirewireVideo::RecordFrames(
                                    int frame_number, 
                                    unsigned char* image, 
                                    bool wait,
                                    bool jpeg,
                                    bool hdr
                                    )
    {

        dc1394video_frame_t *frame = NULL;
        
        //wait or not -- usually yes otherwise likely to return empty frame
        const dc1394capture_policy_t policy =
        wait ? DC1394_CAPTURE_POLICY_WAIT : DC1394_CAPTURE_POLICY_POLL;

        dc1394_capture_dequeue(camera, policy, &frame);  
        
        if( frame ){
            memcpy(image,frame->image,frame->image_bytes);
            dc1394_capture_enqueue(camera,frame);
        }
 
        hdr ? SaveFile(frame_number, *frame, "hdr-video", jpeg) : SaveFile(frame_number, *frame, "video", jpeg);
        
        return true;       
        
    }              
                            
    bool FirewireVideo::RecordFramesOneShot(    
                                    int frame_number,
                                    unsigned char* image,
                                    bool jpeg,
                                    bool hdr
                                    ) 
    {
    dc1394video_frame_t *frame = NULL;

    dc1394_video_set_one_shot( camera, DC1394_ON );

    dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, &frame);  
    if( frame ){
        memcpy(image,frame->image,frame->image_bytes);
        dc1394_capture_enqueue(camera,frame);
    }

    hdr ? SaveFile(frame_number, *frame, "hdr-video", jpeg) : SaveFile(frame_number, *frame, "video", jpeg);

    return true;       

    }
        
    bool FirewireVideo::CaptureFrame(
                                     int frame_number,
                                     unsigned char* image, 
                                     bool wait,   
                                     bool jpeg 
                                     )
        {
            dc1394video_frame_t *frame = NULL;
            
            const dc1394capture_policy_t policy =
            wait ? DC1394_CAPTURE_POLICY_WAIT : DC1394_CAPTURE_POLICY_POLL;
            
            dc1394_capture_dequeue(camera, policy, &frame);  
            
            if( frame ){
                memcpy(image,frame->image,frame->image_bytes);
                dc1394_capture_enqueue(camera,frame);
            }
            
            boost::thread(&FirewireVideo::SaveFile, this, frame_number, *frame, "single-frames", jpeg);
            
            return true;     
           
            
        }
    bool FirewireVideo::CaptureFrameOneShot(
                                     int frame_number,
                                     unsigned char* image,  
                                     bool jpeg 
                                     )
    {
        dc1394video_frame_t *frame = NULL;
        
        dc1394_video_set_one_shot( camera, DC1394_ON );
        
        dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, &frame);  
        if( frame ){
            memcpy(image,frame->image,frame->image_bytes);
            dc1394_capture_enqueue(camera,frame);
        }
        
        boost::thread(&FirewireVideo::SaveFile, this, frame_number, *frame, "single-frames", jpeg);
        
        return true;     
        
    }

    void FirewireVideo::CaptureHDRFrame(unsigned char* image, int n, uint32_t shutter[])
    {
        cout << "[HDR]: Starting HDR frame capture" << endl;
        
        // frame array
        dc1394video_frame_t *frame[n];
        //dc1394video_frame_t *discarded_frame;

        // discard images from DMA buffer
        FlushDMABuffer();
        
        // embed to hdr register shutter values (abs->quant)
        SetHDRShutterFlags(
                           shutter[0],
                           shutter[0],
                           shutter[1],
                           shutter[2]
                           );
    
        // turn hdr register on
        SetHDRRegister(true);
        
        // enable multi-shot mode
        SetMultiShotOn(n);
                
        // start transmission again
        Start();

        //discard first frame -- not working
        //dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, &discarded_frame);
        //dc1394_capture_enqueue(camera, discarded_frame);
        
        // grab n frames from dma
        for(int i = 0; i < n ; i++){
            if(dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, &frame[i]) != DC1394_SUCCESS){
                throw VideoException("[DC1394 ERROR]: Could not dequeue frame");
            }
        }   
        
        // disable multi-shot mode
        SetMultiShotOff();
        
        // turn off hdr register
        SetHDRRegister(false);
        
        boost::thread_group thread_group;
        
        // save each frame to jpeg and return frames to dma to requeue the buffer
        for(int i = 0; i < n; i++){
            if(frame[i]){
                // copy to image buffer -- remove later
                //memcpy(image,frame[i]->image,frame[i]->image_bytes);
                //cout << "Shutter: " << ReadShutter(image) << endl;
                
                // add to thread group
                thread_group.create_thread(boost::bind(&FirewireVideo::SaveFile, this, i, *frame[i], "hdr-image", true)); 
                
                if(dc1394_capture_enqueue(camera, frame[i]) != DC1394_SUCCESS)
                    throw VideoException("[DC1394 ERROR]: Could not enqueue frame");
            }
        }
    
        cout << "[HDR]: Generating HDR frame" << endl;

        char time_stamp[32];
        char command[1024];
        char *tmo;
        char *format;
        
        // set attributes from config or if not loaded, to defaults
        if(!config.empty()){
            tmo = (char*) config.find("HDR_TMO")->second.c_str();
            format = (char*) config.find("HDR_FORMAT")->second.c_str();
        } else {
            tmo = (char *) "drago03";
            format = (char *) "jpeg";
        }
        
        GetTimeStamp(time_stamp);
        
        sprintf(command, "pfsinme ./hdr-image/jpeg/*1.jpeg ./hdr-image/jpeg/*2.jpeg\
                | pfshdrcalibrate -f camera.response \
                | pfsoutexr ./hdr-image/hdr.exr && pfsinexr ./hdr-image/hdr.exr \
                | pfstmo_%s | pfsout ./hdr-image/%s-HDR.jpeg \
                | rm -rf ./hdr-image/jpeg && rm -rf ./hdr-image/hdr.exr \
                && echo '[HDR]: HDR frame generated: ./hdr-image/%s-HDR.%s'", 
                tmo, time_stamp, time_stamp, format);

        //don't run final command until all other threads finish
        thread_group.join_all();
        boost::thread(system, command);
    }
        
    void FirewireVideo::SaveSingleFrame(unsigned char *image){
        
        dc1394video_frame_t *frame = NULL;
        
        // set one shot mode
        if(dc1394_video_set_one_shot( camera, DC1394_ON ) != DC1394_SUCCESS)
            throw VideoException("[DC1394 ERROR]: Could not set one shot mode");
        
        // dequeue frame
        if(dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, &frame) != DC1394_SUCCESS)
            throw VideoException("[DC1394 ERROR]: Could not dequeue frame");
        
        if( frame ){
        
            // for purpose of updating gui -- can be swapped with frame->image
            memcpy(image,frame->image,frame->image_bytes);
            dc1394_capture_enqueue(camera,frame);
            
            char filename[128];
            char date_time[128];
            MetaData metaData;
            dc1394video_mode_t video_mode = DC1394_VIDEO_MODE_640x480_RGB8; // place in constructor
            
            // create directories
            mkdir("single-frames", 0755);
            mkdir("single-frames/jpeg/", 0755);
            
            // get time stamp
            GetTimeStamp(date_time);
            // create filename
            sprintf(filename, "./single-frames/jpeg/%s%s", date_time, ".jpeg");
            
            CreateJPEG(frame, filename, video_mode);
            ReadMetaData(frame->image, &metaData);
            
            // write exif data from image meta data if abs table exists, else get from camera
            !shutter_abs_map.empty() 
            ? WriteExifDataFromImageMetaData(&metaData, filename)
            : WriteExifData(this, filename);
           
            cout << "[SAVE]: JPEG image saved to " << filename << endl;
        }
    }
    
    void FirewireVideo::GetTimeStamp(char* time_stamp){
        time_t rawtime;
        struct tm * timeinfo;
        time ( &rawtime );
        timeinfo = localtime ( &rawtime );
        strftime(time_stamp,32,"%d%b%Y_%H-%M-%S",timeinfo);
    }

    bool FirewireVideo::SaveFile(
                                 int frame_number, 
                                 dc1394video_frame_t frame, 
                                 const char* folder, 
                                 bool jpeg
                                 )
    {
        char filename[128];
        char dir[128];
        MetaData metaData;
        dc1394video_mode_t video_mode = DC1394_VIDEO_MODE_640x480_RGB8;

        // create top directory
        mkdir(folder, 0755);
        
        // pad file names -- ugly but works
        stringstream ps;
        ps << frame_number;
        std::string padding = ps.str();
        padding.insert(padding.begin(), 6 - padding.size(), '0');
        char * padded_frame_number = new char[padding.size()];
        std::copy(padding.begin(), padding.end(), padded_frame_number);
        
        // save to jpeg or ppm
        if( jpeg ){  
            
            // create jpeg folder
            sprintf(dir, "%s/jpeg", folder);
            mkdir(dir, 0755);
            
            sprintf(filename, "./%s/jpeg/%s%s%s", folder, "image", padded_frame_number, ".jpeg");
           
            CreateJPEG(&frame, filename, video_mode);
            ReadMetaData(frame.image, &metaData);

            // write exif data from image meta data if abs table exists, else from camera
            !shutter_abs_map.empty() 
            ? WriteExifDataFromImageMetaData(&metaData, filename)
            : WriteExifData(this, filename);

            cout << "[SAVE]: JPEG image saved to " << filename << endl;

            //cout << "[IMAGE]: Image average luminance cd/m^2: " << GetAvgLuminance(filename) << endl;  

        } 
        else{   
            
            // create ppm folder
            sprintf(dir, "%s/ppm", folder);
            mkdir(dir, 0755);
            
            // create path for ppm
            sprintf(filename, "./%s/ppm/%s%s%s", folder, "image", padded_frame_number, ".ppm");
            CreatePPM(&frame, filename, video_mode);
            cout << "[SAVE]: PPM image saved to " << filename << endl;
            
        }
        
        delete[] padded_frame_number;

        return true;
    }
    
    void FirewireVideo::CreatePPM(dc1394video_frame_t *frame, 
                                  const char* filename, 
                                  dc1394video_mode_t video_mode)
        {
       
        FILE* imagefile;
        unsigned int width, height;
        
        err = dc1394_get_image_size_from_video_mode(
                                              camera, 
                                              video_mode, 
                                              &width, 
                                              &height
                                              );
        if (err != DC1394_SUCCESS){
            throw VideoException("[DC1394 ERROR]: Could not get image size from video mode");
        }
        
        uint64_t numPixels = height*width;
        imagefile = fopen(filename, "wb");
        
        if( imagefile == NULL) {
            perror( "[FILE ERROR]: Can't create output file");
        }
        
        fprintf(imagefile,"P6\n%u %u\n255\n", width, height);
        fwrite(frame->image, 1, numPixels*3, imagefile);
        fclose(imagefile);
        
    }
        

    bool FirewireVideo::CreateJPEG(dc1394video_frame_t *frame, 
                                 const char *filename,  
                                 dc1394video_mode_t video_mode)
    {            
        unsigned int width, height;
        
        // get width & height
        err = dc1394_get_image_size_from_video_mode(
                                                    camera, 
                                                    video_mode, 
                                                    &width, 
                                                    &height
                                                    );
        if ( err != DC1394_SUCCESS ) {
            throw VideoException("[DC1394 ERROR]: Could not get image size from video mode");
        }
        
        struct jpeg_compress_struct cinfo;
        struct jpeg_error_mgr jerr;
        
        // this is a pointer to one row of image data
        JSAMPROW row_pointer[1];
        FILE *outfile = fopen( filename, "wb" );
        
        if ( !outfile ) {
            printf("[ERROR]: Error opening output jpeg file %s\n!", filename );
            return false;
        }
        
        cinfo.err = jpeg_std_error( &jerr );
        jpeg_create_compress(&cinfo);
        jpeg_stdio_dest(&cinfo, outfile);
    
        // Setting the parameters of the output file here 
        cinfo.image_width = width;  
        cinfo.image_height = height;
        cinfo.input_components = 3;
        cinfo.in_color_space = JCS_RGB;
        
        // default compression parameters
        jpeg_set_defaults( &cinfo );
        
        // set quality to 100%
        jpeg_set_quality ( &cinfo, 100, TRUE);
        
        // Now do the compression
        jpeg_start_compress( &cinfo, TRUE );
        
        // like reading a file, this time write one row at a time
        while( cinfo.next_scanline < cinfo.image_height )
        {
            row_pointer[0] = &frame->image[ cinfo.next_scanline * cinfo.image_width *  cinfo.input_components];
            jpeg_write_scanlines( &cinfo, row_pointer, 1 );
        }
        // similar to read file, clean up after we're done compressing 
        jpeg_finish_compress( &cinfo );
        jpeg_destroy_compress( &cinfo );
        fclose( outfile );
        
        return true;
    }
    
    dc1394video_frame_t* FirewireVideo::ConvertToRGB(dc1394video_frame_t *original_frame)
    {       
        dc1394video_frame_t *new_frame = new dc1394video_frame_t();
        new_frame->color_coding=DC1394_COLOR_CODING_RGB8;
        dc1394_convert_frames(original_frame, new_frame);
        
        return new_frame;
    
    }
         
    /*-----------------------------------------------------------------------
     *  REPORTING
     *-----------------------------------------------------------------------*/
    
    void FirewireVideo::PrintCameraReport() 
    {
        // print unique id of camera(not node/slot on chipset)
        //cout << camera->guid << endl; 
        
        // print camera details
        dc1394_camera_print_info(camera, stdout); 
        
        // print camera features
        dc1394_feature_print_all(&features, stdout);
        
    }
        
    void FirewireVideo::CreatePixIntensityMap(dc1394video_frame_t frame){
        
        int colours = 3;
        int numPixels = frame.size[0] * frame.size[1] * colours;
        int bit_depth = 2 << (frame.data_depth-1);
        map<int,int> image_pixel_intensity_count;
        
        for(int i = 0 ; i < bit_depth ; i++){
            image_pixel_intensity_count[i]=0;
        }
        
        int i = -1;
        int stop = numPixels - colours;
        
        while(i<stop){
            image_pixel_intensity_count[((int)frame.image[++i] * 0.299) 
                                        + ((int)frame.image[++i] * 0.587) 
                                        + ((int)frame.image[++i] * 0.114)]++;                        
        }

        map<int,int>::iterator pos;
        int saturation_count = 0;
        int count;
        
        for(pos = image_pixel_intensity_count.begin(); pos != image_pixel_intensity_count.end() ; pos++){
            
            count = pos->second;
            if( count == 0 || count == 255 )
            {
                saturation_count++;
            }

        }

    }
    //float FirewireVideo::AEC(dc1394video_frame_t frame, float st, bool under_over){
    float FirewireVideo::AEC(unsigned char *image, float st, bool under_over){
        
        //int i = GetMetaOffset() - 1;
        
        int colours = 3;
        int num_pixels = 921600 - GetMetaOffset();
        //int num_pixels = frame.size[0] * frame.size[1] * colours;
        int bit_depth = 2 << 7;
        //int bit_depth = 2 << (frame.data_depth-1);
        float shutter_optimum = under_over ? 0.0005 : 0.0005 ;
        
        int stop = num_pixels - colours;
        int intensity;
        int saturation_count = 0;
        
        map<int,int> image_pixel_intensity_count;
        map<int,int>::iterator pos; 
        
        // over/under exposed specific criteria
        int start, finish, saturation;
        
        // hard coded for speed
        if( under_over ){
            start = 0;    
            finish = 127; // finish = (bit_depth / 2 ) - 1
            saturation = 0; 
        } else {
            start = 128;  // finish = (bit_depth + 1) / 2
            finish = 255; // finish = bit_depth - 1
            saturation = 255; // finish = bit_depth - 1
        }
        
        for(int i = 0 ; i < bit_depth ; i++){
            image_pixel_intensity_count[i] = 0;
        }
        

        int i = -1;
        while( i < stop ){
            
            // get greyscale value once for speed
            intensity = 
              ( (int)image[++i] * 0.299 ) 
            + ( (int)image[++i] * 0.587 )
            + ( (int)image[++i] * 0.114 );
            
            //increment value in map
            image_pixel_intensity_count[intensity]++;    
            
            // if saturated, increment
            // if( saturation == 0 || saturation == bit_depth )
            if( intensity == saturation )
            {
                saturation_count++;
            }

        }
        
        int num_pixels_minus_saturation = ( num_pixels - GetMetaOffset() ) - saturation_count;
        double proportion;
        
        for( pos = image_pixel_intensity_count.find(start); pos->first <= image_pixel_intensity_count.find(finish)->first ; pos++ ){
    
            proportion += pos->second/num_pixels_minus_saturation;
            
        }
        
        return st * ( shutter_optimum / proportion );

    }
                                    
    float FirewireVideo::AEC(dc1394video_frame_t frame, float st, bool under_over){
       
        int i = GetMetaOffset() - 1;
        int colours = 3;
        int num_pixels = (frame.size[0] * frame.size[1] * colours) - GetMetaOffset();
        int bit_depth = 2 << (frame.data_depth-1);
        float shutter_optimum = under_over ? 0.0005 : 0.0005 ;
        
        int stop = num_pixels - GetMetaOffset() - colours;
        int intensity;
        int saturation_count = 0;
        
        map<int,int> image_pixel_intensity_count;
        map<int,int>::iterator pos; 
        
        // over/under exposed specific criteria
        int start, finish, saturation;
        
        // hard coded for speed
        if( under_over ){
            start = 0;    
            finish = 127; // finish = (bit_depth / 2 ) - 1
            saturation = 0; 
        } else {
            start = 128;  // finish = (bit_depth + 1) / 2
            finish = 255; // finish = bit_depth - 1
            saturation = 255; // finish = bit_depth - 1
        }
        
        for(int i = 0 ; i < bit_depth ; i++){
            image_pixel_intensity_count[i] = 0;
        }
        
        //int i = -1;
        while( i < stop ){
            
            // get greyscale value once for speed
            intensity = 
            ( (int)frame.image[++i] * 0.299 ) 
            + ( (int)frame.image[++i] * 0.587 )
            + ( (int)frame.image[++i] * 0.114 );
            
            //increment value in map
            image_pixel_intensity_count[intensity]++;    
            
            // if saturated, increment
            // if( saturation == 0 || saturation == bit_depth )
            if( intensity == saturation )
            {
                saturation_count++;
            }
            
        }
        
        int num_pixels_minus_saturation = ( num_pixels - GetMetaOffset() ) - saturation_count;
        double proportion;
        
        for( pos = image_pixel_intensity_count.find(start); pos->first <= image_pixel_intensity_count.find(finish)->first ; pos++ ){
            
            proportion += pos->second/num_pixels_minus_saturation;
            
        }
        
        cout << "proportion: " << proportion << endl;
        
        return st * ( shutter_optimum / proportion );
        
        }

        
    /*-----------------------------------------------------------------------
     *  CONVENIENCE UTILITIES
     *-----------------------------------------------------------------------*/
        
    void FirewireVideo::GetBestSettings( dc1394video_mode_t &video_mode, 
                                         dc1394framerate_t &framerate 
                                       )
        {
    
        dc1394video_modes_t video_modes;
        dc1394color_coding_t coding;
        dc1394framerates_t framerates;
        int i;

        // get video modes:
        err = dc1394_video_get_supported_modes( camera, &video_modes );
            if( err != DC1394_SUCCESS )
                throw VideoException("[DC1394 ERROR]: Could not get supported modes");
        
        // select highest res mode:
        for (i = video_modes.num-1;i>=0;i--) {
           
            if (!dc1394_is_video_mode_scalable(video_modes.modes[i])) {
                dc1394_get_color_coding_from_video_mode(camera,video_modes.modes[i], &coding);
               
                if (coding == DC1394_COLOR_CODING_RGB8) {
                    video_mode = video_modes.modes[i]; // best video_mode set
                    break;
                    
                }
                
            }
            
        }
            
        if (i < 0) {
            dc1394_log_error("[ERROR]: Could not get a valid RGB8 mode");
        }
        
        err = dc1394_get_color_coding_from_video_mode(camera, video_mode, &coding);
        if( err != DC1394_SUCCESS )
            throw VideoException("[DC1394 ERROR]: Could not get colour coding");
        
        // get highest framerate
        err = dc1394_video_get_supported_framerates(camera, video_mode, &framerates);
        if( err != DC1394_SUCCESS )
            throw VideoException("[DC1394 ERROR]: Could not get frame rates");
            
        framerate = framerates.framerates[framerates.num-1]; // best video_mode set
        
    }
    

    // UNFINISHED
    void FirewireVideo::GetResponseFunction()
    {
        dc1394video_frame_t *frame = NULL;
        FILE* file;
        char hdrgen_file[32] = "camera.hdrgen";

        float min, max, step_size, shutter;
        int no_steps = 10; // grab 10 frames
        
        // turn off HDR register control
        SetHDRRegister(false);
        
        // set all features to automatic
        SetAllFeaturesAuto();
        
        // get max and min for shutter time
        if(dc1394_feature_get_absolute_boundaries(camera, DC1394_FEATURE_SHUTTER, &min, &max) != DC1394_SUCCESS)
            throw VideoException("[DC1394 ERROR]: Failed to read shutter");
        
        shutter = (max - min)/2; // first half of shutter range is black
        step_size = shutter/no_steps; // calculate step size
        
        cout << "Min: " << min << endl;
        cout << "Max: " << max << endl;
        cout << "Step Size: " << step_size << endl;
        
        if(dc1394_feature_set_mode(camera, DC1394_FEATURE_SHUTTER, DC1394_FEATURE_MODE_MANUAL) != DC1394_SUCCESS) 
            throw VideoException("[DC1394 ERROR]: Could not set manual shutter mode");

        if(dc1394_feature_set_absolute_control(camera, DC1394_FEATURE_SHUTTER, DC1394_ON) != DC1394_SUCCESS)
            throw VideoException("[DC1394 ERROR]: Could not set absolute control for shutter");
        
        file = fopen(hdrgen_file, "wb");
        if( file == NULL) {
            perror( "[ERROR]: Can't create hdrgen file");
        }
 
        //stop camera for one shot mode
        //StopForOneShot();
        
        for(int frame_number = 0; shutter <= max ; frame_number++){
            
            cout << endl << "[INFO]: Frame number: " << frame_number << endl;
            cout << "[INFO]: Shutter value: " << shutter << endl;
            
            if(dc1394_feature_set_absolute_value(camera, DC1394_FEATURE_SHUTTER, shutter) != DC1394_SUCCESS) 
                throw VideoException("[DC1394 ERROR]: Could not set shutter value");
            
            
            // wait 1/30th second
            boost::this_thread::sleep(boost::posix_time::seconds(1/30));        
            
            // set camera to one shot
            dc1394_video_set_one_shot( camera, DC1394_ON );
            
            // grab image from DMA
            dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, &frame);
                       
            
            if ( frame ) {
                
                // save to jpeg with exif data
                SaveFile(frame_number, *frame, "response-function", true);
                
                // append line to hdrgen script for response function
                JpegToHDRGEN("response-function", file, frame_number);
                
            }

            // return frame to DMA
            dc1394_capture_enqueue(camera, frame);
            
            // increment shutter value by step size
            shutter += step_size;
        }
     
        //close hdrgen file
        cout << "[FILE]: Closing hdrgen script file" << endl;
        fclose(file);
        
        // reset shutter to auto
        SetFeatureAuto(DC1394_FEATURE_SHUTTER);

        // generate response function 
        cout << "[INFO]: Generating response function" << endl;
        system("pfsinhdrgen camera.hdrgen | pfshdrcalibrate -s camera.response");
        cout << "[INFO]: Camera Response Function file generated" << endl;
        
        // saving plot for reference
        system("gnuplot response_plotting_script.plt");
        cout << "[INFO]: Camera Response Function plot saved to camera_response.jpeg" << endl;
        
    }
        
    bool FirewireVideo::CheckResponseFunction(){
        
        ifstream file("camera.response");
        return file;

    }
    
    int FirewireVideo::GetMetaOffset(){
        
        int offset = 0;
        
        if(meta_data_flags & META_TIMESTAMP) { offset++; }
        if(meta_data_flags & META_GAIN) { offset++; }
        if(meta_data_flags & META_SHUTTER) { offset++; }
        if(meta_data_flags & META_BRIGHTNESS) { offset++; }
        if(meta_data_flags & META_EXPOSURE) { offset++; }
        if(meta_data_flags & META_WHITE_BALANCE) { offset++; }
        if(meta_data_flags & META_FRAME_COUNTER) { offset++; }
        if(meta_data_flags & META_STROBE) { offset++; }
        if(meta_data_flags & META_GPIO_PIN_STATE) { offset++; }
        if(meta_data_flags & META_ROI_POSITION) { offset++; }
        
        return offset;
        
    }
    
    void FirewireVideo::LoadConfig(){

        try {
            
            boost::property_tree::ptree pt;
            boost::property_tree::ini_parser::read_ini("config.ini", pt);
            
            // HDR
            config.insert( pair<string,string>( "HDR_TMO", pt.get<string>("HDR.tone_mapping_operator") ) );
            config.insert( pair<string,string>( "HDR_FORMAT", pt.get<string>("HDR.format") ) );
            
            // VIDEO
            config.insert( pair<string,string>( "VIDEO_FORMAT", pt.get<string>("VIDEO.format") ) );
            
        } catch (exception& e){
            cerr << "[CONFIG ERROR]:" << e.what() << endl;
        }

    }
        
    string FirewireVideo::GetConfigValue(string attribute){ 
        return config.find(attribute)->second;
    }
        
    void FirewireVideo::SetConfigValue(string attribute, string value){        
        config[attribute] = value;
    }
    
    bool FirewireVideo::CheckConfigLoaded(){
        return config.empty();
    }
        
    int FirewireVideo::nearest_value(int value, int step, int min, int max) {

        int low, high;

        low=value-(value%step);
        high=value-(value%step)+step;
        if (low<min)
        low=min;
        if (high>max)
        high=max;

        if (abs(low-value)<abs(high-value))
        return low;
        else
        return high;
        }

        double FirewireVideo::bus_period_from_iso_speed(dc1394speed_t iso_speed)
        {
        double bus_period;

        switch(iso_speed){
        case DC1394_ISO_SPEED_3200:
          bus_period = 15.625e-6;
          break;
        case DC1394_ISO_SPEED_1600:
          bus_period = 31.25e-6;
          break;
        case DC1394_ISO_SPEED_800:
          bus_period = 62.5e-6;
          break;
        case DC1394_ISO_SPEED_400:
           bus_period = 125e-6;
           break;
        case DC1394_ISO_SPEED_200:
           bus_period = 250e-6;
           break;
        case DC1394_ISO_SPEED_100:
           bus_period = 500e-6;
           break;
        default:
          throw VideoException("[DC1394 ERROR]: iso speed not valid");
        }

        return bus_period;
        }

    }

