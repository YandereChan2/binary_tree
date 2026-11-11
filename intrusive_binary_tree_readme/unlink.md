# `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::unlink`

```C++
static node_proxy unlink(edge_proxy e)noexcept;
```

摘下 `e` 所指代的 _树位置_ 上的整棵子树，并使该 _树位置_ 变空。

与不可向上导航版本使用 [`cut`](cut.md) 不同，可向上导航版本的边已经带有父 _二叉树_ 信息，
因此 `unlink` 还能断开子树根与父 _二叉树_ 的关系。

## 参数

||||
|-:|-|:-|
|`e`|-|要摘下的子树所在的 _树位置_|

## 返回值

被摘下的子树根的代理；`e` 为空时返回空代理。

## 复杂度

常数。

## 异常

（无）

## 示例

```C++
auto sub = tree.unlink(tree.root().get_right());
assert(!sub.get_parent()); // 已与旧树断开

Yc::basic_intrusive_parent_aware_binary_tree<> other;
other.set_root(sub);
```

## 备注

本函数不销毁 _二叉树_。返回的子树需要由调用方接管或销毁。
