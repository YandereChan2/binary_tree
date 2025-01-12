# Yc::binary_tree<T, Allocator>::erase

```C++
void erase(edge_const_proxy p)noexcept
```

如果存在，则销毁位于 `p` 所指代的 _树位置_ 上的 _二叉树_ 。调用后，`p` 所指代的 _树位置_ 依然有效，但是其上不再存在 _二叉树_ 。换言之， `p.null() == true` 。

如果 `p` 不从 `root()` 可达，或者 `p` 不指代合法的 _树位置_ ，行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

## 参数

||||
|-:|-|:-|
|`p`|-|指代要移除的 _二叉树_ 的 _树位置_ |

## 返回值

（无）

## 复杂度

与所有由 `p` 可达的 _二叉树_ 的数量成线性关系。

## 异常

（无）
