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


class IfAfterIfInForNet(nn.Cell):
    def __init__(self):
        super().__init__()
        self.param_a = Parameter(Tensor(5, mstype.int32), name='a')
        self.param_b = Parameter(Tensor(4, mstype.int32), name='b')

    def construct(self, x):
        out = x + self.param_b
        for _ in range(4):
            if out <= 20:
                out += self.param_a
        self.param_b += 3
        if x < self.param_b:
            out -= self.param_b
        return out


class IfAfterIfInForNet1(nn.Cell):
    def __init__(self):
        super().__init__()
        self.param_a = Parameter(Tensor(5, mstype.int32), name='a')
        self.param_b = Parameter(Tensor(4, mstype.int32), name='b')

    def construct(self, x):
        out = self.func(x)
        if x < self.param_b:
            out -= self.param_b
        return out

    def func(self, x):
        out = x + self.param_b
        for _ in range(4):
            if out <= 20:
                out += self.param_a
        self.param_b += 3
        return out


class IfAfterIfInForNet2(nn.Cell):
    def __init__(self):
        super().__init__()
        self.param_a = Parameter(Tensor(5, mstype.int32), name='a')
        self.param_b = Parameter(Tensor(4, mstype.int32), name='b')

    def construct(self, x):
        out = self.func(x)
        if x < self.param_b:
            out -= self.param_b
        return out

    def func(self, x):
        out = x + self.param_b
        for _ in range(4):
            out = self.subfunc(out)
        self.param_b += 3
        return out

    def subfunc(self, x):
        if x <= 20:
            x += self.param_a
        return x


class IfAfterIfInForNet3(nn.Cell):
    def __init__(self):
        super().__init__()
        self.param_a = Parameter(Tensor(5, mstype.int32), name='a')
        self.param_b = Parameter(Tensor(4, mstype.int32), name='b')

    def construct(self, x):
        out = self.func(x)
        if x < self.param_b:
            out -= self.param_b
        return out

    def func(self, x):
        out = x + self.param_b
        for _ in range(3):
            out += self.subfunc(x)
        self.param_b += 3
        return out

    def subfunc(self, x):
        if x > 10:
            return self.param_a
        return self.param_b


class GradNet(nn.Cell):
    def __init__(self, net):
        super(GradNet, self).__init__()
        self.net = net

    def construct(self, *inputs):
        return grad_all(self.net)(*inputs)


def control_flow_if_after_if_in_for(input_net, x):
    # graph mode
    context.set_context(mode=context.GRAPH_MODE)
    net = input_net()
    grad_net = GradNet(net)
    graph_forward_res = net(x)
    graph_backward_res = grad_net(x)

    # pynative mode
    context.set_context(mode=context.PYNATIVE_MODE)
    net = input_net()
    grad_net = GradNet(net)
    pynative_forward_res = net(x)
    pynative_backward_res = grad_net(x)

    assert graph_forward_res == pynative_forward_res
    assert graph_backward_res == pynative_backward_res


def test_if_after_if_in_for():
    x = Tensor(2, mstype.int32)
    control_flow_if_after_if_in_for(IfAfterIfInForNet, x)


def test_if_after_if_in_for_01():
    x = Tensor(2, mstype.int32)
    control_flow_if_after_if_in_for(IfAfterIfInForNet1, x)


def test_if_after_if_in_for_02():
    x = Tensor(2, mstype.int32)
    control_flow_if_after_if_in_for(IfAfterIfInForNet2, x)


def test_if_after_if_in_for_03():
    x = Tensor(2, mstype.int32)
    control_flow_if_after_if_in_for(IfAfterIfInForNet3, x)
