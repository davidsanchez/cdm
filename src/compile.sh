rm -r build
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=.. -DMOS_PATH=/MOS_v4.0.2/ ..
make 
#VERBOSE=1
