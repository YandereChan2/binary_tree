# `Yc::basic_intrusive_binary_tree<vptr>::cut`

```C++
static node_proxy cut(edge_proxy e)noexcept;
```

摘下 `e` 所指代的 _树位置_ 上的整棵子树，并使该 _树位置_ 变空。
子树自身的父子关系保持不变。

可向上导航版本没有同名的 `cut`：它的边已经带有父 _二叉树_ 信息，直接使用
[`unlink`](unlink.md) 完成同样的操作，并额外断开子树根与父 _二叉树_ 的关系。

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
auto sub = tree.cut(tree.root().get_right());

Yc::basic_intrusive_binary_tree<> other;
other.set_root(sub);
```

## 备注

本函数不销毁 _二叉树_。返回的子树需要由调用方接管或销毁。
