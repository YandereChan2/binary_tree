# `Yc::child_check`

在文件 `"parent_aware_binary_tree.h"` 定义

```C++
enum child_check
{
    no_check = 0,
    check_left = 1,
    check_right = 2,
    check_left_rev = 4,
    check_right_rev = 8,
    check_all = 15, // check_all == check_left | check_right | check_left_rev | check_right_rev
};
```

`Yc::child_check` 是一个[_位掩码类型 (BitmaskType)_](https://zh.cppreference.com/w/cpp/named_req/BitmaskType)，用于 [`swap_node`](swap_node.md) 函数中指定是否需要检查两个交换节点之间的邻接关系（即互为父子节点的情况）。

当两个节点互为父子节点时，不能简单地分别交换节点位置和左右子树，而需要一种特殊的交换算法来处理。`child_check` 的各个位标志指示了 `swap_node` 应当检查哪些邻接情况。

## 枚举项

|枚举项|整数值|说明|
|-:|-:|:-|
|`no_check`|`0`|不检查任何邻接关系。假定两个节点不相邻。|
|`check_left`|`1`|检查 `l` 的左子节点是否为 `r`。|
|`check_right`|`2`|检查 `l` 的右子节点是否为 `r`。|
|`check_left_rev`|`4`|检查 `r` 的左子节点是否为 `l`。|
|`check_right_rev`|`8`|检查 `r` 的右子节点是否为 `l`。|
|`check_all`|`15`|等于 `check_left \| check_right \| check_left_rev \| check_right_rev`，默认值，可处理大多数常见情况。|

## 使用方式

```C++
// 使用默认值，检查所有常见邻接关系
tree_type::swap_node(p1, p2);

// 只检查 l 的右子节点是否为 r
tree_type::swap_node(p1, p2, child_check::check_right);

// 确定两个节点不相邻，不进行检查
tree_type::swap_node(p1, p2, child_check::no_check);
```
