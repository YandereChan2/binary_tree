# 代理失效规则

本文说明 `Yc::binary_tree` 的四种代理在什么情况下仍然可用、什么情况下会失效。
所有“失效后使用”都是行为未定义。

## 快速规则

| 代理 | 有效条件 | 会使其失效的操作 |
|:-|:-|:-|
| `edge_proxy` / `edge_const_proxy` | 它指向的槽位仍存在 | 槽位所属的节点被销毁（失效）；槽位被清空（仍有效但 `null()`） |
| `node_proxy` / `node_const_proxy` | 它指向的节点仍存在 | 该节点被销毁（`erase`、`clear`、或丢弃拥有它的返回值） |
| 元素引用 / 指针 | 元素仍存在 | 与 `node_proxy` 相同 |

## edge_proxy

`edge_proxy` 绑定的是**槽位本身**，不是槽位里当时的节点。因此：

- `root()` 返回的根边代理在 `binary_tree` 对象生存期内一直有效；树为空时它仍然
  有效，只是 `null() == true`。
- `get_left()` / `get_right()` / `get_children()` 返回的边代理，以父节点仍然存活
  为前提；父节点被销毁后，这些代理指向已释放对象的成员，全部失效。
- `insert` / `emplace` / `splice` 会把新子树放入目标槽位，槽位本身不变；代理仍然
  有效，但 `operator*` 现在访问的是新元素。
- `erase` / `clear` / `cut` 会清空槽位；**传入这些函数的那个槽位**仍然“指向这个槽位”，
  但 `null()` 变为 `true`，不能再解引用。被删除 / 切出子树**内部**的边代理则随节点
  销毁而失效。
- `swap_sub_tree` 交换两个槽位的内容；代理仍然分别指向原来的槽位，但槽位里的
  节点换了。
- `swap_node` 在相邻父子情形下会改变节点身份；调用前取得的边代理可能从此指向
  另一个节点。需要继续操作时请重新取得代理。
- 节点的析构会连同它的 `left` / `right` 成员一起销毁；指向这些成员的边代理随之
  失效。

## node_proxy

`node_proxy` 绑定的是**节点地址**。本库的节点由分配器分配，插入、旋转、交换、
`cut` / `splice` 都不会搬移节点，所以节点代理在节点存活期间保持有效。

节点代理会在以下情况失效：

1. 节点被 `erase` / `clear` 销毁；
2. 节点属于某个函数的返回值（`insert` / `emplace` / `splice` 换出的原子树，
   或 `cut` / `recur_and_write` 切出的树），而调用方**丢弃了返回值**：临时
   `binary_tree` 的析构会销毁整棵原子树。
3. `binary_tree` 对象析构：它会 `clear()` 整棵树，所有节点被销毁。

只要节点还活着，`node_proxy` 就不受树的形状变化影响；它不关心节点当前在哪个
槽位。

## 元素引用和指针

`operator*` / `operator->` 返回的 `T&` / `T*` 与 `node_proxy` 的生存期规则相同：
节点被销毁时失效。特别地，**丢弃 `insert` / `emplace` / `splice` / `cut` /
`recur_and_write` 的返回值会销毁被换出的子树**，其中所有元素引用和指针都会失效。

## 不失效的操作

以下操作只改链接，不销毁节点，因此不会让节点代理或元素引用失效：

- `swap`、`swap_sub_tree`、`swap_node`；
- `left_rotate` / `right_rotate`；
- `insert` / `emplace` / `splice` / `cut` / `recur_and_write` 本身（在返回值被保留
  的前提下）。

它们可能让 `edge_proxy` 指向不同的节点，或者让某个槽位变空；具体见上文。
