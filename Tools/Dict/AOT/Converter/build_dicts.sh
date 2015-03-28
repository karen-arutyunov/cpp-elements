#!/bin/sh

lemmatizer_dir=/home/karen_arutyunov/projects/research/Morph/Lemmatizer
lib_dir=$lemmatizer_dir/Lib

export RML=$lemmatizer_dir
export LD_LIBRARY_PATH=$lib_dir:$LD_LIBRARY_PATH

echo "Dumping eng"
ElAOTConverter raw lang=eng > ./eng
echo "Dumping rus"
ElAOTConverter raw lang=rus > ./rus
echo "Dumping ger"
ElAOTConverter raw lang=ger > ./ger
