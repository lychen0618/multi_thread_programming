#!/usr/bin/env bash

echo "================= build started ================="

cur=`pwd`
source_root=$(cd `dirname $0`; pwd)
cd $source_root

bazel --output_user_root=../ build --cxxopt="-std=c++11" --cxxopt="-Wall" --cxxopt="-Wextra" --cxxopt="-Wno-unused" ... -c dbg
if [ $? -ne 0 ];then
    echo "==============!!! build failed !!!==============";
    exit 1;
fi

echo "================= build success ================="

cd $cur
echo "================= output created ================="
