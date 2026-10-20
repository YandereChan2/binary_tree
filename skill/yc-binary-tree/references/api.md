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
| `pointer` | `std::allocator_traits<Alloc>::pointer` |
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
| `static void swap_node(edge_const_proxy l, edge_const_proxy r) noexcept` | 交换两个节点：净效果是**两个位置上的值互换、子树留在原位**；父子相邻不需要判断（先换槽位、再成对换孩子槽位，自指会被自动解掉） |

### 遍历

| 签名 | 说明 |
|---|---|
| `template<class F> F for_each(F f) noexcept(is_nothrow_invocable_v<F&, T&> && is_nothrow_move_constructible_v<F>)` | 以未指定的顺序遍历，回调收到 `T&`；按值接收并返回函数对象（同 `std::for_each`） |
| `template<class F> F for_each(F f) const noexcept(is_nothrow_invocable_v<F&, const T&> && is_nothrow_move_constructible_v<F>)` | 以未指定的顺序遍历，回调收到 `const T&`；同上 |

两者都按值接收 `f`、并把 `f` 按值返回（与 `std::for_each` 一致），所以有状态函数对象能带出累积结果：`auto acc = t.for_each(accumulator{});`。
遍历顺序**未指定**，调用方不应依赖它；当前实现是"左 - 本 - 右"的中序，但这只是实现现状，不承诺保持不变。
`noexcept` 是条件性的、取决于 `f`：`f` 能无异常接受对应左值**且**能无异常移动构造时整次遍历即 `noexcept`，否则 `f` 抛出的异常向调用方传播。因此把 `noexcept` lambda 传进来即可在 `noexcept` 上下文（如 `tree_hash_set::clear()`）中安全调用（lambda 的移动构造总是 `noexcept`）。

供容器在销毁节点前做收尾（如哈希表先把所有桶摘链，再 `clear()` 销毁桶节点）。`f` 不得改变树结构。

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
- 两个子句柄的写入顺序未指定
- ⚠️ `p` 处**必须为空**，否则 `std::unreachable()`

### 自由函数与函数对象

| 名称 | 说明 |
|---|---|
| `void swap(binary_tree<T,Alloc>& l, binary_tree<T,Alloc>& r) noexcept` | 转发到成员 `swap` |
| `Yc::binary_tree_functional::get_children` | `constexpr inline` 对象，可直接作 `CG`（对 4 种代理均可调用） |

---

## `Yc::parent_aware_binary_tree<T, Alloc = std::allocator<T>>`

`#include "parent_aware_binary_tree.h"`

上表中除下列差异外**全部相同**（`swap_node` / `for_each` 两个实现都有，签名与语义一致）：

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

## 关于 `swap_node` 的父子相邻

`swap_node` 只有"先换槽位、再成对交换两边孩子槽位"三步，**没有**相邻形态的专门分支，也不需要调用方给提示。父节点里那条指向孩子的槽位在第一步后会短暂自指，第三步正好把它解成"父节点上位、孩子落回原位"；四种相邻形态与不相邻形态结果一致（已用所有非空边两两组合的穷举测试覆盖）。把这两步的顺序反过来才会互指成环、丢节点。净语义是"两个位置上的值互换、子树留在原位"。

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
| `binary_tree::edge_proxy` / `edge_const_proxy` | 槽位指针；失效时为 `0` |
| `binary_tree::node_proxy` / `node_const_proxy` | 节点指针 |
| `parent_aware::edge_proxy` / `edge_const_proxy` | 槽位地址；失效时为 `0` |
| `parent_aware::node_proxy` / `node_const_proxy` | 节点指针 |

这 8 个特化要求 `std::hash<std::allocator_traits<Alloc>::pointer>` 可用。`Alloc::pointer` 是自定义 [_花式指针 (fancy pointer)_](https://zh.cppreference.com/w/cpp/named_req/Allocator#.E8.8A.B1.E5.BC.8F.E6.8C.87.E9.92.88) 时，需要由用户为该类型提供 `std::hash` 特化。

哈希与 `operator==` 保持一致：相等（同槽位 / 同节点）的代理必得相同哈希。因此 8 种代理都可以直接作为 `std::unordered_set`、`std::unordered_map` 的键，无效边（默认构造的代理）同样可以安全地参与比较与哈希。

> 注意：**左子边**与其所指代的 _二叉树_ 的 `node_proxy` 虽然指代同一个对象，但属于不同的特化，哈希值不作保证相同。判断它们指代同一对象请使用 `operator==` 。
