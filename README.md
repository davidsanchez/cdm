# CDM
Camera Displacement Monitor


# Docker build

to build the Docker container, use the file buildCMDContainer

./buildCMDContainer

then run it 

docker run -tid --name mycdm cdm:v1.0

docker exec -ti mycdm sh 

In the docker machine make :

cd boost
./b2 install  --with=all
cd ../cmd/build
make
