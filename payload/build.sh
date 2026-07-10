cd cdm/build/
git config --global --add safe.directory /home/cdmmgr/cdm

git pull
make
make install
cd ../../../