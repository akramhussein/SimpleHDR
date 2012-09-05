/* This file is part of the Pangolin HDR extension project
 *
 * http://github.com/akramhussein/hdr
 *
 * Copyright (c) 2012 Akram Hussein
 *
 * Original source code by Steven Lovegrove
 *
 * http://github.com/stevenlovegrove/Pangolin
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

    #ifndef PANGOLIN_FIREWIRE_H
    #define PANGOLIN_FIREWIRE_H

    #include <stdio.h>
    #include <stdint.h>
    #include <stdlib.h>
    #include <inttypes.h>
    #include <sys/stat.h>
    #include <time.h>
    #include <map>

    #include <pangolin/pangolin.h>
    #include <pangolin/video.h>
    #include <pangolin/timer.h>


    #include <dc1394/dc1394.h>

    #include <jpeglib.h>

    #include <boost/thread/thread.hpp>
    #include <boost/property_tree/ptree.hpp>
    #include <boost/property_tree/ini_parser.hpp>

    #ifndef _WIN32
    #include <unistd.h>
    #endif

    namespace pangolin
    {
       
    std::string Dc1394ColorCodingToString(dc1394color_coding_t coding);

    dc1394color_coding_t Dc1394ColorCodingFromString(std::string coding);

    void Dc1394ModeDetails(dc1394video_mode_t mode, unsigned& w, unsigned& h, std::string& format );

    class FirewireFrame
    {
    friend class FirewireVideo;
    public:
    bool isValid() { return frame; }
    unsigned char* Image() { return frame ? frame->image : 0; }
    unsigned Width() const { return frame ? frame->size[0] : 0; }
    unsigned Height() const { return frame ? frame->size[1] : 0; }

    protected:
    FirewireFrame(dc1394video_frame_t* frame) : frame(frame) {}
    dc1394video_frame_t *frame;
    };
        
    struct Guid
    {
        Guid(uint64_t guid):guid(guid){}
        uint64_t guid;
    };

    struct MetaData 
    {
        uint32_t flags;
        unsigned int brightness;
        unsigned int auto_exposure;
        unsigned int whitebalance_u_b, whitebalance_v_r;
        uint32_t timestamp, frame_count;
        uint32_t shutterQuant, gain;
        float shutterAbs;
        bool abs_on; 

        //TODO: Add strobe, GPIO and ROI functionality if needed.
        uint32_t strobe, gpio, roi;

        void copy_from( MetaData *in ) {
            flags=in->flags;
            brightness=in->brightness;
            auto_exposure=in->auto_exposure;
            whitebalance_u_b=in->whitebalance_u_b;
            whitebalance_v_r=in->whitebalance_v_r;
            timestamp=in->timestamp;
            frame_count=in->frame_count;
            shutterQuant=in->shutterQuant;
            gain=in->gain;
            shutterAbs=in->shutterAbs;
            strobe = in->strobe;
            gpio=in->gpio;
            roi=in->roi;
        }
    };

    typedef enum {
    META_TIMESTAMP = 1,
    META_GAIN = 2,
    META_SHUTTER = 4,
    META_BRIGHTNESS = 8,
    META_EXPOSURE = 16,
    META_WHITE_BALANCE = 32,
    META_FRAME_COUNTER = 64,
    META_STROBE = 128,
    META_GPIO_PIN_STATE = 256,
    META_ROI_POSITION = 512,
    META_ALL = 1023,
    META_ABS = 32678, 
    META_ALL_AND_ABS = 33791,
    } meta_flags;
   
    class FirewireVideo : public VideoInterface
    {
    public:
    const static int MAX_FR = -1;
    const static int EXT_TRIG = -1;

    FirewireVideo(
    unsigned deviceid = 0,
    dc1394video_mode_t video_mode = DC1394_VIDEO_MODE_640x480_RGB8,
    dc1394framerate_t framerate = DC1394_FRAMERATE_30,
    dc1394speed_t iso_speed = DC1394_ISO_SPEED_400,
    int dma_buffers = 75
    );

    FirewireVideo(
    Guid guid,
    dc1394video_mode_t video_mode = DC1394_VIDEO_MODE_640x480_RGB8,
    dc1394framerate_t framerate = DC1394_FRAMERATE_30,
    dc1394speed_t iso_speed = DC1394_ISO_SPEED_400,
    int dma_buffers = 75
    );

    FirewireVideo(
      Guid guid,
      dc1394video_mode_t video_mode,
      int framerate,
      uint32_t width, uint32_t height,
      uint32_t left, uint32_t top,
      dc1394speed_t iso_speed,
      int dma_buffers, bool reset_at_boot=false
    );

    FirewireVideo(
      unsigned deviceid,
      dc1394video_mode_t video_mode,
      int framerate,
      uint32_t width, uint32_t height,
      uint32_t left, uint32_t top,
      dc1394speed_t iso_speed,
      int dma_buffers, bool reset_at_boot=false
    );

    ~FirewireVideo();

    /* Implement VideoSource::Width()
     @return width
     */
    unsigned Width() const { return width; }

    /* Implement VideoSource::Height()
     @return height
     */ 
    unsigned Height() const { return height; }

    /*Implement VideoSource::SizeBytes()
     @return byte size
     */  
    size_t SizeBytes() const;

    /*Implement VideoSource::PixFormat()
     @return string of format
     */
     std::string PixFormat() const;

    /*Implement VideoSource::Start()
    @exception dc1394 error
     */
    void Start();

    /* Implement VideoSource::Stop()
     @exception dc1394 error
     */  
    void Stop();

    /* Clear the DMA buffer
    @exception dc1394 error
     */
    void FlushDMABuffer();
        
    /*-----------------------------------------------------------------------
     * MULTI/ONE SHOT CONTROL
     *-----------------------------------------------------------------------*/
        
    /* check for multi-shot
     @return bool flag (yes = true)
     @exception dc1394 error
     */
    bool CheckMultiShotCapable();
        
    /* Turn Multi-shot On
     @param num of frames
     @exception dc1394 error
     */
    void SetMultiShotOn(int num_frames);
    
    /* Turn Multi-shot Off
     @exception dc1394 error
     */
    void SetMultiShotOff();
        
    //! Stop the video stream before using One-shot mode
    void StopForOneShot();

    /* Grab one shot (iso-transmission must be off - call StopForOneShot first)
     @param image buffer
     @return bool flag
     @exception dc1394 error
     */
     bool GrabOneShot(unsigned char* image);
    
    /* Check to see if camera is one-shot capable
     @return bool flag (yes = true) 
    @exception dc1394 error 
     */  
    bool CheckOneShotCapable();

    /*-----------------------------------------------------------------------
     * REGISTERS - META DATA, HDR & LOOPUP TABLE ETC
     *-----------------------------------------------------------------------*/
        
    /* set the meta data flags to be included in image data
     @param flags
     @exception dc1394 error  
     */ 
    void SetMetaDataFlags(int flags);
    
    /* return the current meta data flags from camera
     @return flags
     @exception dc1394 error
     */
    uint32_t GetMetaDataFlags();
    
    /* set HDR register on/off 
    @param bool flag (on = true)
    @exception dc1394 error
     */  
    void SetHDRRegister(bool power);

    /* get HDR register flags
     @return flags
     @excepion dc1394 error
     */
    uint32_t GetHDRFlags();

    /* set HDR register shutter flags
     @param hdr bank 1 flags
     @param hdr bank 2 flags
     @param hdr bank 3 flags
     @param hdr bank 4 flags
     @excepion dc1394 error
     */
    void SetHDRShutterFlags(uint32_t shut0, uint32_t shut1, uint32_t shut2, uint32_t shut3);
    
    /* get HDR register shutter flags (pass by reference)
     @param hdr bank 1 flags
     @param hdr bank 2 flags
     @param hdr bank 3 flags
     @param hdr bank 4 flags
     @excepion dc1394 error
     */
    void GetHDRShutterFlags(uint32_t &shut0, uint32_t &shut1, uint32_t &shut2, uint32_t &shut3); 
        
    /* set HDR register gain flags
     @param hdr bank 1 flags
     @param hdr bank 2 flags
     @param hdr bank 3 flags
     @param hdr bank 4 flags
     @excepion dc1394 error
     */
    void SetHDRGainFlags(uint32_t gain0, uint32_t gain1, uint32_t gain2, uint32_t gain3);
    
    /* get HDR register gain flags (pass by reference)
     @param hdr bank 1 flags
     @param hdr bank 2 flags
     @param hdr bank 3 flags
     @param hdr bank 4 flags
     @excepion dc1394 error
     */
    void GetHDRGainFlags(uint32_t &gain0, uint32_t &gain1, uint32_t &gain2, uint32_t &gain3); 
        
    /* read the meta data from an image according to meta flags
     @param image buffer
     @param metadata (by reference)
     */
    void ReadMetaData( unsigned char *image, MetaData *metaData );
    
    /* return time stamp from image data
     @param image buffer
     @return time stamp (UNIX type)
     */
    uint32_t ReadTimeStamp( unsigned char *image );
        
    /* read image data and check map/lookup to get absolute shutter value
     @param image buffer
     @return abs shutter time
     */
    float ReadShutter( unsigned char *image );
        
    /* create lookup table to convert quantised shutter values to absolute values
     */
     void CreateShutterLookupTable();
        
    /* create lookup hash maps to convert quantised shutter values to absolute values and vice versa
     */
    void CreateShutterMaps();
     
    /**
     get quantised shutter value for corresponding absolute value
     @param abs value
     @return quant value
     */   
    int GetShutterMapQuant(float val);
        
    /**
     get absolute shutter value for corresponding quantised value
     @param quant value
     @return abs value
     */   
    float GetShutterMapAbs(int val);
    
    /**
     print shutter absolute map <int,float>
     */   
    void PrintShutterMapAbs();
        
    /**
     print shutter quantised map <float,int> 
     */
    void PrintShutterMapQuant();
        
    /*-----------------------------------------------------------------------
     *  FRAME GRAB
     *-----------------------------------------------------------------------*/

    /**
     Implement VideoSource::GrabNext()
     @param image buffer
     @param wait flag
     @return bool flag
     */ 
    bool GrabNext( unsigned char* image, bool wait = true );

    /**
     Implement VideoSource::GrabNewest()
     @param image buffer
     @param wait flag
     @return bool flag
     */ 
    bool GrabNewest( unsigned char* image, bool wait = true );

    /**
     Return object containing reference to image data within
     DMA buffer. The FirewireFrame must be returned to
     signal that it can be reused with a corresponding PutFrame()
     @param wait flag
     @return firewire frame
     */ 
    FirewireFrame GetNext(bool wait = true);

    /**
     Return object containing reference to newest image data within
     DMA buffer discarding old images. The FirewireFrame must be
     returned to signal that it can be reused with a corresponding PutFrame()
     @param wait flag
     @return firewire frame
     */  
    FirewireFrame GetNewest(bool wait = true);
 
    /**
     return FirewireFrame object. Data held by FirewireFrame is
      nvalidated on return.
     @param firewire frame
     */   
    void PutFrame(FirewireFrame& frame);
        
    /*-----------------------------------------------------------------------
     *  FEATURE CONTROL
     *-----------------------------------------------------------------------*/    

    /**
     set all features to auto mode
     @exception dc1394 error
     */
    void SetAllFeaturesAuto();
     
    /**
     set all features to manual mode
     @exception dc1394 error
     */
    void SetAllFeaturesManual();

    /**
     set feature to auto mode
     @param feature
     @exception dc1394 error
     */
    void SetFeatureAuto(dc1394feature_t feature);

    /**
     set feature to manual mode
     @param feature
     @exception dc1394 error
     */
    void SetFeatureManual(dc1394feature_t feature);
    
    /**
     set feature to on
     @param feature
     @exception dc1394 error
     */
    void SetFeatureOn(dc1394feature_t feature);
    
    /**
     set feature to off
     @param feature
     */
    void SetFeatureOff(dc1394feature_t feature);
           
    /**
     set feature absolute value
     @param feature
     @param value
          @exception dc1394 error
     */
    void SetFeatureValue(dc1394feature_t feature, float value);
    
    /**
     set feature quantised value
     @param feature
     @param value
     @exception dc1394 error
    */
    void SetFeatureQuant(dc1394feature_t feature, int value);
    
    /**
     get feature on/off
     @param feature
     @return bool flag (on/off)
     @exception dc1394 error
     */
    bool GetFeaturePower(dc1394feature_t feature);
        
    /**
     get feature enabled mode (auto [0] or manual [1])
     @param feature
     @param mode
     @exception dc1394 error
     */
    int GetFeatureMode(dc1394feature_t feature) const;
        
    /**
     get feature absolute value
     @param feature
     @return absolute value
     @exception dc1394 error
     */
    float GetFeatureValue(dc1394feature_t feature) const;
    
    /**
     get feature quantised value
     @param feature
     @return quantised value
     @exception dc1394 error
     */    
    int GetFeatureQuant(dc1394feature_t feature) const;
    

    /**
     get feature absolute max value
     @param feature
     @return max absolute value
     @exception dc1394 error
     */
    float GetFeatureValueMax(dc1394feature_t feature) const;

    /**
     get feature absolute min value
     @param feature
     @return min absolute value
          @exception dc1394 error
     */
    float GetFeatureValueMin(dc1394feature_t feature) const;
        
    /**
     get feature quantised max value
     @param feature
     @return max quantised value
     @exception dc1394 error
     */
    int GetFeatureQuantMax(dc1394feature_t feature) const;
    
    /**
     get feature quantised min value
     @param feature
     @return min quantised value
     @exception dc1394 error
     */
    int GetFeatureQuantMin(dc1394feature_t feature) const;
    
    /**
     reset brightness to 0
     @exception dc1394 error
     */
    void ResetBrightness();
        
    /**
     reset gamma to 1.0
     @exception dc1394 error
     */
    void ResetGamma();
    
    /**
    reset hue to 0
    @exception dc1394 error
     */
    void ResetHue();

    /*-----------------------------------------------------------------------
     *  WHITE BALANCE CONTROLS
     *-----------------------------------------------------------------------*/
      
    /**
     set white balance to auto mode
     @exception dc1394 error
     */
    void SetSingleAutoWhiteBalance();
           
    /**
     set white balance blue/U value and red/v value
     @param blue/u value
     @param red/v value
     @exception dc1394 error
     */
    void SetWhiteBalance(unsigned int u_b_value, unsigned int v_r_value);

    /**
     get white balance blue/U value and red/v value
     @param pass by reference blue/u value 
     @param pass by reference red/v value
     @exception dc1394 error
     */
    void GetWhiteBalance(unsigned int *Blue_U_val, unsigned int *Red_V_val);
     
    /**
     get white balance blue/U value
     @returns white balance blue/u value
     @exception dc1394 error
     */
    int GetWhiteBalanceBlueU();
   
    /**
     get white balance red/V value
     @returns white balance red/v value
     @exception dc1394 error
     */
    int GetWhiteBalanceRedV();
        
    /*-----------------------------------------------------------------------
     *  TRIGGERS
     *-----------------------------------------------------------------------*/

    /**
     set the trigger to internal, i.e. determined by video mode
     @exception dc1394 error
     */
    void SetInternalTrigger();

    /**
     set the trigger to internal, i.e. determined by video mode
     @param trigger mode
     @param trigger polarity
     @param trigger source
     @exception dc1394 error
     */
    void SetExternalTrigger(
      dc1394trigger_mode_t mode=DC1394_TRIGGER_MODE_0,
      dc1394trigger_polarity_t polarity=DC1394_TRIGGER_ACTIVE_HIGH,
      dc1394trigger_source_t source=DC1394_TRIGGER_SOURCE_0
    );      
    
    /*-----------------------------------------------------------------------
     *  RECORDING/SAVING
     *-----------------------------------------------------------------------*/
        
    /**
     record multiple frames
     @param frame number
     @param image buffer
     @param wait or poll for image from dma
     @param jpeg or not
     @param hdr frame or not
     @returns bool flag
     */
    bool RecordFrames(     
                      int frame_number,      // current frame number
                      unsigned char* image,  // empty image buffer -- to go in future
                      bool wait,             // defaults = true
                      bool jpeg = true,      // true = jpeg, false = ppm
                      bool hdr = false       // hdr folder or not
                      ); 
        
    /**
     records multiple frames using 'One Shot' mode
     @param frame number
     @param image buffer
     @param jpeg?
     @param hdr?
     @return bool flag if success/fail
     @exception dc1394 error
     */
    bool RecordFramesOneShot(   
                            int frame_number,     // current frame number
                            unsigned char* image, // empty image buffer -- to go
                            bool jpeg = true,     // true = jpeg, false = ppm
                            bool hdr = true // hdr folder or not
                            );
                
    /**
     saves one frame
     @param frame number
     @param image buffer
     @param wait for frame or poll
     @param jpeg?
     @return bool flag if success/fail
     @exception dc1394 error or exiv error
     */ 
    bool CaptureFrame(
                      int frame_number,
                      unsigned char* image,  // empty image buffer -- to go in future
                      bool wait,              // defaults = true
                      bool jpeg = true        //true = jpeg, false = ppm
                     );

    /**
     saves one framewith 'One Shot' mode
     @return bool flag if success/fail
     @param frame number
     @param image buffer
     @param jpeg?
     @exception dc1394 error or exiv error
     */ 
    bool CaptureFrameOneShot(
                          int frame_number,
                          unsigned char* image, // empty image buffer -- to go 
                          bool jpeg = true      // true = jpeg, false = ppm
                      );
        
    /**
     HDR Frame Capture
     @param image buffer
     @param number of frames to capture
     @param quantised shutter array
     @exception dc1394 error or exiv error
     */ 
    void CaptureHDRFrame(unsigned char* image, int n, uint32_t shutter[]);
         
    /**
     grab and save indidivdual frame with time stamp
     @exception dc1394 error or exiv error
     */ 
    void SaveSingleFrame(unsigned char* image);

    /**
     save image file to ppm or jpeg
     @param frame numner    
     @param dc1394 frame
     @param file path
     @param jpeg?
     @exception dc1394 error or exiv error
     */   
    bool SaveFile(    
                    int frame_number,           // current frame number
                    dc1394video_frame_t frame,  // frame buffer (copy - threading)
                    const char* path,           // folder name
                    bool jpeg = true            // true = jpeg, false = ppm
                );

    /**
     save normal video
     @exception dc1394 error
     */    
    void SaveVideo();

    /**
     save HDR video
     @param frame number
     @exception dc1394 error
     */  
    void SaveHDRVideo(int frame_number);
        
    /**
     convert dc1394 frame to RGB from YUV
     @param dc1394 frame
     @return new converted dc1394 frame
     @exception dc1394 error
     */  
    dc1394video_frame_t* ConvertToRGB(dc1394video_frame_t *original_frame);
    
    
    /*-----------------------------------------------------------------------
     *  AUTOMATIC EXPOSURE CONTROL
     *-----------------------------------------------------------------------*/

    /**
     returns updated shutter time using SimpleHDR AEC algorithm for HDR mode
     @param image buffer
     @param current shutter time in us (abs)
     @param under or over bool flag
     @returns new shutter time
     */
    float AEC(unsigned char *image, float st, bool under_over);
        
    /**
     get loaded aec value from config file 
     @returns attribute value
     */
    float GetAECValue(std::string attribute);
    
              
    /*-----------------------------------------------------------------------
     *  CAMERA UTILITIES
     *-----------------------------------------------------------------------*/

    /**
     get pointer to camera
     @return dc1394 camera
     @exception dc1394 error
     */  
    dc1394camera_t *GetCamera() { return camera; }

    /**
     get camera feature set
     @return dc1394 feature set
     @exception dc1394 error
     */  
    dc1394featureset_t *GetFeatures() { return &features; }

    /**
     get video mode
     @return  dc1394 video mode
     @exception dc1394 error
     */     
    dc1394video_mode_t *GetVideoMode() { return &video_mode; }

    /**
     get camera vendor as string
     @return string
     @exception dc1394 error
     */     
    std::string GetCameraVendor() const { return camera->vendor; }
    
    /**
     get camera model as string
     @return string
     @exception dc1394 error
     */ 
    std::string GetCameraModel() const { return camera->model; }
        
    /**
     print full camera report
     @exception dc1394 error
     */ 
    void PrintCameraReport();
        
    /**
     get best video mode and frame rate for RGB colour space
     @param dc1394 video mode as reference
     @param dc1394 frame rate as reference
     @exception dc1394 error
     */ 
    void GetBestSettings(dc1394video_mode_t &video_mode, dc1394framerate_t &framerate);
        
    /*-----------------------------------------------------------------------
     *  FILE UTILITIES
     *-----------------------------------------------------------------------*/
        
    /**
     generate camera response function and save to file
     @exception dc1394 error
     */ 
    void GetResponseFunction();
        
    /**
     check if response function exists
     @return bool flag (yes = true)
     @exception dc1394 error
     */ 
    bool CheckResponseFunction();
        
    /**
     get number of bytes to skip when scanning frame
     @return number of bytes
     */ 
    int GetMetaOffset(); 
            
    /**
     load config file
     @exception boost error
     */ 
    void LoadConfig();
        
    /**
     check config is loaded
     @return bool flag (yes = true)
     */ 
    bool CheckConfigLoaded();
        
    /**
     get loaded configuration file attribute value
     @param attribute string
     @return attribute string
     */ 
    std::string GetConfigValue(std::string attribute);  
        
    /**
     set loaded configuration file attribute value
     @param attribute string
     @param value string
     */ 
    void SetConfigValue(std::string attribute, std::string value); 
        

    /**
     get time stamped string
     @param pass as reference
     */ 
    void GetTimeStamp(char* time_stamp);

    /**
     get padded number string
     @return padded number string
     */ 
    char* PadNumber(int frame_number);
        
    protected:

    void init_camera(
    uint64_t guid, int dma_frames,
    dc1394speed_t iso_speed,
    dc1394video_mode_t video_mode,
    dc1394framerate_t framerate
    );

    void init_format7_camera(
      uint64_t guid, int dma_frames,
      dc1394speed_t iso_speed,
      dc1394video_mode_t video_mode,
      int framerate,
      uint32_t width, uint32_t height,
      uint32_t left, uint32_t top, bool reset_at_boot
    );

    static int nearest_value(int value, int step, int min, int max);
    static double bus_period_from_iso_speed(dc1394speed_t iso_speed);
        
    bool running;
    dc1394camera_t *camera;
    unsigned width, height, top, left;
    dc1394featureset_t features;
    dc1394_t * d;
    dc1394camera_list_t * list;
    mutable dc1394error_t err;
    dc1394video_mode_t video_mode;
        
    uint32_t meta_data_flags;
    bool hdr_register; // 1 = on
    
    float* shutter_lookup_table;

    std::map<int,float> shutter_abs_map;
    std::map<float,int> shutter_quant_map; 
        
    std::map<std::string, std::string> config;
      
    std::map<std::string, float> aec_values;
        
    };

    }

    #endif // PANGOLIN_FIREWIRE_H
