import Frame
import glob
import argparse
import sys

def main () :
    parser = argparse.ArgumentParser("Extract frames from a media file\n")
    parser.add_argument('-O', '--out',      dest='dstfmt',                         help="Set destination folder and format of frames extracted")
    parser.add_argument('-N', '--nframes',  dest='nframes_requested', type=int,    help="Save up to nframes")
    parser.add_argument('-B', '--bulksave', dest='bulksave', action='store_true',  help="Enable bulk saving")
    parser.add_argument('-S', '--sepsave',  dest='sepsave',  action='store_true',  help="Enable seperated save")
    parser.add_argument('-I', '--infile',   dest='infile',                         help="Media file to be extracted from")

    args = parser.parse_args()
    
    if (args.infile == None) :
        print(f'{sys.argv[0]}: fatal error: no input file\nTerminating.')
        exit(1)

    if (args.dstfmt == None) :
        print(f'{sys.argv[0]}: fatal error: output format not supplied\nTerminating.')
        exit(1)

    f = Frame.Frame()
    f.open(args.infile)
    f.extract(dstfmt=args.dstfmt, nframes=args.nframes_requested, bulksave=args.bulksave, sepsave=args.sepsave)
    f.close()

if __name__ == "__main__" :
    main()