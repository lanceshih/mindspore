#!/bin/bash
# Copyright 2020 Huawei Technologies Co., Ltd
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ============================================================================
echo "=============================================================================================================="
echo "Please run the script as: "
echo "sh run_standalone_train_gpu.sh DATASET_PATH"
echo "for example: sh run_standalone_train_gpu.sh /home/workspace/ag"
echo "It is better to use absolute path."
echo "=============================================================================================================="
get_real_path(){
  if [ "${1:0:1}" == "/" ]; then
    echo "$1"
  else
    echo "$(realpath -m $PWD/$1)"
  fi
}

DATASET=$(get_real_path $1)
echo $DATASET
DATANAME=$(basename $DATASET)
echo $DATANAME

if [ -d "train" ];
then
    rm -rf ./train
fi
mkdir ./train
cp ../*.py ./train
cp -r ../src ./train
cp -r ../scripts/*.sh ./train
cd ./train || exit
echo "start training for standalone GPU device"

python train.py --device_target="GPU" --data_path=$1 --data_name=$DATANAME > log_fasttext.log 2>&1 &
cd ..