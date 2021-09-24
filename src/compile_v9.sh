rm -r build
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=.. -DMOS_PATH=/MOS_v9.3.0/ ..
make 
#VERBOSE=1
