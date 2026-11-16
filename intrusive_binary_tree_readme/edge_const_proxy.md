# `Yc::intrusive_binary_tree_edge_const_proxy<vptr>` , `Yc::intrusive_parent_aware_binary_tree_edge_const_proxy<vptr>`

只读的 _树位置_ 代理。除解引用返回 `const node_base&` 外，导航方式与
[`edge_proxy`](edge_proxy/edge_proxy.md) 一致。

```C++
proxy() = default;
proxy(edge_proxy e) noexcept; //(1)
proxy(const node_base& parent, bool left_child) noexcept; //(2)
proxy(const virtual_base& parent, bool left_child) noexcept; //(3) 仅可向上导航版本
```

（`virtual_base` 即可向上导航版本的
`intrusive_parent_aware_binary_tree_virtual_node_base<vptr>`。）

## 成员函数

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

## 语义

`valid` / `null` / `operator bool` / `operator*` / `get_*` 的前置条件和
[`edge_proxy`](edge_proxy/edge_proxy.md) 相同：

- 默认构造的代理 `valid() == false`，对它调用 `null()` 是行为未定义；
- 只读代理只能读取位置，不能修改树形；
- 根边上的 `go_up()` / `get_parent()` 对父感知版本是**行为未定义**（会访问
  _虚拟二叉树_ 中不存在的成员）；
- 只支持在**已纳管**节点上导航；脱管节点的槽位指向自身，导航会得到自环。

## 辅助类

### std::hash<edge_const_proxy>

对代理对象启用的 `std::hash` 特化。空代理哈希为 0；相等的代理哈希相等。
