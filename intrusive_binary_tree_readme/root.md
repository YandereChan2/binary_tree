# `Yc::basic_intrusive_binary_tree<vptr>::root` , `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::root`

```C++
edge_proxy root()noexcept; //(1)
```

```C++
edge_const_proxy root()const noexcept; //(2)
```

```C++
edge_const_proxy croot()const noexcept; //(3)
```

返回指代根 _树位置_ 的代理。

- 不可向上导航版本：根 _树位置_ 直接由该代理指代；
- 可向上导航版本：根 _树位置_ 的父是 _虚拟二叉树_。

返回的代理在树对象生存期内始终有效，即使树为空（此时 `root().null() == true`）。

## 参数

（无）

## 返回值

指代根 _树位置_ 的代理；非 const 版本返回 `edge_proxy`，const 版本和 `croot` 返回
`edge_const_proxy`。

## 复杂度

常数。

## 异常

（无）

## 示例

```C++
auto e = tree.root();
if (e) { /* 根 _二叉树_ 存在 */ }
auto left = e.get_left();
```

## 备注

可向上导航版本的根 _树位置_ 不能再向上；对根调用 `go_up()` / `get_parent()` 是无效操作。
需要根 _二叉树_ 的代理时使用 [`nroot()`](nroot.md)。
