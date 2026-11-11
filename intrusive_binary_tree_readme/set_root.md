# `Yc::basic_intrusive_binary_tree<vptr>::set_root` , `Yc::basic_intrusive_binary_tree<vptr>::release_root` , `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::set_root` , `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::release_root`

```C++
void set_root(node_proxy n)noexcept; //(1)
```

```C++
node_proxy release_root()noexcept; //(2)
```

- (1) 接管一棵已经与原来的树断开的子树，把它作为本树的根。
- (2) 摘下整棵树，根 _树位置_ 变空，返回被摘下的根 _二叉树_ 的代理。

可向上导航版本会同时维护父子关系：

- `set_root` 把根 _二叉树_ 的父设为当前树的 _虚拟二叉树_；
- `release_root` 断开根 _二叉树_ 与父 _二叉树_ 的关系。

## 参数

||||
|-:|-|:-|
|`n`|-|要接管的根 _二叉树_（可以为空代理）|

## 返回值

- (1) （无）
- (2) 原先的根 _二叉树_ 的代理；树为空时返回空代理。

## 复杂度

常数。

## 异常

（无）

## 示例

```C++
Yc::basic_intrusive_binary_tree<> src;
auto subtree = src.release_root();

Yc::basic_intrusive_binary_tree<> dst;
dst.set_root(subtree);
```

## 备注

本函数不销毁、不创建 _二叉树_。`n` 必须是一棵与任何其他树断开的子树的根；
把它同时挂到两棵树上会使树损坏。
