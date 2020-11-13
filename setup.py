from distutils.core import setup, Extension
import glob 

def main () :
    setup(name="Trace", 
          version="1.0.0",
          description="Core Trace functionality for frame extraction",
          author="Traits",
          author_email="grostaco@gmail.com",
          ext_modules=[Extension("Frame", ["Ext/FrameWrapper.c"] + glob.glob("src/video/*.c"), include_dirs=['./include'],
                                 libraries=["avcodec", "avformat", "avutil", "swscale"]),
                       Extension("Query", ["Ext/QueryWrapper.c", "src/query/query.c"], include_dirs=['./include'],
                                 libraries=[]) 
                       ], 
        )

if __name__ == "__main__" :
    main()