#!/bin/sh

# Starts the Docker container and navigate to the project working directory.

docker run --rm -it \
  -v "$(pwd):/home/vagrant/compilers" \
  -w "/home/vagrant/compilers" \
  cffs/compilers /bin/bash