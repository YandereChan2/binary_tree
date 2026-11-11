# `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::parent_of_nroot`

```C++
node_proxy parent_of_nroot()noexcept;
node_const_proxy parent_of_nroot()const noexcept;
node_const_proxy parent_of_cnroot()const noexcept;
```

返回指代 _虚拟二叉树_ 的代理。非 const 版本返回 `node_proxy`，const 版本和
`parent_of_cnroot` 返回 `node_const_proxy`。

_虚拟二叉树_ 是可向上导航版本中根 _二叉树_ 的父：

- 根 _二叉树_ 的 `get_parent()` 返回它；
- 它的 `get_left()` 是根 _二叉树_；
- 它本身不包含用户数据。

该代理永远不会为空。它主要用于比较与导航：

```C++
assert(tree.nroot().get_parent() == tree.parent_of_nroot());
assert(tree.parent_of_nroot().get_left() == tree.nroot());
```

## 参数

（无）

## 返回值

_虚拟二叉树_ 的代理；非 const 版本为 `node_proxy`，const 版本为 `node_const_proxy`。

## 复杂度

常数。

## 异常

（无）

## 备注

对 _虚拟二叉树_ 调用 `operator*` / `get_right` / `get_parent` 是无效操作。
