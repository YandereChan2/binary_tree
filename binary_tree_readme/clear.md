# Yc::binary_tree<T, Allocator>::clear

```C++
void clear()noexcept
```

销毁 `binary_tree` 对象内部 _树位置_ 上的 _二叉树_ ，会一并销毁该 _二叉树_ 的左右  _树位置_ 上的 _二叉树_ ，如果有。此函数如同于调用 `erase(root())` 。

## 参数

（无）

## 返回值

（无）

## 复杂度

与实际销毁的 _二叉树_ 的数量成正比。

## 异常

（无）
