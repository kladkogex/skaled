#!/bin/bash

VERSION=$1
DOCKER_USERNAME=$2
DOCKER_PASSWORD=$3
IMAGE_NAME=skalenetwork/schain:$VERSION

docker login -u $DOCKER_USERNAME -p $DOCKER_PASSWORD
docker push $IMAGE_NAME
