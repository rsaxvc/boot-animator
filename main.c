/**
 * How to Convert AVI Files to BootAnimation.zip with OpenCV
 *
 * Author  Richard Allen
 * License GPL
 * Website http://rsaxvc.net
 */

#include <stdio.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <sys/stat.h>
#include <popt.h>

enum
{
	ARG_DUMMY,
	ARG_INPUT,
	ARG_OUTPUT,
	ARG_NUM_FRAMES,
	ARG_LOOP,
	ARG_FRAMERATE,
	ARG_FRAMESKIP,
	ARG_FRAMESEEK,
	ARG_WIDTH,
	ARG_HEIGHT,
	ARG_CNT
};

struct poptOption popts[]=
{
POPT_AUTOHELP
{"input"    , 'i', POPT_ARG_STRING, NULL, ARG_INPUT     , NULL, "input filename (avi/mpg/...)"},
{"output"   , 'o', POPT_ARG_STRING, NULL, ARG_OUTPUT    , NULL, "output filename (defaults to bootanimation.zip)"},
{"numframes", 'n', POPT_ARG_INT   , NULL, ARG_NUM_FRAMES, NULL, ""},
{"loop"     , 'l', POPT_ARG_INT   , NULL, ARG_LOOP      , NULL, ""},
{"framerate", 'f', POPT_ARG_INT   , NULL, ARG_FRAMERATE , NULL, "frames per second, defaults from input video"},
{"frameskip", 's', POPT_ARG_INT   , NULL, ARG_FRAMESKIP , NULL, "frameskip - only keep every nth frame where n is frameskip"},
{"frameseek", 'k', POPT_ARG_INT   , NULL, ARG_FRAMESEEK , NULL, "seek to this frame first" },
{"width"    , 'w', POPT_ARG_INT   , NULL, ARG_WIDTH     , NULL, "width of image"},
{"height"   , 'h', POPT_ARG_INT   , NULL, ARG_HEIGHT    , NULL, "heightof image"},
POPT_TABLEEND
};

static void write_desc(int width, int height, float fps, int loop, char * fname, char * name );

int main( int num_args, const char * const args[] )
{
    IplImage  * frame;
    int       	key;
    char 		fname_buf[200];

    int width;
    int height;
    float framerate;
    int frameseek;
    int frameskip;
    int loop;

    if( num_args != 2 )
    {
    printf("usage: %s video.avi <flags>\r\n", args[0]);
    exit(1);
    }

    /* load the AVI file */
    CvCapture *capture = cvCaptureFromAVI( args[1] );

    /* always check */
    if( !capture )
    	{
    	printf("Couldn't open %s", args[1]);
    	return 3;
    	}

    width     	= cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH );
    height 		= cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT );
    framerate 	= cvGetCaptureProperty( capture, CV_CAP_PROP_FPS );
    frameskip   = 1;
    frameseek   = 0;
    loop		= 1;

    //Do arg parsing here

    int frame_idx;

    frame_idx = 0;

    mkdir( "part0", S_IXOTH | S_IRWXG | S_IRWXU );
    write_desc( width, height, framerate, loop, "desc.txt", "part0" );

    //Seek into the stream
	while( frameseek-- )cvQueryFrame( capture );

    while( key != 'q' ) {
        /* get a frame */
		snprintf(fname_buf, sizeof( fname_buf ), "part0/boot_%05i.png", frame_idx++ );
    	frame = cvQueryFrame( capture );
    	if( !frame )
			{
    		//ran off end of video
    		break;
			}
        if(!cvSaveImage( fname_buf, frame, 0 ) )
        	{
        	printf("Could not save: %s\n",fname_buf);
        	exit(2);
        	}
    }

    /* free memory */
    cvReleaseCapture( &capture );

    system("zip -rv -Z store bootanimation.zip desc.txt part0");

    return 0;
}

static void write_desc(int width, int height, float fps, int loop, char * fname, char * name )
{
FILE * fptr;

fptr = fopen(fname, "w+");

if( fptr == NULL )
	{
	printf("Couldn't open %s", fname );
	exit(4);
	}
fprintf(fptr, "%i %i %f\r\n", width, height, fps);
fprintf(fptr, "p %i 0 %s\r\n", loop!=0, name );
fclose( fptr );
}

