# `Yc::intrusive_binary_tree_edge_proxy<vptr>` , `Yc::intrusive_parent_aware_binary_tree_edge_proxy<vptr>`

指代一个 _树位置_ ，即父 _二叉树_ 到子 _二叉树_ 之间的位置。

- 不可向上导航版本：_树位置_ 直接由边代理指代（内部是“指向槽位的指针”）；
- 可向上导航版本：_树位置_ 由父 _二叉树_ 与方向（左 / 右）共同指代；根 _树位置_
  的父是 _虚拟二叉树_ 。

## 构造函数

```C++
proxy()noexcept; //(1)
```

```C++
proxy(node_base& parent, bool left_child)noexcept; //(2) 不可向上导航版本
```

```C++
proxy(node_proxy parent, bool left_child)noexcept; //(3)
```

```C++
proxy(intrusive_parent_aware_binary_tree_virtual_node_base<vptr>& parent,
      bool left_child)noexcept; //(4) 仅可向上导航版本
```

- (1) 空代理；
- (2) 由父 _二叉树_ 与方向构造子 _树位置_ ；
- (3) 由父 _二叉树_ 的代理与方向构造子 _树位置_ ；
- (4) 可向上导航版本：父可以是真实 _二叉树_ ，也可以是 _虚拟二叉树_ （根 _树位置_
  永远是左）。

**父节点必须是已纳管节点（或可向上导航版本的 _虚拟二叉树_）。** 对脱管节点取边，
得到的是指向节点自身的槽位，而不是空边；在此之上导航会得到自环。

由 _二叉树_ 得到边的操作使用 [`node_proxy`](../node_proxy/node_proxy.md) 的转换函数。
注意：**只有可向上导航版本**的 `node_proxy` 提供 `operator edge_proxy`；不可向上
导航版本没有这个转换，只能通过 `get_children()` 取得子树位置。

## 成员函数

### valid

```C++
bool valid()const noexcept;
```

是否指代一个 _树位置_ 。默认构造的代理返回 `false`。有效的 _树位置_ 上也可以没有
_二叉树_ （`null() == true`）。

### null

```C++
bool null()const noexcept;
```

该 _树位置_ 上是否没有 _二叉树_ 。`valid() == false` 时调用是未定义行为。

### operator bool

```C++
explicit operator bool()const noexcept;
```

等价于 `valid() && !null()`。

### go_left, go_right

```C++
proxy& go_left()noexcept;
proxy& go_right()noexcept;
```

移动到当前 _二叉树_ 的左 / 右子 _树位置_ 。当前代理不按语境转换为 `true` 时行为
未定义。返回 `*this` 以支持链式调用。

### go_up（仅可向上导航版本）

```C++
proxy& go_up()noexcept;
```

从子 _树位置_ 移动到父 _树位置_ 。根 _树位置_ 的父是 _虚拟二叉树_ ，不能再向上；
对根 _树位置_ 调用会读取 _虚拟二叉树_ 中不存在的 `parent`，是**行为未定义**，不是
no-op。

### operator*

```C++
node_base& operator*()const noexcept;
```

返回该 _树位置_ 上的 _二叉树_ 的侵入式类型引用。当前代理不按语境转换为 `true` 时
行为未定义。本代理不提供 `operator->`。

### get_left, get_right

```C++
proxy get_left()const noexcept;
proxy get_right()const noexcept;
```

返回当前 _二叉树_ 的左 / 右子 _树位置_ 。当前代理必须非空，否则行为未定义。

### get_parent（仅可向上导航版本）

```C++
proxy get_parent()const noexcept;
```

返回父 _树位置_ 。根 _树位置_ 没有父 _树位置_ ；对根 _树位置_ 调用是**行为未定义**。

### get_children

```C++
std::pair<proxy, proxy> get_children()const noexcept;
```

返回当前 _二叉树_ 的左右子 _树位置_ 。当前代理必须非空，否则行为未定义。

### is_left（仅可向上导航版本）

```C++
bool is_left()const noexcept;
```

当前 _树位置_ 是否为父 _二叉树_ 的左子位置。根 _树位置_ 也构造为“左”，但它没有
真实父节点；不要依赖根边上的 `is_left()`。

## 非成员函数

### operator==

```C++
bool operator==(proxy l, proxy r)noexcept;
```

不可向上导航版本：当且仅当二者都无效，或指代同一个 _树位置_ 时返回 `true`。
可向上导航版本：当且仅当父 _二叉树_ 与方向都相同时返回 `true`。

## 辅助类

### std::hash<edge_proxy>

对代理对象启用的 `std::hash` 特化。空代理哈希为 0；相等的代理哈希相等。

## 失效规则

边代理绑定的是槽位。`erase` / `clear` / `cut` 会让槽位变空；`insert` / `emplace` /
`splice` 会替换槽位内容；`swap_sub_tree` / `swap_node` 可能让槽位指向另一个节点；
节点销毁会让槽位失效。需要长期保存时，优先保存 [`node_proxy`](../node_proxy/node_proxy.md)，
或在使用前重新取得边代理。完整规则见[代理失效规则](../proxy_lifetime.md)。
