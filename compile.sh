#!/bin/bash -f
POSITIONAL=()
output="a.out"
while [[ $# -gt 0 ]]
do
  key="$1"
  case $key in
    -o)
    output="$2"
    shift # past argument
    shift # past value
    ;;
    *)    # files
    POSITIONAL+=("$1") # save it in an array for later
    shift # past argument
    ;;
  esac
done
echo  g++ -o $output ${POSITIONAL[@]} `pkg-config --cflags --libs opencv --libs gl`
g++ -o $output ${POSITIONAL[@]} `pkg-config --cflags --libs opencv --libs gl`

echo ""
