from distutils.core import setup, Extension
import glob 

def main () :
    setup(name="Frame", 
          version="1.0.0",
          description="Core Trace functionality for frame extraction",
          author="Traits",
          author_email="grostaco@gmail.com",
          ext_modules=[Extension("Frame", ["Ext/FrameWrapper.c"] + glob.glob("src/video/*.c"), include_dirs=['./include'],
                                 libraries=["avcodec", "avformat", "avutil", "swscale"])], 
        )

if __name__ == "__main__" :
    main()