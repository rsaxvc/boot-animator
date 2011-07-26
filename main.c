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
#include <sys/param.h>
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

int help = 0;
int usage = 0;

static const char * input_string = NULL;
static const char * output_string = NULL;

// -1 means default it
int num_frames = -1;
int loop       = -1;
int framerate  = -1;
int frameskip  = -1;
int frameseek  = -1;
int width      = -1;
int height     = -1;

struct poptOption popts[]=
{
POPT_AUTOHELP
{"input"    , 'i', POPT_ARG_STRING, &input_string,  ARG_INPUT     , NULL, "input filename (avi/mpg/...)"},
{"output"   , 'o', POPT_ARG_STRING, &output_string, ARG_OUTPUT    , NULL, "output filename (defaults to bootanimation.zip)"},
{"numframes", 'n', POPT_ARG_INT   , &num_frames,    ARG_NUM_FRAMES, NULL, "number of frames to convert - defaults to all of them"},
{"loop"     , 'l', POPT_ARG_INT   , &loop,          ARG_LOOP      , NULL, "do loop(nonzero=loop)"},
{"framerate", 'f', POPT_ARG_INT   , &framerate,     ARG_FRAMERATE , NULL, "frames per second, defaults from input video"},
{"frameskip", 's', POPT_ARG_INT   , &frameskip,     ARG_FRAMESKIP , NULL, "frameskip - only keep every nth frame where n is frameskip"},
{"frameseek", 'k', POPT_ARG_INT   , &frameseek,     ARG_FRAMESEEK , NULL, "seek to this frame first" },
{"width"    , 'w', POPT_ARG_INT   , &width,         ARG_WIDTH     , NULL, "width of image"},
{"height"   , 'h', POPT_ARG_INT   , &height,        ARG_HEIGHT    , NULL, "heightof image"},
POPT_TABLEEND
};

static void write_desc(int width, int height, float fps, int loop, char * fname, char * name );

int main( int num_args, const char * const args[] )
{
    IplImage  * frame;
    char        fname_buf[200];

    poptContext optCon;

    optCon = poptGetContext("main", num_args, args, popts, 0 );

    while (poptGetNextOpt(optCon) > 0);//churn through the args
    poptResetContext(optCon);

    if (help)
        {
        poptPrintHelp(optCon, stdout, 0);
        goto free_and_quit;
        }

    if (usage)
        {
        poptPrintUsage(optCon, stdout, 0);
        goto free_and_quit;
        }

    printf("width:%i\n",width);
    printf("height:%i\n",height);

    if( num_frames < -1 || num_frames == 0 )
        {
        printf("Invalid number of frames");
        goto free_and_quit;
        }

    if( input_string == NULL )
        {
        poptPrintUsage(optCon, stdout, 0);
        goto free_and_quit;
        }

    if( output_string == NULL )
        {
        output_string = "bootanimation.zip";
        }

    /* load the AVI file */
    CvCapture *capture = cvCaptureFromAVI( input_string );

    /* always check */
    if( !capture )
        {
        printf("Couldn't open %s\n", input_string );
        return 3;
        }

    if(width     == -1 )width     = cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH );
    if(height    == -1 )height    = cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT );
    if(framerate == -1 )framerate = cvGetCaptureProperty( capture, CV_CAP_PROP_FPS );
    if(frameskip == -1 )frameskip = 0;
    if(frameseek == -1 )frameseek = 0;
    if(loop      == -1 )loop      = 1;

    frameskip++;//makes logic easier

    int frame_idx;
    frame_idx = 0;

    mkdir( "part0", S_IXOTH | S_IRWXG | S_IRWXU );
    write_desc( width, height, framerate, loop, "desc.txt", "part0" );

    //Seek into the stream
	while( frameseek-- )
        {
	    frame = cvQueryFrame( capture );
	    if( !frame )
            {
	        printf("Ran outta frames\n");
	        goto free_and_quit;
            }
	    else
            {
	        printf("Skipping frame\n");
            }
        }

	int temp_frameskip;
	temp_frameskip = frameskip;
    while( 1 )
        {
        /* get a frame */
        frame = cvQueryFrame( capture );
        if( !frame )
            {
            //ran off end of video
            break;
            }

        temp_frameskip--;

        if( temp_frameskip == 0 )
            {
            snprintf(fname_buf, sizeof( fname_buf ), "part0/boot_%05i.png", frame_idx++ );
            if(!cvSaveImage( fname_buf, frame, 0 ) )
                {
                printf("Could not save: %s\n",fname_buf);
                exit(2);
                }
            temp_frameskip = frameskip;
            }

        if( num_frames > 0 )
            {
            num_frames--;
            if( num_frames == 0 )
                {
                break;
                }
            }
        }

    /* free memory */
    cvReleaseCapture( &capture );

    char sysbuf[1024];
    strcpy( sysbuf, "zip -rv -Z store " );
    strcat( sysbuf, output_string );
    strcat( sysbuf, " desc.txt part0" );
    system( sysbuf );

free_and_quit:
    optCon = poptFreeContext(optCon);
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
fprintf(fptr, "%i %i %i\r\n", width, height, (int)fps);
fprintf(fptr, "p %i 0 %s\r\n", loop!=0, name );
fclose( fptr );
}

