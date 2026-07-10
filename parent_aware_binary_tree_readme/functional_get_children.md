# `Yc::parent_aware_binary_tree_functional::get_children`

在文件 `"parent_aware_binary_tree.h"` 定义

```C++
namespace Yc::parent_aware_binary_tree_functional
{
    constexpr inline Yc::details::parent_aware_binary_tree_get_children_t get_children{};
}
```

`Yc::parent_aware_binary_tree_functional::get_children` 是一个[_定制点对象 (customization point object)_](https://zh.cppreference.com/w/cpp/language/customization_point)，能够统一地对四种代理类型（[`edge_proxy`](edge_proxy/edge_proxy.md)、[`edge_const_proxy`](edge_proxy/edge_proxy.md)、[`node_proxy`](node_proxy/node_proxy.md)、[`node_const_proxy`](node_proxy/node_proxy.md)）调用 `get_children()`。

主要用于泛型编程场景，例如配合 [`recur_and_write`](recur_and_write.md) 或[构造函数](constructor.md)（5）进行递归树构建，而无需关心传入的具体代理类型。

## 返回值

返回一个 `std::pair`，其元素类型取决于传入的代理类型：

- 传入 `edge_proxy`：返回 `std::pair<edge_proxy, edge_proxy>`
- 传入 `edge_const_proxy`：返回 `std::pair<edge_const_proxy, edge_const_proxy>`
- 传入 `node_proxy`：返回 `std::pair<edge_proxy, edge_proxy>`
- 传入 `node_const_proxy`：返回 `std::pair<edge_const_proxy, edge_const_proxy>`

## 使用示例

```C++
// 在泛型代码中统一获取子节点
auto children = Yc::parent_aware_binary_tree_functional::get_children(proxy);
```
