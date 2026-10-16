# API 清单

两个头文件的完整公开成员。`p` 一律指 `edge_const_proxy`；`noexcept` 与 `const` 按源码照录。

---

## `Yc::binary_tree<T, Alloc = std::allocator<T>>`

`#include "binary_tree.h"`

### 类型别名

| 名称 | 定义 |
|---|---|
| `allocator_type` | `Alloc` |
| `value_type` | `T` |
| `reference` | `T&` |
| `pointer` | `std::allocator_traits<Alloc>::pointer`（分配给节点的存储、节点的左右 _树位置_ 与根都由它表示；`Alloc::pointer` 为 fancy pointer 时即为该 fancy pointer） |
| `edge_proxy` | `details::binary_tree_edge_proxy<T, placeholder_alloc>` |
| `edge_const_proxy` | `details::binary_tree_edge_const_proxy<T, placeholder_alloc>` |
| `node_proxy` | `details::binary_tree_node_proxy<T, placeholder_alloc>` |
| `node_const_proxy` | `details::binary_tree_node_const_proxy<T, placeholder_alloc>` |

### 构造 / 赋值 / 析构

| 签名 | 说明 |
|---|---|
| `binary_tree() = default` | 空树 |
| `explicit binary_tree(const Alloc& a) noexcept` | 仅指定分配器 |
| `template<class VG, class CG, class IH> binary_tree(VG vg, CG cg, IH h, const Alloc& a = {})` | 从外部树形结构构建；`h` 需可 `bool` 转换 |
| `binary_tree(const binary_tree&)` | 深拷贝（`select_on_container_copy_construction`） |
| `binary_tree& operator=(const binary_tree&)` | 带 `_guard` 的强异常安全；开头有自赋值检查，`a = a` 安全且不会清空 |
| `binary_tree(binary_tree&&) noexcept` | `exchange(root_ptr)` + 移动分配器 |
| `binary_tree& operator=(binary_tree&&) noexcept(POCMA \|\| is_always_equal)` | 分配器不等时逐个移动 |
| `~binary_tree()` | `clear()` |

### 容量与导航

| 签名 | 说明 |
|---|---|
| `Alloc get_allocator() const noexcept` | |
| `bool empty() const noexcept` | `root_ptr == nullptr` |
| `edge_proxy root() noexcept` / `edge_const_proxy root() const noexcept` | 指向树自身 `root_ptr` 的边 |
| `edge_const_proxy croot() const noexcept` | |
| `node_proxy nroot() noexcept` / `node_const_proxy nroot() const noexcept` | 根节点 |
| `node_const_proxy cnroot() const noexcept` | |

### 修改（注意返回值）

| 签名 | 说明 |
|---|---|
| `binary_tree cut(edge_const_proxy p) noexcept` | 把 `p` 处子树摘下来，返回持有它的新树；原槽位置空 |
| `void erase(edge_const_proxy p) noexcept` | 删除 `p` 处节点**及其整棵子树**（迭代实现） |
| `void clear() noexcept` | `erase(root())` |
| `template<class... Args> binary_tree emplace(edge_const_proxy p, Args&&... args)` | 在 `p` 处原地构造，**返回被顶掉的旧子树** |
| `binary_tree insert(edge_const_proxy p, const T& v)` | `emplace(p, v)` |
| `binary_tree insert(edge_const_proxy p, T&& v)` | `emplace(p, std::move(v))` |
| `binary_tree splice(edge_const_proxy to, edge_const_proxy from) noexcept` | 把 `from` 子树挂到 `to`，**返回 `to` 原有的子树** |
| `binary_tree splice(edge_const_proxy p, binary_tree& tree) noexcept` | 拼入整棵树 |
| `binary_tree splice(edge_const_proxy p, binary_tree&& tree) noexcept` | 同上 |
| `static void swap_sub_tree(edge_const_proxy l, edge_const_proxy r) noexcept` | 交换两个槽位 |
| `void swap(binary_tree& other) noexcept` | POCS 感知 |
| `static void swap_node(edge_const_proxy l, edge_const_proxy r, child_check chk = check_all) noexcept` | 交换两个节点**连同各自子树**；默认识别四种父子相邻形态并正确重连，确认不相邻时可传 `no_check` |

### 遍历

