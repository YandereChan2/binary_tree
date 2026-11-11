# `Yc::basic_intrusive_binary_tree<vptr>::nroot` , `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::nroot`

```C++
node_proxy nroot()noexcept;
node_const_proxy nroot()const noexcept;
node_const_proxy cnroot()const noexcept;
```

返回指代根 _二叉树_ 的代理；树为空时返回空代理。非 const 版本返回 `node_proxy`，
const 版本和 `cnroot` 返回 `node_const_proxy`。

与 [`root()`](root.md) 的区别：`root()` 返回根 _树位置_ 的代理，而 `nroot()` 返回处于
该 _树位置_ 上的 _二叉树_ 的代理。需要 `go_left` / `go_right` / `go_up` / `get_parent`
等导航时使用 `nroot()`。

## 参数

（无）

## 返回值

根 _二叉树_ 的代理；非 const 版本为 `node_proxy`，const 版本为 `node_const_proxy`。

## 复杂度

常数。

## 异常

（无）

## 示例

```C++
if (tree.nroot())
{
    auto left = tree.nroot().get_left();
}
```

## 备注

可向上导航版本的根 _二叉树_ 的 `get_parent()` 返回 _虚拟二叉树_ 的代理。
