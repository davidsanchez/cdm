rm -r build
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=.. -DMOS_PATH=/MOS_v7.0.0/ ..
make 
#VERBOSE=1
