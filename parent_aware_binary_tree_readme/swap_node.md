# `Yc::parent_aware_binary_tree<T, Alloc>::swap_node`

```C++
static void swap_node(edge_const_proxy l, edge_const_proxy r)
```

交换两个 _二叉树_ 节点，同时交换它们各自的子树。相当于先调用 [`swap_sub_tree(l, r)`](swap_sub_tree.md) 交换节点位置，再交换左右子节点位置。

如果两个 _树位置_ 的来源 `parent_aware_binary_tree` 的分配器不比较相等，则行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

## 参数

||||
|-:|-|:-|
|`l` , `r`|-|指代要进行交换的两个 _二叉树_ 对应的 _树位置_ |

## 返回值

（无）

## 复杂度

常数

## 异常

（无）
