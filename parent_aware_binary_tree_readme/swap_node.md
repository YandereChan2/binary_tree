# `Yc::parent_aware_binary_tree<T, Alloc>::swap_node`

```C++
static void swap_node(edge_const_proxy l, edge_const_proxy r)noexcept
```

交换两个 _二叉树_ 节点。净效果是**两个 _树位置_ 上的值互换、子树留在原位**（树的形状不变）。

`l` 和 `r` 都必须是指代**存在的节点**的 _树位置_。传入空 _树位置_（`(bool)l == false`，例如对没有左孩子的节点取 `get_left()`）会解引用空节点，行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

两个节点是否互为父子**不影响结果**，因此不需要判断，也不需要调用方提供任何提示。四种相邻形态（左右各两种顺序）与不相邻形态的结果一致。交换后向上导航（`go_up` 一类）依然正确。

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
