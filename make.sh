#!/bin/bash

function BuildAll
{
    echo "build util"
    cd build
    cmake ..
    make
    cd ..
}


function Clean
{
    cd build
    rm  -rf *
    cd ..
}

BuildAll
