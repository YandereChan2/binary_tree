# `Yc::basic_intrusive_binary_tree<vptr>::clear` , `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::clear`

```C++
void clear()noexcept;
```

拆掉整棵树，但**不销毁任何 _二叉树_**。

- 所有 _二叉树_ 都会变成没有孩子的孤立 _二叉树_；
- 可向上导航版本还会断开所有 _二叉树_ 与父 _二叉树_ 的关系。

## 参数

（无）

## 返回值

（无）

## 复杂度

与 _二叉树_ 的数量成线性关系。

## 异常

（无）

## 示例

```C++
tree.clear();
assert(tree.empty());
// 各 _二叉树_ 仍然存活，需要由调用方销毁
```

## 备注

本函数等价于用空回收器调用 [`erase`](erase.md) 拆掉根。
