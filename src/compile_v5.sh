rm -r build
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=.. -DMOS_PATH=/MOS_v5.0.1/ ..
make 
#VERBOSE=1