| 签名 | 说明 |
|---|---|
| `template<class F> void for_each(F&& f) noexcept` | 中序遍历，回调收到 `T&` |
| `template<class F> void for_each(F&& f) const noexcept` | 中序遍历，回调收到 `const T&` |

供容器在销毁节点前做收尾（如哈希表先把所有桶摘链，再 `clear()` 销毁桶节点）。`f` 不得改变树结构。右子树走循环不递归，有左孩子时才递归一次，递归深度 ≤ 树高；析构走的 `erase()` 是纯迭代。

### 旋转

| 签名 | 说明 |
|---|---|
| `void left_rotate(edge_const_proxy p) noexcept` | 右子提升到 `p` 的位置 |
| `void right_rotate(edge_const_proxy p) noexcept` | 左子提升 |
| `bool checked_left_rotate(edge_const_proxy p) noexcept` | 右子不存在则返回 `false` 且不动 |
| `bool checked_right_rotate(edge_const_proxy p) noexcept` | 左子不存在则返回 `false` 且不动 |

旋转内部用 `cut`/`splice` 实现，并断言中间树必须为空；违反则 `std::unreachable()`。

### 递归写入

| 签名 | 说明 |
|---|---|
| `template<class VG, class CG, class IH> binary_tree recur_and_write(edge_const_proxy p, VG vg, CG cg, IH h)` | 从 `h` 描述的树写入 `p`；**返回 `p` 处原有的子树** |

- `vg`：`h -> T`（值获取）
- `cg`：`h -> pair<左句柄, 右句柄>`
- `h`：当前句柄，`bool` 可转换，`false` 表示空
- ⚠️ `p` 处**必须为空**，否则 `std::unreachable()`

### 自由函数与函数对象

| 名称 | 说明 |
|---|---|
| `void swap(binary_tree<T,Alloc>& l, binary_tree<T,Alloc>& r) noexcept` | 转发到成员 `swap` |
| `Yc::binary_tree_functional::get_children` | `constexpr inline` 对象，可直接作 `CG`（对 4 种代理均可调用） |

---

## `Yc::parent_aware_binary_tree<T, Alloc = std::allocator<T>>`

`#include "parent_aware_binary_tree.h"`

上表中除下列差异外**全部相同**（`swap_node` / `child_check` / `for_each` 两个实现都有，签名与语义一致）：

`pointer` 同样为 `std::allocator_traits<Alloc>::pointer`；在父感知版本中它另外还表示节点的 `parent` 与哨兵节点。

### 额外的导航

| 签名 | 说明 |
|---|---|
| `node_proxy parent_of_nroot() noexcept` / `node_const_proxy parent_of_nroot() const noexcept` | 哨兵节点本身（"根的父节点"） |
| `node_const_proxy parent_of_cnroot() const noexcept` | |

`root()` 返回的是**哨兵 → 真根**的边；`nroot()` 等价于 `parent_of_nroot().go_left()`。

### 节点 → 边 的转换

`parent_aware_binary_tree` 的节点代理能回到"指向它的边"（靠节点自己的 `parent` 指针）：`node_proxy` 有 `operator edge_proxy()`，`node_const_proxy` 有 `operator edge_const_proxy()`。转换由**节点侧**的转换运算符提供，边代理一侧不再另设转换构造函数（两头都写会二义）。`binary_tree` 没有父指针，无法从节点回到边，只能用 `get_children()` 拿子边。

### 移动构造

`parent_aware_binary_tree(parent_aware_binary_tree&& other) noexcept : alloc{std::move(other.alloc)} { swap_sub_tree(root(), other.root()); }`

—— 与 `binary_tree` 一样**移动**分配器，并通过交换根槽位搬空对方。

### 自由函数与函数对象

| 名称 | 说明 |
|---|---|
| `void swap(parent_aware_binary_tree<T,Alloc>&, parent_aware_binary_tree<T,Alloc>&) noexcept` | |
| `Yc::parent_aware_binary_tree_functional::get_children` | 同 `binary_tree_functional::get_children` |

---

## 共通类型：`Yc::child_check`

```cpp
enum child_check {
    no_check   = 0,
    check_left = 1,   // l.get_left()  == r
    check_right= 2,   // l.get_right() == r
    check_left_rev  = 4,   // l == r.get_left()
    check_right_rev = 8,   // l == r.get_right()
    check_all  = 15,       // 四项之并
};
```

