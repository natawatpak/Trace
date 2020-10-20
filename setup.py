from distutils.core import setup, Extension

def main () :
    setup(name="Trace", 
          version="1.0.0",
          description="Core Trace functionality for frame extraction",
          author="Traits",
          author_email="grostaco@gmail.com",
          ext_modules=[Extension("Trace", ["Ext/TraceWrapper.c", "src/video/extract.c"], include_dirs=['./include'],
                                 libraries=["avcodec", "avformat", "avutil", "swscale"])], 
        )

if __name__ == "__main__" :
    main()