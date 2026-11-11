# `Yc::intrusive_binary_tree_edge_const_proxy<vptr>` , `Yc::intrusive_parent_aware_binary_tree_edge_const_proxy<vptr>`

只读的 _树位置_ 代理。除解引用返回 `const node_base&` 外，导航方式与
[`edge_proxy`](edge_proxy/edge_proxy.md) 一致。

```C++
proxy() = default;
proxy(edge_proxy e) noexcept; //(1)
proxy(const node_base& parent, bool left_child) noexcept; //(2)
proxy(const virtual_base& parent, bool left_child) noexcept; //(3) 仅可向上导航版本
```

成员函数：

```C++
bool valid()const noexcept;
bool null()const noexcept;
explicit operator bool()const noexcept;

proxy& go_left()noexcept;
proxy& go_right()noexcept;
proxy& go_up()noexcept; // 仅可向上导航版本

const node_base& operator*()const noexcept;

proxy get_left()const noexcept;
proxy get_right()const noexcept;
proxy get_parent()const noexcept; // 仅可向上导航版本
std::pair<proxy, proxy> get_children()const noexcept;
bool is_left()const noexcept; // 仅可向上导航版本

friend bool operator==(proxy l, proxy r)noexcept;
```

只读代理只用于读取位置和 _二叉树_，不提供任何修改树形的操作。
