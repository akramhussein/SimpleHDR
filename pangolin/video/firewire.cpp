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

    #include <stdio.h>
    #include <stdint.h>
    #include <stdlib.h>
    #include <inttypes.h>
    #include <sys/stat.h>

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
      throw VideoException("format7 modes need to be initialized through the constructor that allows for specifying the roi");

    camera = dc1394_camera_new (d, guid);
    if (!camera)
        throw VideoException("Failed to initialize camera");

    // Attempt to stop camera if it is already running
    dc1394switch_t is_iso_on = DC1394_OFF;
    dc1394_video_get_transmission(camera, &is_iso_on);
    if (is_iso_on==DC1394_ON) {
        dc1394_video_set_transmission(camera, DC1394_OFF);
    }

    err = dc1394_feature_get_all(camera, &features);  
    if (err != DC1394_SUCCESS) {
        throw VideoException("Could not get camera feature set");
    }

    cout << "Using camera with GUID " << camera->guid << endl;

    //-----------------------------------------------------------------------
    //  setup capture
    //-----------------------------------------------------------------------

    if( iso_speed >= DC1394_ISO_SPEED_800)
    {
        err=dc1394_video_set_operation_mode(camera, DC1394_OPERATION_MODE_1394B);
        if( err != DC1394_SUCCESS )
            throw VideoException("Could not set DC1394_OPERATION_MODE_1394B");
    }

    err=dc1394_video_set_iso_speed(camera, iso_speed);
    if( err != DC1394_SUCCESS )
        throw VideoException("Could not set iso speed");

    err=dc1394_video_set_mode(camera, video_mode);
    if( err != DC1394_SUCCESS )
        throw VideoException("Could not set video mode");

    err=dc1394_video_set_framerate(camera, framerate);
    if( err != DC1394_SUCCESS )
        throw VideoException("Could not set framerate");

    err=dc1394_capture_setup(camera,dma_frames, DC1394_CAPTURE_FLAGS_DEFAULT);
    if( err != DC1394_SUCCESS )
        throw VideoException("Could not setup camera - check settings");

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
        throw VideoException("roi can be specified only for format7 modes");

    camera = dc1394_camera_new (d, guid);
    if (!camera)
        throw VideoException("Failed to initialize camera");

    // Attempt to stop camera if it is already running
    dc1394switch_t is_iso_on = DC1394_OFF;
    dc1394_video_get_transmission(camera, &is_iso_on);
    if (is_iso_on==DC1394_ON) {
        dc1394_video_set_transmission(camera, DC1394_OFF);
    }

    cout << "Using camera with GUID " << camera->guid << endl;

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
           throw VideoException("Could not set DC1394_OPERATION_MODE_1394B");
    }

    err=dc1394_video_set_iso_speed(camera, iso_speed);
    if( err != DC1394_SUCCESS )
        throw VideoException("Could not set iso speed");

    // check that the required mode is actually supported
    dc1394format7mode_t format7_info;

    err = dc1394_format7_get_mode_info(camera, video_mode, &format7_info);
    if( err != DC1394_SUCCESS )
      throw VideoException("Could not get format7 mode info");

    // safely set the video mode
    err=dc1394_video_set_mode(camera, video_mode);
    if( err != DC1394_SUCCESS )
        throw VideoException("Could not set format7 video mode");

    // set position to 0,0 so that setting any size within min and max is a valid command
    err = dc1394_format7_set_image_position(camera, video_mode,0,0);
    if( err != DC1394_SUCCESS )
      throw VideoException("Could not set format7 image position");

    // work out the desired image size
    width = nearest_value(width, format7_info.unit_pos_x, 0, format7_info.max_size_x - left);
    height = nearest_value(height, format7_info.unit_pos_y, 0, format7_info.max_size_y - top);

    // set size
    err = dc1394_format7_set_image_size(camera,video_mode,width,height);
    if( err != DC1394_SUCCESS )
      throw VideoException("Could not set format7 size");

    // get the info again since many parameters depend on image size
    err = dc1394_format7_get_mode_info(camera, video_mode, &format7_info);
    if( err != DC1394_SUCCESS )
      throw VideoException("Could not get format7 mode info");

    // work out position of roi
    left = nearest_value(left, format7_info.unit_size_x, format7_info.unit_size_x, format7_info.max_size_x - width);
    top = nearest_value(top, format7_info.unit_size_y, format7_info.unit_size_y, format7_info.max_size_y - height);

    // set roi position
    err = dc1394_format7_set_image_position(camera,video_mode,left,top);
    if( err != DC1394_SUCCESS )
      throw VideoException("Could not set format7 size");

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
        throw VideoException("Could not set format7 packet size");

    } else {

      // setting packet size to get the desired frame rate according to the libdc docs
      // does not do the trick, so for now we support only max frame rate

        throw VideoException("In format 7 only max frame rate is currently supported");
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
        throw VideoException("Could not turn off frame rate");

    float value;
    err=dc1394_feature_get_absolute_value(camera,DC1394_FEATURE_FRAME_RATE,&value);
    if( err != DC1394_SUCCESS )
        throw VideoException("Could not get framerate");

    cout<<" framerate(shutter permitting):"<<value<<endl;

    //-----------------------------------------------------------------------
    //  setup capture
    //-----------------------------------------------------------------------

    err=dc1394_capture_setup(camera,dma_frames, DC1394_CAPTURE_FLAGS_DEFAULT);
    if( err != DC1394_SUCCESS )
        throw VideoException("Could not setup camera - check settings");

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
            throw VideoException("Unknown colour coding");
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
      throw VideoException("Unknown colour coding");
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
            throw VideoException("Could not start camera iso transmission");
        running = true;
    }
    }

    void FirewireVideo::Stop()
    {
    if( running )
    {
        // Stop transmission
        err = dc1394_video_set_transmission(camera,DC1394_OFF);
        if( err != DC1394_SUCCESS )
            throw VideoException("Could not stop the camera");
        running = false;
    }
    }

    void FirewireVideo::StopForOneShot()
    {
    if( running )
    {
        // Stop transmission
        err = dc1394_video_set_transmission(camera,DC1394_OFF);
        if( err != DC1394_SUCCESS )
            throw VideoException("Could not stop the camera");
        running = false;
    }
    //Call to eliminate spurious frames
    err = dc1394_video_set_one_shot(camera, DC1394_OFF);
    if( err != DC1394_SUCCESS )
        throw VideoException("Could not set one shot to OFF");
    FlushDMABuffer();
    }

    bool FirewireVideo::CheckOneShotCapable() {
    if (camera->one_shot_capable >0 ) return true;
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
        return true;
    }
    return false;
    }
    
    void FirewireVideo::FlushDMABuffer()
    {
    dc1394video_frame_t *frame;
    dc1394error_t err;

    while( 1 ) {
        err=dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_POLL, &frame);
        if (err != DC1394_SUCCESS) break;
        dc1394_capture_enqueue(camera, frame);
    }
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
        throw VideoException("Failed to get 1394 bus");
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
        throw VideoException("Failed to get 1394 bus");
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
            throw VideoException("Failed to get 1394 bus");

        err=dc1394_camera_enumerate (d, &list);
        if( err != DC1394_SUCCESS )
            throw VideoException("Failed to enumerate cameras");

        if (list->num == 0)
            throw VideoException("No cameras found");

        if( deviceid >= list->num )
            throw VideoException("Invalid camera index");

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
        throw VideoException("Failed to get 1394 bus");

    err=dc1394_camera_enumerate (d, &list);
    if( err != DC1394_SUCCESS )
        throw VideoException("Failed to enumerate cameras");

    if (list->num == 0)
        throw VideoException("No cameras found");

    if( deviceid >= list->num )
        throw VideoException("Invalid camera index");

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
    
    // move to header file and make vector
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
        
    void FirewireVideo::SetAllFeaturesAuto(){
                
    dc1394feature_modes_t modes;
        
    for (int i = 0; i <= 22; i++){
        
        if(feature[i] == DC1394_FEATURE_TRIGGER || feature[i] == DC1394_FEATURE_TRIGGER_DELAY ){
            dc1394_feature_set_power(camera, DC1394_FEATURE_TRIGGER, DC1394_OFF);       // these tend to break things so i've left them alone
            dc1394_feature_set_power(camera, DC1394_FEATURE_TRIGGER_DELAY, DC1394_OFF); // these tend to break things so i've left them alone
            break;
        }
        
        err = dc1394_feature_get_modes(camera, feature[i], &modes);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could not get modes for feature ", dc1394_feature_get_string(feature[i]));
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
            throw VideoException("Could not turn on ", dc1394_feature_get_string(feature));
        }
        
        err = dc1394_feature_set_mode(camera, feature, DC1394_FEATURE_MODE_AUTO);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could not set auto mode for ", dc1394_feature_get_string(feature));
        }
        
    }

    void FirewireVideo::SetFeatureManual(dc1394feature_t feature){
        
        err = dc1394_feature_set_power(camera, feature, DC1394_ON);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could not turn on ", dc1394_feature_get_string(feature));
        }
        
        err = dc1394_feature_set_mode(camera, feature, DC1394_FEATURE_MODE_MANUAL);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could not set manual mode for ", dc1394_feature_get_string(feature));
        }
        
    }

    void FirewireVideo::SetFeatureOn(dc1394feature_t feature){
        
        err = dc1394_feature_set_power(camera, feature, DC1394_ON);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could not turn on ", dc1394_feature_get_string(feature));
        }
        
    }

    void FirewireVideo::SetFeatureOff(dc1394feature_t feature){
        
        err = dc1394_feature_set_power(camera, feature, DC1394_OFF);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could not turn off ", dc1394_feature_get_string(feature));
        }
        
    }

    void FirewireVideo::SetFeatureValue(dc1394feature_t feature, float value){
        
        SetFeatureOn(feature);
        
        err = dc1394_feature_set_mode(camera, feature, DC1394_FEATURE_MODE_MANUAL);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could not set manual mode for ", dc1394_feature_get_string(feature));
        }
        
        err = dc1394_feature_set_absolute_control(camera, feature, DC1394_ON);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could not turn absolute mode off for ", dc1394_feature_get_string(feature));
        }
             
        err = dc1394_feature_set_absolute_value(camera, feature, value);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could set absolute value for ", dc1394_feature_get_string(feature));
        }
        
    }

    void FirewireVideo::SetFeatureQuant(dc1394feature_t feature, int value){
        
        err = dc1394_feature_set_mode(camera, feature, DC1394_FEATURE_MODE_MANUAL);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could not set manual mode for ", dc1394_feature_get_string(feature));
        }
        
        err = dc1394_feature_set_absolute_control(camera, feature, DC1394_OFF);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could not turn absolute mode off for ", dc1394_feature_get_string(feature));
        }
        
        err = dc1394_feature_set_value(camera, feature, value);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could set quantised value for ", dc1394_feature_get_string(feature));
        }
        
    }

    bool FirewireVideo::GetFeaturePower(dc1394feature_t feature){
        
        dc1394switch_t pwr;
        
        err = dc1394_feature_get_power(camera, feature, &pwr);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could check if power on/off for feature ", dc1394_feature_get_string(feature));
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
            throw VideoException("Could not check mode of feature ", dc1394_feature_get_string(feature));
        }
        
        if (mode == 737){
            return 0; // auto
        } 
        
        return 1; // manual
    }
            
    float FirewireVideo::GetFeatureValue(dc1394feature_t feature) const {
       
        float value;
        
        err = dc1394_feature_get_absolute_value(camera, feature, &value);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could set absolute value for ", dc1394_feature_get_string(feature));
        }

        return value;
        
    }

    int FirewireVideo::GetFeatureQuant(dc1394feature_t feature) const {
        
        uint32_t value;
        
        err = dc1394_feature_get_value(camera, feature, &value);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could not get quantised value for ", dc1394_feature_get_string(feature));
        }
        
        return value;
        
    }

    float FirewireVideo::GetFeatureValueMax(dc1394feature_t feature) const {
        
        float min, max;
       
        err = dc1394_feature_get_absolute_boundaries(camera, feature, &min, &max);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could get max absolute value for ", dc1394_feature_get_string(feature));
        }
        
        return max;
        
    }

    float FirewireVideo::GetFeatureValueMin(dc1394feature_t feature) const {
        
        float min, max;
        
        err = dc1394_feature_get_absolute_boundaries(camera, feature, &min, &max);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could get min absolute value for ", dc1394_feature_get_string(feature));
        }
        
        return min;
        
    }

    int FirewireVideo::GetFeatureQuantMax(dc1394feature_t feature) const {
        
        uint32_t min, max;
        
        err =  dc1394_feature_get_boundaries(camera, feature, &min, &max);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could get max quantised value for ", dc1394_feature_get_string(feature));
        }
        
        return max;
        
    }
    
    int FirewireVideo::GetFeatureQuantMin(dc1394feature_t feature) const {
        
        uint32_t min, max;
        
        err =  dc1394_feature_get_boundaries(camera, feature, &min, &max);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could get min quantised value for ", dc1394_feature_get_string(feature));
        }
        
        return min;
        
    }
                                           
    /*-----------------------------------------------------------------------
     *  WHITE BALANCE
     *-----------------------------------------------------------------------*/
        
    void FirewireVideo::SetSingleAutoWhiteBalance(){

        err = dc1394_feature_set_mode(camera, DC1394_FEATURE_WHITE_BALANCE, DC1394_FEATURE_MODE_ONE_PUSH_AUTO);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could not set manual white balance mode");
        }
    }
    
        
    void FirewireVideo::SetWhiteBalance(unsigned int Blue_U_val, unsigned int Red_V_val){

        err = dc1394_feature_set_mode(camera, DC1394_FEATURE_WHITE_BALANCE, DC1394_FEATURE_MODE_MANUAL);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could not set manual white balance mode");
        }

        err = dc1394_feature_whitebalance_set_value(camera, Blue_U_val, Red_V_val);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could not set white balance value");
        }

    }

    void FirewireVideo::GetWhiteBalance(unsigned int *Blue_U_val, unsigned int *Red_V_val) {

    err = dc1394_feature_whitebalance_get_value(camera,Blue_U_val, Red_V_val );
    if( err != DC1394_SUCCESS )
        throw VideoException("Failed to read white balance");
    }    

    /*-----------------------------------------------------------------------
     *  RESET 
     *-----------------------------------------------------------------------*/
        
    void FirewireVideo::ResetGamma()
    {
        err = dc1394_feature_set_power(camera, DC1394_FEATURE_GAMMA, DC1394_ON);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could not set gamma feature on");
        }
        
        err = dc1394_feature_set_absolute_control(camera, DC1394_FEATURE_GAMMA, DC1394_ON);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could not set absolute control for gamma");
        }
        
        err = dc1394_feature_set_absolute_value(camera, DC1394_FEATURE_GAMMA, 1.0);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could not set absolute gamma value to 1.0");
        }
        
    } 
    
    void FirewireVideo::ResetHue()
    {
        err = dc1394_feature_set_power(camera, DC1394_FEATURE_HUE, DC1394_ON);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could not set hue feature on");
        }
        
        err = dc1394_feature_set_value(camera, DC1394_FEATURE_HUE, 2048);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could not set absolute hue value to 2048 i.e. 0");
        }
        
    } 
        
    /*-----------------------------------------------------------------------
     *  TRIGGERS
     *-----------------------------------------------------------------------*/

    void FirewireVideo::SetInternalTrigger() 
    {
        err = dc1394_external_trigger_set_power(camera, DC1394_OFF);
        if ( err != DC1394_SUCCESS) {
            throw VideoException("Could not set internal trigger mode");
        }
        
    }

    void FirewireVideo::SetExternalTrigger(dc1394trigger_mode_t mode, dc1394trigger_polarity_t polarity, dc1394trigger_source_t source)
    {
    err = dc1394_external_trigger_set_polarity(camera, polarity);
    if (err != DC1394_SUCCESS) {
        throw VideoException("Could not set external trigger polarity");
    }

    err = dc1394_external_trigger_set_mode(camera, mode);
    if (err != DC1394_SUCCESS) {
        throw VideoException("Could not set external trigger mode");
    }

    err = dc1394_external_trigger_set_source(camera, source);
    if (err != DC1394_SUCCESS) {
        throw VideoException("Could not set external trigger source");
    }

    err = dc1394_external_trigger_set_power(camera, DC1394_ON);
    if (err != DC1394_SUCCESS) {
        throw VideoException("Could not set external trigger power");
    }
}

    /*-----------------------------------------------------------------------
     *  META DATA & LOOPUP TABLE ETC
     *-----------------------------------------------------------------------*/
        
    void FirewireVideo::SetMetaDataFlags(  int flags ) 
    {
        meta_data_flags = 0x80000000 | flags;

        err = dc1394_set_control_register(camera, 0x12f8, meta_data_flags);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could not set meta data flags");
        }
    }

    uint32_t FirewireVideo::GetMetaDataFlags() 
    {
        uint32_t value;
        err = dc1394_get_control_register(camera, 0x12f8, &value);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could not get meta data flags");
        }
        return value;
    }

    void FirewireVideo::ReadMetaData( unsigned char *image, MetaData *metaData ) {
        
    uint8_t* data = (uint8_t*)image;

    int offset = 0;

    metaData->flags = meta_data_flags;

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
        //Convert quantized value to absolute value from lookup table
        if(shutter_lookup_table) metaData->shutterAbs = shutter_lookup_table[metaData->shutterQuant];
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
        //Convert quantized value to absolute value from lookup table
        if(shutter_lookup_table) ret = shutter_lookup_table[shutterQuant];

    }
    return ret;
    }

    void FirewireVideo::CreateShutterLookupTable() {
        shutter_lookup_table = new float[4096];
        for (int i=0; i<4096; i++) {
            SetFeatureValue(DC1394_FEATURE_SHUTTER, i);
            shutter_lookup_table[i] = GetFeatureValue(DC1394_FEATURE_SHUTTER);
            cout << shutter_lookup_table[i] << endl;
        }
    }
    
        
    /*-----------------------------------------------------------------------
     *   RECORDING/SAVING
     *-----------------------------------------------------------------------*/
        
    void GetTimeStamp(char* date_time){
            
            time_t ltime;
            struct tm *Tm;
            
            ltime=time(NULL);
            Tm=localtime(&ltime);
            
            sprintf(date_time,"%d/%d/%d-%d:%d:%d",
                    Tm->tm_mday,
                    Tm->tm_mon,
                    Tm->tm_year,
                    Tm->tm_hour,
                    Tm->tm_min,
                    Tm->tm_sec);
            
        }
      
        
    bool FirewireVideo::RecordFrames(
                                    int frame_number, 
                                    unsigned char* image, 
                                    bool wait,
                                    bool jpg,
                                    bool hdr
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
 
        hdr ? SaveFile(frame_number, frame, "hdr-video", jpg) : SaveFile(frame_number, frame, "video", jpg);
        
        return true;       
        
    }              
                            
    bool FirewireVideo::RecordFramesOneShot(    
                                    int frame_number,
                                    unsigned char* image,
                                    bool jpg,
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

    hdr ? SaveFile(frame_number, frame, "hdr-video", jpg) : SaveFile(frame_number, frame, "video", jpg);

    return true;       

    }
        
    bool FirewireVideo::CaptureFrame(
                                     unsigned char* image, 
                                     bool wait,   
                                     bool jpg 
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
            
            SaveFile(1, frame, "single-frames", jpg);
            
            return true;     
            
        }
   
    bool FirewireVideo::SaveFile(
                                 int frame_number, 
                                 dc1394video_frame_t *frame, 
                                 const char* folder, 
                                 bool jpg
                                 )
    {
        char filename_ppm[128];
        char filename_jpg[128];
        dc1394video_mode_t video_mode = DC1394_VIDEO_MODE_640x480_RGB8;

        // create top directory
        mkdir(folder, 0755);
        
        // create ppm folder
        char ppm_folder[128];
        sprintf(ppm_folder, "%s/ppm", folder);
        mkdir(ppm_folder, 0755);
        
        // create jpg folder
        char jpg_folder[128];
        sprintf(jpg_folder, "%s/jpg", folder);
        mkdir(jpg_folder, 0755);
        
        // create path for ppm
        sprintf(filename_ppm, "./%s/ppm/%s%d%s", folder, "image0000", frame_number, ".ppm");
        
        CreatePPM(frame, filename_ppm, video_mode);
        
        if( jpg ){
            
            sprintf(filename_jpg, "./%s/jpg/%s%d%s", folder, "image0000", frame_number, ".jpg");
            CopyPPMToJPG(filename_ppm, filename_jpg);
            
            WriteExifData(this, filename_jpg);
                                    
        }
        
        cout << "Image Average Luminance cd/m^2: " << GetAvgLuminance(filename_jpg) << endl;      
        
        return true;
    }
    
    void FirewireVideo::CreatePPM(dc1394video_frame_t *frame, const char* filename, dc1394video_mode_t video_mode){
        
        FILE* imagefile;
        unsigned int width, height;
        
        err = dc1394_get_image_size_from_video_mode(
                                              camera, 
                                              video_mode, 
                                              &width, 
                                              &height
                                              );
        if (err != DC1394_SUCCESS){
            throw VideoException("Could not get image size from video mode");
        }
        
        uint64_t numPixels = height*width;
        imagefile = fopen(filename, "wb");
        
        if( imagefile == NULL) {
            perror( "Can't create output file");
        }
        
        fprintf(imagefile,"P6\n%u %u\n255\n", width, height);
        fwrite(frame->image, 1, numPixels*3, imagefile);
        fclose(imagefile);
        
    }
        

    // UNTESTED FUNCTION
    dc1394video_frame_t* FirewireVideo::ConvertToRGB(dc1394video_frame_t *original_frame)
    {
        dc1394video_frame_t *new_frame = NULL;
        new_frame->color_coding=DC1394_COLOR_CODING_RGB8; // should be abstracted
        dc1394_convert_frames(original_frame, new_frame);
        
        return new_frame;
        
    }
         
    /*-----------------------------------------------------------------------
     *  REPORTING
     *-----------------------------------------------------------------------*/
    
    void FirewireVideo::PrintCameraReport() 
    {
        // print unique id of camera(not node/slot on chipset)
        cout << camera->guid << endl; 
        
        // print camera details
        dc1394_camera_print_info(camera, stdout); 
        
        // print camera features
        dc1394_feature_print_all(&features, stdout);
        
    }
        
    /*-----------------------------------------------------------------------
     *  CONVENIENCE UTILITIES
     *-----------------------------------------------------------------------*/
        
    void FirewireVideo::GetBestSettings( dc1394video_mode_t video_mode, 
                                         dc1394framerate_t framerate 
                                       )
        {
    
        dc1394video_modes_t video_modes;
        dc1394color_coding_t coding;
        dc1394framerates_t framerates;
        int i;

        // get video modes:
        err = dc1394_video_get_supported_modes( camera, &video_modes );
            if( err != DC1394_SUCCESS )
                throw VideoException("Could not get supported modes");
        
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
            dc1394_log_error("Could not get a valid RGB8 mode");
        }
        
        err = dc1394_get_color_coding_from_video_mode(camera, video_mode, &coding);
        if( err != DC1394_SUCCESS )
            throw VideoException("Could not get colour coding");
        
        // get highest framerate
        err = dc1394_video_get_supported_framerates(camera, video_mode, &framerates);
        if( err != DC1394_SUCCESS )
            throw VideoException("Could not get frame rates");
            
        framerate = framerates.framerates[framerates.num-1]; // best video_mode set
        
    }
    

        // UNFINISHED
    void FirewireVideo::GetResponseFunction(){
        unsigned char* image = new unsigned char[SizeBytes()];
        FILE* file;
        char hdrgen_file[32] = "camera.hdrgen";
        
        float min, max, step_size, shutter;
        int no_steps = 10; // grab 10 frames
        
        SetAllFeaturesAuto();
        
        err = dc1394_feature_get_absolute_boundaries(camera, DC1394_FEATURE_SHUTTER, &min, &max);
        if( err != DC1394_SUCCESS ){
            throw VideoException("Failed to read shutter");
        }
        
        shutter = (max - min)/2; // first half of shutter range is black
        step_size = shutter/no_steps;
        
        cout << "Min: " << min << endl;
        cout << "Max: " << max << endl;
        cout << "Step Size: " << step_size << endl;
        
        err = dc1394_feature_set_mode(camera, DC1394_FEATURE_SHUTTER, DC1394_FEATURE_MODE_MANUAL);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could not set manual shutter mode");
        }
        
        err = dc1394_feature_set_absolute_control(camera, DC1394_FEATURE_SHUTTER, DC1394_ON);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could not set absolute control for shutter");
        }
        
        file = fopen(hdrgen_file, "wb");
        if( file == NULL) {
            perror( "Can't create hdrgen file");
        }
        
        for(int frame_number = 0; shutter <= max ; frame_number++){
            
            cout << "frame number: " << frame_number << endl;
            cout << "shutter value: " << shutter << endl;
            
            err = dc1394_feature_set_absolute_value(camera, DC1394_FEATURE_SHUTTER, shutter);
            if (err != DC1394_SUCCESS) {
                throw VideoException("Could not set shutter value");
            }
            
            boost::this_thread::sleep(boost::posix_time::seconds(1/30));
            dc1394video_frame_t *frame = NULL;
            
            dc1394_video_set_one_shot( camera, DC1394_ON );
            
            dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, &frame);  
            if( frame ){
                memcpy(image,frame->image,frame->image_bytes);
                dc1394_capture_enqueue(camera,frame);
            }
            
            SaveFile(frame_number, frame, "response-function", true);
            
            JpgToHDRGEN("response-function", file, frame_number);
            
            shutter += step_size;
            
        }
     
        fclose(file);
        
        err = dc1394_feature_set_mode(camera, DC1394_FEATURE_SHUTTER, DC1394_FEATURE_MODE_AUTO);
        if (err != DC1394_SUCCESS) {
            throw VideoException("Could not set auto shutter mode");
        }

        // run pfs calibrate
        cout << "Generating response function" << endl;
        
        //run gnu plot
        cout << "DONE" << endl;
    
        delete[] image;
        
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
          throw VideoException("iso speed not valid");
        }

        return bus_period;
        }

    }

