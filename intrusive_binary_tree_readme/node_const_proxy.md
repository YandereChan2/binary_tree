# `Yc::intrusive_binary_tree_node_const_proxy<vptr>` , `Yc::intrusive_parent_aware_binary_tree_node_const_proxy<vptr>`

只读的 _二叉树_ 代理。除解引用返回 `const node_base&` 外，导航方式与
[`node_proxy`](node_proxy/node_proxy.md) 一致。

```C++
proxy() = default;
proxy(node_proxy p) noexcept; //(1)
explicit proxy(const node_base& n) noexcept; //(2)
explicit proxy(edge_const_proxy e) noexcept; //(3)
```

- (1) 由可修改代理构造；
- (2) 由 _二叉树_ 的侵入式类型引用构造；
- (3) 由只读边代理取得该 _树位置_ 上的 _二叉树_。

成员函数：

```C++
bool null()const noexcept;
explicit operator bool()const noexcept;
const node_base& operator*()const noexcept;

proxy& go_left()noexcept;
proxy& go_right()noexcept;
proxy& go_up()noexcept; // 仅可向上导航版本

proxy get_left()const noexcept;
proxy get_right()const noexcept;
proxy get_parent()const noexcept; // 仅可向上导航版本
std::pair<edge_const_proxy, edge_const_proxy> get_children()const noexcept;

friend bool operator==(proxy l, proxy r)noexcept;
```

可向上导航版本还提供到 `edge_const_proxy` 的转换：

```C++
operator edge_const_proxy()const noexcept;
```

`const` 版本只能读取 _二叉树_ 基类，不能通过它修改树。
