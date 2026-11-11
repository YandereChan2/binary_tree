# `Yc::basic_intrusive_binary_tree<vptr>::clear` , `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::clear`

```C++
void clear()noexcept;
```

拆掉整棵树，但**不销毁任何 _二叉树_**。

- 所有 _二叉树_ 都会**脱管**：所有链接字段都指向自身；
- 脱管后的 _二叉树_ 可以安全析构；
- 可向上导航版本中，父关系也随脱管一起清除。

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

本函数等价于 [`erase(root())`](erase.md)。
