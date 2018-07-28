#!/bin/bash

# pull to make sure we are not rebuilding for nothing
docker pull gcr.io/flutter-cirrus/build-engine-image:0.2

docker build --tag gcr.io/flutter-cirrus/build-engine-image:0.2 .

