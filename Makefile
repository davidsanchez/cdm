all:
	@echo 'Creating all Docker images'
	make image
	make dev-image

image-prod:
	@echo 'Creating main Docker image for CDM...'
	#wget -N https://boostorg.jfrog.io/artifactory/main/release/1.79.0/source/boost_1_79_0.tar.gz
	#cp boost_1_79_0.tar.gz boost.tar.gz 
	#gunzip boost.tar.gz
	#tar -xvf boost.tar 
	#rm -r payload/boost_1_79_0
	#mv boost_1_79_0 payload/
	#rm boost.tar
	docker build  -f containers/Dockerfile . --tag cdm:latest

dev-image:
	@echo 'Creating development Docker image for CDM...'
	docker build  -f containers/Dockerfile.dev . --tag cdm:dev2


image-alma:
	@echo 'Creating development Docker image for CDM...'
	docker build  -f containers/Dockerfile.almalinux . --tag cdm:alma9


local-image:
	@echo 'Creating local Docker image for CDM...'
	docker build  -f containers/Dockerfile.local . --tag cdm:local


clean:
	@echo 'Cleaning all Docker images'
	docker rmi cdm:latest
	docker builder prune

clean-local:
	@echo 'Cleaning all Docker images'
	docker rmi cdm:local
	docker builder prune

clean-dev:
	@echo 'Cleaning all Docker images'
	docker rmi cdm:dev2
	docker builder prune

clean-alma:
	@echo 'Cleaning all Docker images'
	docker rmi cdm:alma9
	docker builder prune