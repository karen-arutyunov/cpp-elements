#!/bin/sh

lemmatizer_dir=/home/karen_arutyunov/projects/research/Morph/Lemmatizer

export RML=$lemmatizer_dir
export RML_PCRE_INCLUDE=/usr/include
export RML_PCRE_LIB=/usr/lib64

$lemmatizer_dir/build.sh

source_dir=$lemmatizer_dir/Source
lib_dir=$lemmatizer_dir/Lib

mkdir $lib_dir

ln -s $source_dir/AgramtabLib/libAgramtabrst.a $lib_dir/libAgramtabrst.a
ln -s $source_dir/GraphanLib/libGraphanrst.a $lib_dir/libGraphanrst.a
ln -s $source_dir/LemmatizerLib/libLemmatizerrst.a $lib_dir/libLemmatizerrst.a
ln -s $source_dir/MorphWizardLib/libMorphWizardrst.a $lib_dir/libMorphWizardrst.a
ln -s $source_dir/StructDictLib/libStructDictrst.a $lib_dir/libStructDictrst.a
