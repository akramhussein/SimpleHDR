#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#include <errno.h>

// Part of the exif command-line source package
//#include "libjpeg/jpeg-data.h"

#include <pangolin/pangolin.h>
#include <pangolin/video.h>
#include <pangolin/video/firewire.h>

using namespace pangolin;
using namespace std;

static void createEXIF(dc1394featureset_t *xFeatures, ExifData ** pParentEd);

int main(int argc, char *argv[])
{   

    ExifData * pEd;
    
    pangolin::FirewireVideo video = FirewireVideo(); // Simplified constructor

    createEXIF(video.GetFeatures(), &pEd);  //tag the file with the settings of the camera
    
    exif_data_dump(pEd);
    
    
    //write the Exif data to a jpeg file
    pData = jpeg_data_new_from_file (FILENAME);  //input data
    if (!pData) {
        printf ("Could not load '%s'!\n", FILENAME);
        return (-1);
    }
    
    printf("Saving EXIF data to jpeg file\n");
    jpeg_data_set_exif_data (pData, pEd);
    printf("Set the data\n");
    jpeg_data_save_file(pData, "foobar2.jpg");
    
    return 0;
    
}


void createEXIF(dc1394featureset_t *xFeatures, ExifData ** pParentEd)
{
    ExifEntry *pE;
    ExifData *pEd;
    
    pEd = exif_data_new ();
    
    //printf ("Adding a Make reference\n");
    pE = exif_entry_new ();
    exif_content_add_entry (pEd->ifd[EXIF_IFD_0], pE);
    exif_entry_initialize (pE, EXIF_TAG_MAKE);
    pE->data= (unsigned char *) "P.GREY";
    exif_entry_unref (pE);
    
    
    *pParentEd = pEd;

}
