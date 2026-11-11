# `Yc::intrusive_binary_tree_node_proxy<vptr>` , `Yc::intrusive_parent_aware_binary_tree_node_proxy<vptr>`

指代一棵 _二叉树_，并提供导航与访问。

`intrusive_binary_tree_node_proxy` 对应不可向上导航版本；
`intrusive_parent_aware_binary_tree_node_proxy` 额外提供 `go_up` / `get_parent`。

## 构造函数

```C++
proxy()noexcept; //(1)
```

```C++
explicit proxy(node_base& n)noexcept; //(2)
```

```C++
explicit proxy(
    intrusive_parent_aware_binary_tree_virtual_node_base<vptr>& n)noexcept; //(3) 仅可向上导航版本
```

```C++
explicit proxy(edge_proxy e)noexcept; //(4)
```

- (1) 空代理；
- (2) 不可向上导航版本：由 _二叉树_ 的侵入式类型引用构造；
- (3) 可向上导航版本：真实 _二叉树_ 或 `parent_of_nroot()` 返回的 _虚拟二叉树_ 都可以构造。
  _虚拟二叉树_ 只能用于导航，不能 `operator*`。
- (4) 由边代理得到该 _树位置_ 上的 _二叉树_；空 _树位置_ 得到空代理。

## 成员函数

### null

```C++
bool null()const noexcept;
```

是否不指代任何 _二叉树_。默认构造和从空 _树位置_ 取得的代理都会返回 `true`。

### operator bool

```C++
explicit operator bool()const noexcept;
```

向 `bool` 的按语境转换，等价于 `!null()`。

### operator*

```C++
node_base& operator*()const noexcept;
```

返回所指 _二叉树_ 的侵入式类型引用。若 `null() == true`，行为未定义；
可向上导航版本还要求代理指代真实 _二叉树_，对 _虚拟二叉树_ 解引用也是未定义行为。

用户拿到引用后自行转换：

```C++
auto& n = static_cast<my_binary_tree&>(*p);
```

本代理不提供 `operator->`。

### go_left, go_right

```C++
proxy& go_left()noexcept;
proxy& go_right()noexcept;
```

移动到左 / 右子 _二叉树_。若当前不按语境转换为 `true`，行为未定义；
对应孩子不存在时，代理变为空（`null() == true`）。返回 `*this` 以支持链式调用。

### go_up（仅可向上导航版本）

```C++
proxy& go_up()noexcept;
```

移动到父 _二叉树_。根 _二叉树_ 会移动到 _虚拟二叉树_；对 _虚拟二叉树_ 再次 `go_up`
是无效操作。

### get_left, get_right, get_parent

```C++
proxy get_left()const noexcept;
proxy get_right()const noexcept;
proxy get_parent()const noexcept; // 仅可向上导航版本
```

返回孩子 / 父 _二叉树_ 的代理。调用者需保证当前代理非空；可向上导航版本的
_虚拟二叉树_ 只能取 `get_left`。

### get_children

```C++
std::pair<edge_proxy, edge_proxy> get_children()const noexcept;
```

返回左右孩子所在的 _树位置_。调用者需保证当前代理非空。

### operator edge_proxy（仅可向上导航版本）

```C++
operator edge_proxy()const noexcept;
```

由一棵真实 _二叉树_ 得到它所在的 _树位置_；根 _二叉树_ 得到指向 _虚拟二叉树_ 的边。
对 _虚拟二叉树_ 调用是无效操作。

## 非成员函数

### operator==

```C++
bool operator==(proxy l, proxy r)noexcept;
```

当且仅当二者都为空，或指代同一棵 _二叉树_ 时返回 `true`。

## 辅助类

### std::hash<node_proxy>

对代理对象启用的 `std::hash` 特化。空代理哈希为 0；相等的代理哈希相等。
