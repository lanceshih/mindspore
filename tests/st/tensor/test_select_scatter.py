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
import numpy as np
import pytest
from tests.mark_utils import arg_mark
import mindspore as ms
import mindspore.nn as nn
from mindspore import Tensor


class Net(nn.Cell):
    def __init__(self, axis, index):
        super(Net, self).__init__()
        self.axis = axis
        self.index = index

    def construct(self, input_x, src):
        return input_x.select_scatter(src, self.axis, self.index)


@arg_mark(plat_marks=['cpu_linux', 'cpu_windows', 'cpu_macos', 'platform_gpu', 'platform_ascend'],
          level_mark='level2',
          card_mark='onecard',
          essential_mark='unessential')
@pytest.mark.parametrize('mode', [ms.GRAPH_MODE, ms.PYNATIVE_MODE])
def test_tensor_select_scatter(mode):
    """
    Feature: tensor.select_scatter
    Description: Verify the result of select_scatter
    Expectation: success
    """
    ms.set_context(mode=mode)
    x = Tensor([[[0, 1, 2],
                 [3, 4, 5],
                 [6, 7, 8]],
                [[9, 10, 11],
                 [12, 13, 14],
                 [15, 16, 17]]], ms.float32)
    y = Tensor([[18, 19, 20],
                [21, 22, 23]], ms.float32)
    net = Net(1, 0)
    output = net(x, y)
    expect_output = [[[18., 19., 20.],
                      [3., 4., 5.],
                      [6., 7., 8.]],
                     [[21., 22., 23.],
                      [12., 13., 14.],
                      [15., 16., 17.]]]
    assert np.allclose(output.asnumpy(), expect_output)
