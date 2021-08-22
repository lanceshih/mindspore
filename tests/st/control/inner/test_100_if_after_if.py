# Copyright 2021 Huawei Technologies Co., Ltd
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
from mindspore import context
from mindspore import Tensor, nn
from mindspore.ops import composite as C
from mindspore.common import dtype as mstype
from mindspore.common.parameter import Parameter

grad_all = C.GradOperation(get_all=True)
context.set_context(device_target="Ascend")


class IfAfterIfNet(nn.Cell):
    def __init__(self):
        super().__init__()
        self.param_a = Parameter(Tensor(5, mstype.int32), name='a')
        self.param_b = Parameter(Tensor(4, mstype.int32), name='b')

    def construct(self, x, y):
        out = y
        if self.param_a > self.param_b:
            x += 5
        self.param_b += 4
        if x < self.param_b:
            out += self.param_b
        return out


class IfAfterIfNet1(nn.Cell):
    def __init__(self):
        super().__init__()
        self.param_a = Parameter(Tensor(5, mstype.int32), name='a')
        self.param_b = Parameter(Tensor(4, mstype.int32), name='b')

    def construct(self, x, y):
        out = y
        x = self.func(x)
        if x < self.param_b:
            out += self.param_b
        return out

    def func(self, x):
        if self.param_a > self.param_b:
            x += 5
        self.param_b += 4
        return x


class IfAfterIfNet2(nn.Cell):
    def construct(self, x, y):
        x += 1
        out = self.func(x, y)
        if out > 10:
            out += 5
        return out

    def func(self, x, y):
        if x < y:
            y += x
        else:
            y -= x
        return y


class IfAfterIfNet3(nn.Cell):
    def __init__(self):
        super().__init__()
        self.param_a = Parameter(Tensor(5, mstype.int32), name='a')
        self.param_b = Parameter(Tensor(4, mstype.int32), name='b')

    def construct(self, x, y):
        out = x * y + self.func(self.param_b)
        if self.param_a > self.param_b:
            out += 5
        return out

    def func(self, x):
        if self.param_a > self.param_b:
            x += 5
        self.param_b += 4
        return x


class GradNet(nn.Cell):
    def __init__(self, net):
        super(GradNet, self).__init__()
        self.net = net

    def construct(self, *inputs):
        return grad_all(self.net)(*inputs)


def control_flow_if_after_if(input_net, x, y):
    # graph mode
    context.set_context(mode=context.GRAPH_MODE)
    net = input_net()
    grad_net = GradNet(net)
    graph_forward_res = net(x, y)
    graph_backward_res = grad_net(x, y)

    # pynative mode
    context.set_context(mode=context.PYNATIVE_MODE)
    net = input_net()
    grad_net = GradNet(net)
    pynative_forward_res = net(x, y)
    pynative_backward_res = grad_net(x, y)

    assert graph_forward_res == pynative_forward_res
    assert graph_backward_res == pynative_backward_res


def test_if_after_if():
    x = Tensor(2, mstype.int32)
    y = Tensor(5, mstype.int32)
    control_flow_if_after_if(IfAfterIfNet, x, y)


def test_if_after_if_01():
    x = Tensor(2, mstype.int32)
    y = Tensor(5, mstype.int32)
    control_flow_if_after_if(IfAfterIfNet1, x, y)


def test_if_after_if_02():
    x = Tensor(2, mstype.int32)
    y = Tensor(5, mstype.int32)
    control_flow_if_after_if(IfAfterIfNet2, x, y)


def test_if_after_if_03():
    x = Tensor(2, mstype.int32)
    y = Tensor(5, mstype.int32)
    control_flow_if_after_if(IfAfterIfNet3, x, y)
