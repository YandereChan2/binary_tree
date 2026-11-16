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
- (3) 由只读边代理取得该 _树位置_ 上的 _二叉树_ 。

## 成员函数

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

对 _虚拟二叉树_ 调用这个转换同样是行为未定义；它只适用于真实 _二叉树_。

## 语义

- `null()` / `operator*` 的前置条件与 [`node_proxy`](node_proxy/node_proxy.md)
  相同；空代理解引用是行为未定义。
- 只读代理不能在脱管节点上导航；脱管节点的链接指向自身，导航会得到自环。
- 父感知版本中，_虚拟二叉树_ 只支持 `get_left` / `go_left`；对它调用
  `get_right` / `get_parent` / `go_up` / `operator*` 都是行为未定义。
- 不能通过只读代理修改树。

## 辅助类

### std::hash<node_const_proxy>

对代理对象启用的 `std::hash` 特化。空代理哈希为 0；相等的代理哈希相等。
