# `Yc::basic_intrusive_binary_tree<vptr>::swap` , `Yc::basic_intrusive_parent_aware_binary_tree<vptr>::swap`

```C++
// 成员函数
void swap(basic_intrusive_binary_tree& other)noexcept;

void swap(basic_intrusive_parent_aware_binary_tree& other)noexcept;

// 非成员函数（ADL）
template<class vptr>
void swap(basic_intrusive_binary_tree<vptr>&,
          basic_intrusive_binary_tree<vptr>&)noexcept;

template<class vptr>
void swap(basic_intrusive_parent_aware_binary_tree<vptr>&,
          basic_intrusive_parent_aware_binary_tree<vptr>&)noexcept;
```

交换两棵树的根，不搬移、不复制任何 _二叉树_。

可向上导航版本在交换后会把两边的根 _二叉树_ 的父分别修正为各自树的 _虚拟二叉树_。

## 参数

|||
|-:|:-|
|`other`|要交换的另一棵树|

## 返回值

（无）。

## 复杂度

常数。

## 异常

（无）。

## 示例

```C++
using tree = Yc::basic_intrusive_binary_tree<>;
my_binary_tree x(1), y(2);
tree a, b;
tree::insert(a.root(), x);
tree::insert(b.root(), y);
Yc::swap(a, b);
assert(a.nroot() == tree::node_proxy{ y });
a.clear();
b.clear();
```

## 备注

- 自交换是无操作。
- 交换不改变各 _二叉树_ 的子关系；父感知版本只修正两个根 _二叉树_ 的 `parent`。
- `swap` 与两个树对象析构无关：它只换根，不会 `clear()` 任何一方。
