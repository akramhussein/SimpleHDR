 /* This file is part of the Pangolin HDR extension project
 *
 * http://github.com/akramhussein/hdr
 *
 * Copyright (c) 2012 Akram Hussein
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

#include "image.h"

using namespace std;

namespace pangolin
{
    // bool CreatePPM(unsigned char* image, int w, int h, const char* filename)
    bool CreatePPM(unsigned char* image, int width, int height, const char* filename)
    {
        
        FILE* imagefile;
        
        uint64_t numPixels = height*width;
        imagefile = fopen(filename, "wb");
        
        if( imagefile == NULL) {
            perror( "[IMAGE ERROR]: Can't create output file");
            return false;
        }
        
        fprintf(imagefile,"P6\n%u %u\n255\n", width, height);
        fwrite(image, 1, numPixels*3, imagefile);
        fclose(imagefile);
        
        return true;
        
    }
    
    bool CreateJPEG(unsigned char* image, int width, int height, const char* filename)
    {            
    
        // this is a pointer to one row of image data
        JSAMPROW row_pointer[1];
        FILE *imagefile = fopen( filename, "wb" );

        struct jpeg_compress_struct cinfo;
        struct jpeg_error_mgr jerr;
             
        if ( !imagefile ) {
            printf("[IMAGE ERROR]: Error opening output jpeg file %s\n!", filename );
            return false;
        }
        
        cinfo.err = jpeg_std_error( &jerr );
        jpeg_create_compress(&cinfo);
        jpeg_stdio_dest(&cinfo, imagefile);
        
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
            row_pointer[0] = &image[ cinfo.next_scanline * cinfo.image_width *  cinfo.input_components];
            jpeg_write_scanlines( &cinfo, row_pointer, 1 );
        }
        // similar to read file, clean up after we're done compressing 
        jpeg_finish_compress( &cinfo );
        jpeg_destroy_compress( &cinfo );
        fclose( imagefile );
        
        return true;
    }
    
    bool LoadJPEG(unsigned char* image_buffer, const char* filename){
        
        struct jpeg_decompress_struct cinfo;
        struct jpeg_error_mgr jerr;
        
        FILE *infile;		
        JSAMPARRAY buffer;	
        int row_stride;		
        
        if ((infile = fopen(filename, "rb")) == NULL)
        {
            printf("[IMAGE ERROR]: Error opening input jpeg file %s\n!", filename );
            return false;
        }
        
        cinfo.err = jpeg_std_error( &jerr );
        jpeg_create_decompress(&cinfo);
        
        jpeg_stdio_src(&cinfo, infile);
        
        (void) jpeg_read_header(&cinfo, TRUE);
        (void) jpeg_start_decompress(&cinfo);
        row_stride = cinfo.output_width *cinfo.output_components;
        
        buffer = (*cinfo.mem->alloc_sarray)
        ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
        
        while (cinfo.output_scanline < cinfo.output_height)
        {
            JDIMENSION read_now = jpeg_read_scanlines(&cinfo, buffer, 1);
            memcpy(&image_buffer[(cinfo.output_scanline - read_now) *cinfo.output_width *cinfo.output_components], buffer[0], row_stride);
        }
        
        jpeg_finish_decompress(&cinfo);
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        
        return true;
        
    }
    
    void CopyFormatToFormat(const char* from_filename, const char* to_filename)
    {
        
        try {
            Magick::Image img;
            img.read(from_filename);
            img.write(to_filename);
        } 
        catch( Magick::ErrorFileOpen &error ) {
            // Process Magick++ file open error
            cerr << "[IMAGE ERROR]: " << error.what() << endl;
        }

    }

    void WriteExifData(const pangolin::FirewireVideo* video, const std::string& filename)
    {
      
        Exiv2::ExifData exifData;
        
        try {
            
            exifData["Exif.Image.Make"] = video->GetCameraVendor();
            exifData["Exif.Image.Model"] = video->GetCameraModel();
            //exifData["Exif.Photo.ExposureTime"] = Exiv2::floatToRationalCast(metaData->shutterAbs);	
            exifData["Exif.Photo.ExposureTime"] = Exiv2::floatToRationalCast(video->GetFeatureValue(DC1394_FEATURE_SHUTTER));	
            exifData["Exif.Photo.ExposureBiasValue"] = Exiv2::floatToRationalCast(video->GetFeatureValue(DC1394_FEATURE_EXPOSURE)); // Exposure Value (EV)
            exifData["Exif.Photo.ColorSpace"] = uint16_t(1); //sRGB   
            exifData["Exif.Photo.WhiteBalance"] = uint16_t(video->GetFeatureMode(DC1394_FEATURE_WHITE_BALANCE));   // 0=auto,1=man
                                                                                                                  
            /*
            exifData["Exif.Photo.GainControl"] = uint16_t(video->GetFeatureValue(DC1394_FEATURE_GAIN));
            exifData["Exif.Photo.Saturation"] = uint16_t(video->GetFeatureValue(DC1394_FEATURE_SATURATION));
            exifData["Exif.Photo.Sharpness"] = uint16_t(video->GetFeatureValue(DC1394_FEATURE_SHARPNESS));
            */
            
            Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(filename);
            assert(image.get() != 0);
            
            image->setExifData(exifData);
            image->writeMetadata();
            
        }
        catch (Exiv2::AnyError& e) {
            cout << "[IMAGE] Exiv error - '" << e << "'\n";
        }
        
        //cout << "[IMAGE]: Exif data written from camera settings" << endl;
    }

    void WriteExifDataFromImageMetaData(MetaData *metaData, const std::string& filename)
    {
        
        Exiv2::ExifData exifData;
        
        try {
            
            // turned off camera getting because its slow
            
            //exifData["Exif.Image.Make"] = video->GetCameraVendor();
            //exifData["Exif.Image.Model"] = video->GetCameraModel();
            
            exifData["Exif.Photo.FNumber"] = Exiv2::Rational(7, 5); // hard coded -- change to config file later
            exifData["Exif.Photo.ExposureTime"] = Exiv2::floatToRationalCast(metaData->shutterAbs);	
            
            /*
             exifData["Exif.Photo.ExposureTime"] = Exiv2::floatToRationalCast(video->GetFeatureValue(DC1394_FEATURE_SHUTTER));	
             exifData["Exif.Photo.ExposureBiasValue"] = Exiv2::floatToRationalCast(metaData->auto_exposure); // quant value not abs
             exifData["Exif.Photo.WhiteBalance"] = uint16_t(video->GetFeatureMode(DC1394_FEATURE_WHITE_BALANCE));   // 0=auto,1=man
             exifData["Exif.Photo.GainControl"] = uint16_t(video->GetFeatureValue(DC1394_FEATURE_GAIN));
             exifData["Exif.Photo.Saturation"] = uint16_t(video->GetFeatureValue(DC1394_FEATURE_SATURATION));
             exifData["Exif.Photo.Sharpness"] = uint16_t(video->GetFeatureValue(DC1394_FEATURE_SHARPNESS));
             */
            
            exifData["Exif.Photo.ColorSpace"] = uint16_t(1); //sRGB
            

            Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(filename);
            assert(image.get() != 0);
            
            image->setExifData(exifData);
            image->writeMetadata();
            
        }
        catch (Exiv2::AnyError& e) {
            cout << "[IMAGE ERROR]: Exiv error - '" << e << "'\n";
        }
        
        //cout << "[IMAGE]: Exif data written from meta data" << endl;
        
    }
    
    float GetAvgLuminance(const std::string& filename)
    {
        try
        {

            Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(filename);
            image->readMetadata();
            
            Exiv2::ExifData exifData;
            exifData = image->exifData();
            
            if (exifData.empty()){
                throw VideoException("[IMAGE ERROR]: Exiv Error - No Exif Data in image");
            }

            Exiv2::ExifData::const_iterator _expo = exifData.findKey(Exiv2::ExifKey("Exif.Photo.ExposureTime"));
            Exiv2::ExifData::const_iterator _expo2 = exifData.findKey(Exiv2::ExifKey("Exif.Photo.ShutterSpeedValue"));
            Exiv2::ExifData::const_iterator _iso  = exifData.findKey(Exiv2::ExifKey("Exif.Photo.ISOSpeedRatings"));
            Exiv2::ExifData::const_iterator _fnum = exifData.findKey(Exiv2::ExifKey("Exif.Photo.FNumber"));
            Exiv2::ExifData::const_iterator _fnum2 = exifData.findKey(Exiv2::ExifKey("Exif.Photo.ApertureValue"));
            
            // default not valid values
            float expo  = -1;
            float iso   = -1;
            float fnum  = -1;
            
            if (_expo != exifData.end())
            {
                expo=_expo->toFloat();
            }
            else if (_expo2 != exifData.end())
            {
                long num=1, div=1;
                double tmp = std::exp(std::log(2.0) * _expo2->toFloat());
                if (tmp > 1)
                {
                    div = static_cast<long>(tmp + 0.5);
                }
                else
                {
                    num = static_cast<long>(1/tmp + 0.5);
                }
                expo = static_cast<float>(num)/static_cast<float>(div);
            }
            
            if (_fnum != exifData.end())
            {
                fnum = _fnum->toFloat();
            }
            else if (_fnum2 != exifData.end())
            {
                fnum = static_cast<float>(std::exp(std::log(2.0) * _fnum2->toFloat() / 2));
            }
            // some cameras/lens DO print the fnum but with value 0, and this is not allowed for ev computation purposes.
            if (fnum == 0)
                return -1;
            
            //if iso is found use that value, otherwise assume a value of iso=100. (again, some cameras do not print iso in exif).
            if (_iso == exifData.end())
            {
                iso = 100.0;
            }
            else
            {
                iso = _iso->toFloat();
            }
            
            //At this point the three variables have to be != -1
            if (expo!=-1 && iso!=-1 && fnum!=-1)
            {
                // 		std::cerr << "expo=" << expo << " fnum=" << fnum << " iso=" << iso << " |returned=" << (expo * iso) / (fnum*fnum*12.07488f) << std::endl;
                return ( (expo * iso) / (fnum*fnum*12.07488f) );
            }
            else
            {
                return -1;
            }
        }
        catch (Exiv2::AnyError& e)
        {
            throw VideoException("Exiv Error");
        }
    }
    
    void CopyExifData(const std::string& from, const std::string& to, bool dont_overwrite)
    {
        
        Exiv2::Image::AutoPtr sourceimage = Exiv2::ImageFactory::open(from);
        Exiv2::Image::AutoPtr destimage = Exiv2::ImageFactory::open(to);
        
        sourceimage->readMetadata();
        Exiv2::ExifData &src_exifData = sourceimage->exifData();
        if (src_exifData.empty())
        {
            throw Exiv2::Error(1, "No exif data found in the image");
        }
        if (dont_overwrite)
        {
            destimage->readMetadata();
            Exiv2::ExifData &dest_exifData = destimage->exifData();
            Exiv2::ExifData::const_iterator end_src = src_exifData.end();
            
            for (Exiv2::ExifData::const_iterator i = src_exifData.begin(); i != end_src; ++i)
            {
				//check if current source key exists in destination file
				Exiv2::ExifData::iterator maybe_exists = dest_exifData.findKey( Exiv2::ExifKey(i->key()) );
				//if exists AND not to overwrite
				if (maybe_exists != dest_exifData.end())
                {
					continue;
				}
                else
                {
					// copy the value
					// we create a new tag in the destination file, the tag has the key of the source
					Exiv2::Exifdatum& dest_tag = dest_exifData[i->key()];
					//now the tag has also the value of the source
					dest_tag.setValue(&(i->value()));
				}
            }
        }
        else
        {
            destimage->setExifData(src_exifData);
        }
        
        destimage->writeMetadata();
    }
    

    bool JpegToHDRGEN(const char* filename, FILE* hdrgen, int frame_number)
    {
        // pad file names -- ugly but works
        stringstream ps;
        ps << frame_number;
        std::string padding = ps.str();
        padding.insert(padding.begin(), 6 - padding.size(), '0');
        char * padded_frame_number = new char[padding.size()];
        std::copy(padding.begin(), padding.end(), padded_frame_number);
          
        char file_path[128];
        sprintf(file_path, "./%s/jpeg/%s%s%s", filename, "image", padded_frame_number, ".jpeg");
        
        try
        {
           
            Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(file_path);
            image->readMetadata();
            
            Exiv2::ExifData exifData;
            exifData = image->exifData();
            
            if (exifData.empty()){
                throw VideoException("Exiv Error: No Exif Data in image");
            }
            
            Exiv2::ExifData::const_iterator _expo = exifData.findKey(Exiv2::ExifKey("Exif.Photo.ExposureTime"));
            Exiv2::ExifData::const_iterator _expo2 = exifData.findKey(Exiv2::ExifKey("Exif.Photo.ShutterSpeedValue"));
            Exiv2::ExifData::const_iterator _iso  = exifData.findKey(Exiv2::ExifKey("Exif.Photo.ISOSpeedRatings"));
            Exiv2::ExifData::const_iterator _fnum = exifData.findKey(Exiv2::ExifKey("Exif.Photo.FNumber"));
            Exiv2::ExifData::const_iterator _fnum2 = exifData.findKey(Exiv2::ExifKey("Exif.Photo.ApertureValue"));
            
            // default not valid values
            float expo  = -1;
            int iso   = -1;
            double fnum  = -1;
            
            if (_expo != exifData.end())
            {
                expo=_expo->toFloat();
            }
            else if (_expo2 != exifData.end())
            {
                long num=1, div=1;
                double tmp = std::exp(std::log(2.0) * _expo2->toFloat());
                if (tmp > 1)
                {
                    div = static_cast<long>(tmp + 0.5);
                }
                else
                {
                    num = static_cast<long>(1/tmp + 0.5);
                }
                expo = static_cast<float>(num)/static_cast<float>(div);
            }
            
            if (_fnum != exifData.end())
            {
                fnum = _fnum->toFloat();
            }
            else if (_fnum2 != exifData.end())
            {
                fnum = static_cast<float>(std::exp(std::log(2.0) * _fnum2->toFloat() / 2));
            }
            // some cameras/lens DO print the fnum but with value 0, and this is not allowed for ev computation purposes.
            if (fnum == 0)
                return false;
            
            //if iso is found use that value, otherwise assume a value of iso=100. (again, some cameras do not print iso in exif).
            if (_iso == exifData.end())
            {
                iso = 100;
            }
            else
            {
                iso = _iso->toFloat();
            }
            
            //At this point the three variables have to be != -1
            if (expo!=-1 && iso!=-1 && fnum!=-1)
            {
                
                // write new line to file in format:
                // path_to_an_image inverse_of_exposure_time_in_seconds aperture_size iso_speed 0
                fprintf(hdrgen,"%s %f %2.2f %d 0\n", 
                        file_path, 
                        1/expo, 
                        fnum, 
                        iso
                        );
                
            }
            else
            {
                return false;
            }
        }
        catch (Exiv2::AnyError& e)
        {
            throw VideoException("Exiv Error");
        }
        
        return true;
    }
    
    void SaveImageHistogram(const char* filename)
    {
        char histogram[128];
        
        try {
            Magick::Image img;
            img.read(filename);
            sprintf(histogram, "histogram:%s-histogram.jpg", filename);
            img.write(histogram);
        } 
        catch( Magick::ErrorFileOpen &error ) {
            // Process Magick++ file open error
            cerr << "Error: " << error.what() << endl;
        }

    }
}