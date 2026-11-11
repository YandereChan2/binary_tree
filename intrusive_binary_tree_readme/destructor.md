# `Yc::basic_intrusive_binary_tree<vptr>::~basic_intrusive_binary_tree` , `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::~basic_intrusive_parent_aware_binary_tree`

```C++
~basic_intrusive_binary_tree() = default;
```

```C++
~basic_intrusive_parent_aware_binary_tree() = default;
```

析构侵入式树对象。

## 说明

析构**不会**销毁任何 _二叉树_，也**不会**遍历或拆解树：

- 不可向上导航版本：只结束树对象自身；
- 可向上导航版本：只结束树对象自身，各 _二叉树_ 之间的父子关系保持不变。

销毁树对象前，各 _二叉树_ 必须已经被 [`cut`](cut.md) / [`unlink`](unlink.md) /
[`erase`](erase.md) / [`clear`](clear.md) 摘下，或由调用方接管。

## 复杂度

常数。

## 异常

（无）
