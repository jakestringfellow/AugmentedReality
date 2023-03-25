# OSX compiler
#CC = clang++

# Dwarf compiler
CC = /Applications/Xcode.app/Contents/Developer/usr/bin/g++

CXX = $(CC)

# OSX include paths (for MacPorts)
#CFLAGS = -I/opt/local/include -I../include

# OSX include paths (for homebrew, probably)
CFLAGS = -Wc++11-extensions -std=c++11 -I/usr/local/Cellar/opencv/4.7.0_1/include/opencv4 -I../include -DENABLE_PRECOMPILED_HEADERS=OFF

# Dwarf include paths
#CFLAGS = -I../include # opencv includes are in /usr/include
CXXFLAGS = $(CFLAGS)

# OSX Library paths (if you use MacPorts)
#LDFLAGS = -L/opt/local/lib

#OSX Library paths (if you use homebrew, probably)
LDFLAGS = -L/usr/local/Cellar/opencv/4.7.0_1/include/opencv4

# Dwarf Library paths
#LDFLAGS = -L/usr/local/Cellar/opencv/4.7.0_1/lib/opencv4/3rdparty  -L/usr/local/Cellar/opencv/4.7.0_1/lib # opencv libraries are here

# opencv libraries
LDLIBS = -ltiff -lpng -ljpeg -llapack -lblas -lz -lwebp -framework AVFoundation -framework CoreMedia -framework CoreVideo -framework CoreServices -framework CoreGraphics -framework AppKit -framework OpenCL  -lopencv_core -lopencv_highgui -lopencv_video -lopencv_videoio -lopencv_imgcodecs -lopencv_imgproc -lopencv_objdetect -lopencv_calib3d



BINDIR = .

augmentedReality: main.o 
	$(CC) $^ -o $(BINDIR)/$@ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f *.o *~ 
