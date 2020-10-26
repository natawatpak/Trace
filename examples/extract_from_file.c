#include "video/extract.h"
#include "image/imageutil.h"

#include <sys/stat.h>
#include <getopt.h>

int main (int argc, char** argv) { 

    int option_index;
    int c;

    char* infile  = NULL;
    char* outfile = NULL;

    struct option long_options[] = {
        {"help",    no_argument,       0, 'h'},
        {"in",      required_argument, 0, 'i'},
        {"out",     required_argument, 0, 'o'},
    };
    
    while ((c=getopt_long(argc, argv, "hi:o:", long_options, &option_index)) != -1) {
        switch(c) {
            case 'h' :
                printf("""Usage: %s [-h]\
                          \n        [-n frames]\
                          \n        [-o|--out outfmt]\
                          \n        [-i|--in] infile\
                          \nExtract frames from a media file\
                        """, argv[0]);
                exit(0);
            case 'i' :
                infile = optarg;
                break;
            case 'o' :
                outfile = optarg;
                break;
            case '?' :
                printf("?");
                infile = optarg;
                break;
            default :
                abort();
        }
    }
    if (infile == NULL) {
        if (argv[optind] == NULL) {
            fprintf(stderr, "%s: infile not supplied\n", argv[0]);
            exit(1);
        }
        infile = argv[optind];
    }

    if (outfile == NULL) {
        outfile = malloc(128);
        struct stat st;
        if (stat("Frames", &st) == -1) {
            mkdir("Frames", S_IRWXU);
        }
        char* btmp = infile + strlen(infile) - 1;
        while (*btmp != '/')btmp--;
        snprintf(outfile, 128, "Frames%s@f%%d-%%d.png", btmp);
    }

    AVCodecContext* cCtx = avcodec_alloc_context3(NULL);

    extract_frames(infile, AV_PIX_FMT_YUV420P, outfile, cCtx);
    free(outfile);
    exit(0);
}

