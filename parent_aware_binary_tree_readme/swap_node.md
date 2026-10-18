# `Yc::parent_aware_binary_tree<T, Alloc>::swap_node`

```C++
static void swap_node(edge_const_proxy l, edge_const_proxy r)noexcept
```

交换两个 _二叉树_ 节点。实现上是先调用 [`swap_sub_tree(l, r)`](swap_sub_tree.md) 交换两个 _树位置_ 上的节点，再交换这两个节点的左右子节点位置。净效果是**两个 _树位置_ 上的值互换、子树留在原位**（树的形状不变）。

`l` 和 `r` 都必须是指代**存在的节点**的 _树位置_。传入空 _树位置_（`(bool)l == false`，例如对没有左孩子的节点取 `get_left()`）会解引用空节点，行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

两个节点是否互为父子**不需要判断**，也不需要调用方提供任何提示：先交换 _树位置_ 时，父节点里那条指向孩子的 _树位置_ 会短暂地变成自指，紧接着"成对交换孩子 _树位置_"这一步正好把它解成"父节点上位、孩子落回原位"。四种相邻形态（左右各两种顺序）与不相邻形态走的是同一条路径。`swap_sub_tree` 会同时维护 `parent` 链，所以 `go_up` 一类依赖父节点的导航在交换后依然正确。

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
