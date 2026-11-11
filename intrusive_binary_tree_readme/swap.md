# `Yc::basic_intrusive_binary_tree<vptr>::swap` , `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::swap`

```C++
void swap(basic_intrusive_binary_tree& other)noexcept;
```

```C++
void swap(basic_intrusive_parent_aware_binary_tree& other)noexcept;
```

交换两棵树的根，不搬移、不复制任何 _二叉树_。

可向上导航版本在交换后会把两边的根 _二叉树_ 的父分别修正为各自树的 _虚拟二叉树_。

## 参数

||||
|-:|-|:-|
|`other`|-|要交换的另一棵树|

## 返回值

（无）

## 复杂度

常数。

## 异常

（无）

## 示例

```C++
my_binary_tree x, y;
Yc::basic_intrusive_binary_tree<> a, b;
a.set_root(a.node(x));
b.set_root(b.node(y));
a.swap(b);
assert(a.nroot() == a.node(y));
```

## 备注

自交换是无操作。交换不改变各 _二叉树_ 的子关系。
