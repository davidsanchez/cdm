rm -r build
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=.. -DMOS_PATH=/MOS_test.1.3/ ..
make 
#VERBOSE=1
