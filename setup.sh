#!/usr/bin/bash

PLATFORM=$(uname -s)

case "${PLATFORM}" in
    Linux*)     BUILD_DIR="build-linux";CMAKE_GENERATOR="\"Unix Makefiles\"";;
    MINGW*)     BUILD_DIR="build-mingw";CMAKE_GENERATOR="\"MinGW Makefiles\"";;
    *)          echo "${unameOut} is not supported!";exit 1;;
esac

echo "Removing '$BUILD_DIR' ..."
cmd="rm -rf $BUILD_DIR"
eval $cmd

echo "Creating '$BUILD_DIR' ..."
cmd="mkdir -p '$BUILD_DIR'"
eval $cmd

ROOT_DIR=$(pwd)
echo "Changing directory to '$BUILD_DIR' ..."
cmd="cd '$BUILD_DIR'"
eval $cmd

echo "Generating makefiles ..."
cmd="cmake -G $CMAKE_GENERATOR .."
eval $cmd

echo "Returning to '$ROOT_DIR' ..."
cmd="cd '$ROOT_DIR'"
eval $cmd
