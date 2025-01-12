# `Yc::binary_tree<T,Allocator>::cut`

```C++
binary_tree emplace(edge_const_proxy p)noexcept;
```

将 `p` 所指代的 _树位置_ 处的 _二叉树_ 通过返回值转移到外部。

## 参数

||||
|-:|-|:-|
|`p`|-|指代要移除的 _二叉树_ 的 _树位置_ |

## 返回值

原先处于该 _树位置_ 的 _二叉树_ 。如果没有，则返回的 `binary_tree` 对象内部的 _树位置_ 上没有 _二叉树_ 。

## 复杂度

常数

## 异常

（无）
