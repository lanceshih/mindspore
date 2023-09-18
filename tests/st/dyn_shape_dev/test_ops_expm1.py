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

import pytest
import numpy as np
import mindspore as ms
from mindspore import ops, Tensor


@ms.jit
def expm1_forward_func(x):
    return ops.auto_generate.expm1(x)


@ms.jit
def expm1_backward_func(x):
    return ops.grad(expm1_forward_func, (0,))(x)


@pytest.mark.level0
@pytest.mark.env_onecard
@pytest.mark.platform_x86_cpu
@pytest.mark.platform_x86_gpu_training
def test_expm1_forward():
    """
    Feature: Ops.
    Description: test op expm1.
    Expectation: expect correct result.
    """
    x = Tensor(np.array([0.0, 1.0, 2.0, -1]), ms.float32)
    output = expm1_forward_func(x)
    expect = [0., 1.718282, 6.389056, -0.63212055]
    assert np.allclose(output.asnumpy(), expect, rtol=1e-4, atol=1e-4)


@pytest.mark.level0
@pytest.mark.env_onecard
@pytest.mark.platform_x86_cpu
@pytest.mark.platform_x86_gpu_training
def test_expm1_backward():
    """
    Feature: Auto grad.
    Description: test auto grad of op expm1.
    Expectation: expect correct result.
    """
    x = Tensor(np.array([1, -1]), ms.float32)
    output = expm1_backward_func(x)
    expect = [2.7182817, 0.36787948]
    assert np.allclose(output.asnumpy(), expect, rtol=1e-4, atol=1e-4)


@pytest.mark.level0
@pytest.mark.env_onecard
@pytest.mark.platform_x86_cpu
@pytest.mark.platform_x86_gpu_training
def test_expm1_vmap():
    """
    Feature: test vmap function.
    Description: test expm1 op vmap.
    Expectation: expect correct result.
    """
    x = Tensor(np.array([[[0.0, 1.0], [2.0, -1]]]))
    nest_vmap = ops.vmap(ops.vmap(expm1_forward_func, in_axes=0), in_axes=0)
    output = nest_vmap(x)
    expect = [[[0., 1.718282], [6.389056, -0.63212055]]]
    assert np.allclose(output.asnumpy(), expect, rtol=1e-4, atol=1e-4)