两个头文件**共用同一份定义**（宏守卫，可同时包含）。两个被交换的节点互为父子时，直接交换槽位会破坏结构（互指成环、丢节点），所以 `swap_node` 对四种相邻形态各有专门的重连分支；都不命中时退回朴素交换（先换槽位、再换各自子槽位）。传 `no_check` 可强制走朴素路径。

---

## 代理接口

### `edge_proxy` / `edge_const_proxy`

| 成员 | 返回 | 说明 |
|---|---|---|
| `valid() const noexcept` | `bool` | 边自身有效（槽位指针非空） |
| `null() const noexcept` | `bool` | 边所指的**子节点为空** |
| `explicit operator bool() const noexcept` | | `valid() && !null()` |
| `operator*() const noexcept` | `T&` / `const T&` | 子节点中存储的值 |
| `operator->() const noexcept` | `T*` / `const T*` | |
| `go_left()` / `go_right() noexcept` | 自身引用 | 移到左/右子**边** |
| `go_up() noexcept` | 自身引用 | **仅 `parent_aware`** |
| `get_left()` / `get_right() const noexcept` | 新代理 | 不改自身 |
| `get_parent() const noexcept` | 新代理 | **仅 `parent_aware`** |
| `get_children() const noexcept` | `pair<edge,edge>` | `{get_left(), get_right()}` |
| `operator==` | `bool` | **比较槽位身份**，不比较值 |
| `value_type` | `T` | 四个代理一律是 `T`（与 const 迭代器约定一致，不带 const） |
| `reference` | `T&` / `const T&` | const 性由它表达 |

构造函数（`public`）：默认；拷贝；`edge_const_proxy` 可由 `edge_proxy` 隐式构造（共享槽位）。`parent_aware` 另有 `(pointer parent, bool left_flag)`。

### `node_proxy` / `node_const_proxy`

| 成员 | 返回 | 说明 |
|---|---|---|
| `null() const noexcept` | `bool` | 节点指针为空 |
| `explicit operator bool() const noexcept` | | `!null()` |
| `operator*()` / `operator->()` | | 节点中的值 |
| `go_left()` / `go_right()` | 自身引用 | 移到子**节点** |
| `go_up()` | 自身引用 | **仅 `parent_aware`** |
| `get_left()` / `get_right()` | 新节点代理 | |
| `get_parent()` | 新节点代理 | **仅 `parent_aware`** |
| `get_children()` | `pair<edge,edge>` | ⚠️ 返回的是**边**，不是节点代理 |
| `operator==` | `bool` | 比较节点指针 |
| `operator edge_proxy()` | | **仅 `parent_aware`**：由父指针重建出边 |

⚠️ `binary_tree::node_proxy` **没有** `operator edge_proxy()`。想从节点回到"指向它的边"是做不到的（无父指针），只能用 `get_children()` 拿到子边。

### `std::hash` 特化

8 个特化（每个头文件 4 个），定义在全局作用域的 `namespace std` 中。

| 代理 | 哈希对象 |
|---|---|
| `binary_tree::edge_proxy` / `edge_const_proxy` | `std::addressof(*para.ptr)`——槽位地址；`!valid()` 时返回 `0` |
| `binary_tree::node_proxy` / `node_const_proxy` | `p.ptr`——节点地址 |
| `parent_aware::edge_proxy` / `edge_const_proxy` | `std::addressof(para.child())`——槽位地址；`!valid()` 时返回 `0` |
| `parent_aware::node_proxy` / `node_const_proxy` | `para.node`——节点地址 |

哈希与 `operator==` 保持一致：相等（同槽位 / 同节点）的代理必得相同哈希。因此 8 种代理都可以直接作为 `std::unordered_set`、`std::unordered_map` 的键，无效边（默认构造的代理）同样可以安全地参与比较与哈希。

> 注：节点 `left` 位于偏移 0，所以**左子边**的槽位地址恰好等于该节点自身地址，于是 `hash(左子边) == hash(该节点的 node_proxy)`。二者是不同类型，各自独立使用即可；只是直接比对哈希值时要意识到这种数值巧合。
