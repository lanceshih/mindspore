#!/bin/bash
# Copyright 2023 Huawei Technologies Co., Ltd
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

clear

python3 ./main.py --model_path=/your/path/to/xxx0.mindir --device_id=0 --rank_id=0 --config_file=./config_file.ini & 
python3 ./main.py --model_path=/your/path/to/xxx1.mindir --device_id=1 --rank_id=1 --config_file=./config_file.ini
