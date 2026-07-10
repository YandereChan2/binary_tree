# `Yc::parent_aware_binary_tree<T, Alloc>::swap_node`

```C++
static void swap_node(edge_const_proxy l, edge_const_proxy r, child_check chk = check_all)noexcept
```

交换两个 _二叉树_ 节点，同时交换它们各自的子树。相当于先调用 [`swap_sub_tree(l, r)`](swap_sub_tree.md) 交换节点位置，再交换左右子节点位置。

参数 `chk` 指定了需要检查的特殊邻接情况。当且仅当 `l` 和 `r` 互为父子关系且满足对应检查位时，`swap_node` 会采取特殊的交换策略以确保正确性。检查位的语义如下：

- `check_left`：检查 `l` 的左子节点是否为 `r`（即 `r` 是 `l` 的直接左子节点）。
- `check_right`：检查 `l` 的右子节点是否为 `r`。
- `check_left_rev`：检查 `r` 的左子节点是否为 `l`。
- `check_right_rev`：检查 `r` 的右子节点是否为 `l`。
- `check_all`：等于 `check_left | check_right | check_left_rev | check_right_rev`。
- `no_check`：不检查任何邻接关系，假定 `l` 和 `r` 不相邻。

默认参数 `check_all` 可以处理大多数常见情况。如果不确定邻接关系，建议保持默认值。如果确定 `l` 和 `r` 不相邻，可以使用 `no_check` 以提高性能。

如果两个 _树位置_ 的来源 `parent_aware_binary_tree` 的分配器不比较相等，则行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

## 参数

||||
|-:|-|:-|
|`l` , `r`|-|指代要进行交换的两个 _二叉树_ 对应的 _树位置_ |
|`chk`|-|指示需要检查的邻接关系，见 [`Yc::child_check`](child_check.md)|

## 返回值

（无）

## 复杂度

常数

## 异常

（无）
