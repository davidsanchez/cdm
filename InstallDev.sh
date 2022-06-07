mkdir -p build
cp payload/CMakeLists.txt build

cd /home/cdmmgr/boost/
 ./b2 install  --with=all

cd /home/cdmmgr/cdm/build
cmake -DCMAKE_INSTALL_PREFIX=.. -DMOS_PATH=/home/mosmgr/MOS/MOS_X86_9.3.0/
make