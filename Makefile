all:
	@echo 'Creating all Docker images'
	make image
	make dev-image

image:
	@echo 'Creating main Docker image for CDM...'
	docker build --no-cache  -f containers/Dockerfile . --tag cdm:latest

image-dev:
	@echo 'Creating development Docker image for CDM...'
	docker build  -f containers/Dockerfile.almalinux . --tag cdm:dev

image-local:
	@echo 'Creating local Docker image for CDM...'
	docker build --platform linux/x86_64  -f containers/Dockerfile.local  --tag cdm:local --progress=plain  . 


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