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

#include "image.h"

using namespace std;

namespace pangolin
{
    
    void CopyPPMToJPG(const char* filename_ppm, const char* filename_jpg){
        
        try {
            Magick::Image img;
            img.read(filename_ppm);
            img.write(filename_jpg);
        } 
        catch( Magick::ErrorFileOpen &error ) {
            // Process Magick++ file open error
            cerr << "Error: " << error.what() << endl;
        }
    }
       
    void WriteExifData(const pangolin::FirewireVideo* video, const std::string& filename)
    {
      
        Exiv2::ExifData exifData;
        
        try {
            
            // TO DO - change to metadata instead
            exifData["Exif.Image.Make"] = video->GetCameraVendor();
            exifData["Exif.Image.Model"] = video->GetCameraModel();
            exifData["Exif.Photo.FNumber"] = Exiv2::Rational(7, 5); // hard coded
            exifData["Exif.Photo.ExposureTime"] = Exiv2::floatToRationalCast(video->GetFeatureValue(DC1394_FEATURE_SHUTTER));	
            exifData["Exif.Photo.ExposureBiasValue"] = Exiv2::floatToRationalCast(video->GetFeatureValue(DC1394_FEATURE_EXPOSURE));	
            exifData["Exif.Photo.ColorSpace"] = uint16_t(1);
            exifData["Exif.Photo.WhiteBalance"] = uint16_t(video->GetFeatureMode(DC1394_FEATURE_WHITE_BALANCE));   // 0=auto,1=man
            exifData["Exif.Photo.GainControl"] = uint16_t(video->GetFeatureValue(DC1394_FEATURE_GAIN));
            exifData["Exif.Photo.Saturation"] = uint16_t(video->GetFeatureValue(DC1394_FEATURE_SATURATION));
            exifData["Exif.Photo.Sharpness"] = uint16_t(video->GetFeatureValue(DC1394_FEATURE_SHARPNESS));

            
            Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(filename);
            assert(image.get() != 0);
            
            image->setExifData(exifData);
            image->writeMetadata();
            
        }
        catch (Exiv2::AnyError& e) {
            cout << "Exiv error: '" << e << "'\n";
        }
        

    }

    /* Borrowed from LuminanceHDR package
     * 
     * Credits to Giuseppe Rota <grota@users.sourceforge.net>
     *
     * This function obtains the "average scene luminance" (cd/m^2) from an image file.
     * "average scene luminance" is the L (aka B) value mentioned in [1]
     * You have to take a log2f of the returned value to get an EV value.
     * 
     * We are using K=12.07488f and the exif-implied value of N=1/3.125 (see [1]).
     * K=12.07488f is the 1.0592f * 11.4f value in pfscalibration's pfshdrcalibrate.cpp file.
     * Based on [3] we can say that the value can also be 12.5 or even 14.
     * Another reference for APEX is [4] where N is 0.3, closer to the APEX specification of 2^(-7/4)=0.2973.
     * 
     * [1] http://en.wikipedia.org/wiki/APEX_system
     * [2] http://en.wikipedia.org/wiki/Exposure_value
     * [3] http://en.wikipedia.org/wiki/Light_meter
     * [4] http://doug.kerr.home.att.net/pumpkin/#APEX
     * 
     * This function tries first to obtain the shutter speed from either of
     * two exif tags (there is no standard between camera manifacturers):
     * ExposureTime or ShutterSpeedValue.
     * Same thing for f-number: it can be found in FNumber or in ApertureValue.
     * 
     * F-number and shutter speed are mandatory in exif data for EV calculation, iso is not.
     */
    float GetAvgLuminance(const std::string& filename)
    {
        try
        {

            Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(filename);
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
    
    void CopyExifData(const std::string& from, const std::string& to,  bool dont_overwrite)
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
    

    bool JpgToHDRGEN(const char* filename, FILE* hdrgen, int frame_number)
    {
        
        char file_path[128];
        sprintf(file_path, "./%s/jpg/image0000%d.jpg", filename, frame_number);
        
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
    
}