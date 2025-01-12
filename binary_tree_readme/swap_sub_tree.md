# `Yc::binary_tree<T,Allocator>::swap_sub_tree`

```C++
static void swap_sub_tree(edge_const_proxy l, edge_const_proxy r)noexcept
```

交换两个 _树位置_ 上的 _二叉树_ 。如果其中一个 _树位置_ 没有二叉树，那么相当于从另一个 _树位置_ 进行二叉树转移。如果都没有，则无操作。

如果两个 _树位置_ 的来源 `binary_tree` 的分配器不比较相等，则行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

## 参数

||||
|-:|-|:-|
|`l` , `r`|-|指代要进行交换的 _树位置_ |

## 返回值

（无）

## 复杂度

常数

## 异常

（无）
