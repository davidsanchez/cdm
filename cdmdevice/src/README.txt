---------------------------------
Using MAKE
make
---------------------------------


---------------------------------
Using CMAKE
Using a separate build directory:

cd build
cp ../src/CMakeLists.txt .
cmake -DCMAKE_INSTALL_PREFIX=.. -DMOS_PATH=<your MOS PATH installation> ..
make
---------------------------------


cmake -DCMAKE_INSTALL_PREFIX=.. -DMOS_PATH=/home/mosmgr/MOS/MOS_X86_9.3.0/