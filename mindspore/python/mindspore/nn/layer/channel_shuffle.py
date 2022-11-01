# Copyright 2022 Huawei Technologies Co., Ltd
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
"""channel shuffle"""
from mindspore.ops.primitive import constexpr
from mindspore.ops import operations as P
from mindspore.nn.cell import Cell

__all__ = ['ChannelShuffle']


class ChannelShuffle(Cell):
    r"""
    Divide the channels in a tensor of shape :math:`(*, C , H, W)`
    into g groups and rearrange them as :math:`(*, C \frac g, g, H, W)`,
    while keeping the original tensor shape.

    Args:
        groups (int): Number of groups to divide channels in. Refer to :math`g`.

    Inputs:
        - **x** (Tensor) - Tensor of shape :math:`(*, C_{in}, H_{in}, W_{in})`.

    Outputs:
        Tensor, with the same type and shape as the `x`.

    Raises:
        TypeError: If `groups` is not an int.
        ValueError: If `groups` is less than 1.
        ValueError: If dims of `x` is less than 3.
        ValueError: If number of channels can not be divisible by groups.

    Supported Platforms:
        ``Ascend`` ``GPU`` ``CPU``

    Examples:
        >>> channel_shuffle = nn.ChannelShuffle(2)
        >>> x = Tensor(np.arange(16).astype(np.int32).reshape(1, 4, 2, 2))
        >>> print(x)
        [[[[0 1],
           [2 3]],
          [[4 5],
           [6 7]],
          [[8 9],
           [10 11]],
          [[12 13],
           [14 15]],
         ]]
        >>> output = channel_shuffle(x)
        >>> print(output)
        [[[[0 1],
           [2 3]],
          [[8 9],
           [10 11]],
          [[4 5],
           [6 7]],
          [[12 13],
           [14 15]],
         ]]
    """
    def __init__(self, groups):
        """Initialize ChannelShuffle."""
        super(ChannelShuffle, self).__init__()
        if not isinstance(groups, int):
            raise TypeError("For ChannelShuffle, the param `groups` must be int, but got {}.".format(type(groups)))
        if groups < 1:
            raise ValueError(f"For ChannelShuffle, the param `groups` must be larger than 0, but got {groups}.")

        self.groups = groups
        self.shape = P.Shape()
        self.reshape = P.Reshape()
        self.transpose = P.Transpose()

    @staticmethod
    @constexpr
    def _check_input_dim(shape, channels, groups, cls_name):
        dim = len(shape)
        if dim < 3:
            raise ValueError(f"For {cls_name}, the in_shape must have more than 2 dims, but got {dim}.")

        if channels % groups != 0:
            raise ValueError(f"For {cls_name}, number of channels must be divisible by groups, "
                             f"but got {channels} channels and {groups} groups.")

    def construct(self, x):
        x_shape = self.shape(x)
        n, c = x_shape[0], x_shape[1]
        self._check_input_dim(x_shape, c, self.groups, self.cls_name)
        out = self.reshape(x, (n, self.groups, c // self.groups, -1))
        out = self.transpose(out, (0, 2, 1, 3))
        return self.reshape(out, x_shape)
