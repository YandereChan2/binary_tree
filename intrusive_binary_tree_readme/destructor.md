# `Yc::basic_intrusive_binary_tree<vptr>::~basic_intrusive_binary_tree` , `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::~basic_intrusive_parent_aware_binary_tree`

```C++
~basic_intrusive_binary_tree();
```

```C++
~basic_intrusive_parent_aware_binary_tree();
```

析构侵入式树对象。两个析构函数都等价于调用 [`clear()`](clear.md)。

## 说明

析构**不会销毁任何 _二叉树_**，但会：

- 遍历整棵树并让所有 _二叉树_ **脱管**（所有链接字段指回自身）；
- 对可向上导航版本，清除根 _二叉树_ 指向本树 `_虚拟二叉树_` 的父指针，因此不会留下
  悬垂的 `parent`；
- 结束树对象自身。

因此，树对象析构后，原来的节点仍然存活且处于脱管状态，可以安全析构，也可以重新
`insert` 到新的树里。这与“销毁节点”是两件事。

## 复杂度

与树中 _二叉树_ 的数量成线性关系。

## 异常

（无）。`clear()` 只改链接，不销毁节点。
