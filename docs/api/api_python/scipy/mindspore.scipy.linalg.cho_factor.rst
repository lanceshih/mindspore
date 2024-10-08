mindspore.scipy.linalg.cho_factor
=================================

.. py:function:: mindspore.scipy.linalg.cho_factor(a, lower=False, overwrite_a=False, check_finite=True)

    计算矩阵的cholesky分解，用于 :func:`mindspore.scipy.linalg.cho_solve`。

    返回包含cholesky分解的矩阵，对于一个Hermitian正定矩阵 `A`，根据 `lower` 取值，进行如下形式的分解：

    - `lower` 为True: :math:`A = L L^*`
    - `lower` 为False: :math:`A = U^* U`

    其中， :math:`L^*` 为 :math:`L` 的共轭转置矩阵。
    其中， :math:`U^*` 为 :math:`U` 的共轭转置矩阵。

    返回值可以直接作为 :func:`mindspore.scipy.linalg.cho_solve` 的第一个参数使用。

    .. note::
        - Windows平台上还不支持 `cho_factor`。
        - 仅支持float32、float64、int32、int64类型的Tensor类型。
        - 如果Tensor是int32、int64类型，它将被强制转换为：mstype.float64类型。

    .. warning::
        返回的矩阵中还包含cholesky分解不使用的条目中的随机数据。如果需要将这些条目清零，请改用  :func:`mindspore.scipy.linalg.cholesky` 函数。

    参数：
        - **a** (Tensor) - 要分解的 :math:`(M,M)` 方阵。
        - **lower** (bool, 可选) - 是计算上三角还是下三角的cholesky分解。
          默认值：``False``。
        - **overwrite_a** (bool, 可选) - 是否覆盖参数 `a` 中的数据（可能会提高性能）。
          默认值：``False``。
          在MindSpore中，这个参数当前不起作用。
        - **check_finite** (bool, 可选) - 是否检查输入矩阵是否只包含有限数。
          禁用可能会带来性能增益，但如果输入确实包含INF或NaN，则可能会导致问题（崩溃、程序不终止）。
          默认值：``True``。
          在MindSpore中，当前这个参数不起作用。

    返回：
        - **c** (Tensor) - 在上三角或下三角中包含 `a` 的cholesky因子的矩阵。
          矩阵的其他部分包含随机数据。
        - **lower** (bool) - 表示cholesky因子是在下三角形还是上三角形。

    异常：
        - **ValueError** - 如果输入的Tensor不是2D方阵。
