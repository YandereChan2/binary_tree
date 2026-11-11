非侵入式、RAII 风格的泛型二叉树实现。外部只通过 `edge_proxy` / `node_proxy` 及其 const 版本操作树；节点内存和 `T` 的构造/析构由非侵入式容器管理。

# 目标

- 和 C++ 标准库保持风格一致
- 零开销原则
- 支持分配器语义
- header-only

# 非目标

- 这不是序列容器库，不提供迭代器，也不承诺遍历顺序
- 这不是平衡树或二叉搜索树的公共基座，而是普通二叉树操作库
- 当前要求 C++23

# 主要代码架构

结构分成两层：

- `intrusive_binary_tree.h`：侵入式二叉树，只维护链接和结构操作，不拥有节点、不分配内存；公开接口只有边代理和节点代理。
- `binary_tree.h` / `parent_aware_binary_tree.h`：非侵入式适配器，内部持有一棵侵入式树，负责节点分配、`T` 的构造/析构、拷贝/移动和分配器传播。

非侵入式适配器只暴露自己的代理，不把侵入式代理作为公开接口。四类代理分别是：

- `edge_proxy` / `edge_const_proxy`
- `node_proxy` / `node_const_proxy`

非侵入式树提供 `root` / `croot`、`nroot` / `cnroot`、`parent_of_nroot` / `parent_of_cnroot`，以及 `cut`、`splice`、`emplace`、`insert`、`erase`、`clear`、旋转、`swap_node`、`for_each` 和 `recur_and_write`。

主要发布件：

- `binary_tree.h`
- `parent_aware_binary_tree.h`
- `intrusive_binary_tree.h`
- `union_data.h`

把文件直接放在头文件目录即可使用，无需配置构建选项。文档分别在 `binary_tree_readme`、`parent_aware_binary_tree_readme` 和 `intrusive_binary_tree_readme` 目录下。

为了验证二叉树库的封装能力，本仓库还实现了基于红黑树、AVL、WAVL、LLRB、SBT、FHQ treap 等平衡树搭建的、接口接近 `std::set` 的类模板：

- `rb_set.h`
- `avl_set.h`
- `wavl_set.h`
- `llrb_set.h`
- `sbt_set.h`
- `fhq_treap_set.h`

这些 POC 头文件依赖 `parent_aware_binary_tree.h`、`set_common.h` 和 `cookie_allocator.h`，且都是自包含的。

# 测试与构建

CMake 提供：

- `test_runner`：GoogleTest 单元测试
- `test_intrusive_binary_tree`：侵入式二叉树独立测试

MSVC 下为这些目标启用 `/utf-8`；可在构建目录运行 `ctest`。

# 性能

可以运行 `benchmark_perf.cpp` 对比 `Yc::rb_set` 与 `std::set`。`Ratio = std::set 时间 / rb_set 时间`，大于 1 表示 `rb_set` 更快。

当前测试条件：

- MSVC 14.44 / GCC 16.1，`-O2`
- `N = 500,000`
- 50 轮平均

| 测试 | MSVC Ratio | GCC Ratio | MSVC rb(ms) | GCC rb(ms) |
|-:|-:|-:|-:|-:|
| insert_random | 0.94x | 0.82x | 206.9 | 213.7 |
| insert_sorted | 0.85x | 1.00x | 51.5 | 42.2 |
| find | 0.95x | 0.96x | 245.8 | 292.6 |
| erase | 1.26x | 1.04x | 124.6 | 133.4 |
| iteration | 0.68x | 0.96x | 59.1 | 60.6 |
| mixed | 1.03x | 0.92x | 106.3 | 107.4 |

性能会随编译器、标准库实现和数据分布变化，以上仅是本机实测结果。

# 快速开始

```C++
#include <print>
#include "binary_tree.h"

struct simple_binary_tree_node
{
    simple_binary_tree_node* left{};
    simple_binary_tree_node* right{};
    int val;
    std::pair<simple_binary_tree_node*, simple_binary_tree_node*> children()const noexcept
    {
        return { left,right };
    }
};

int main()
{
    simple_binary_tree_node tree[]{
                           {tree + 1,tree + 2,1},
    //                      |
    //              ---------------------
    //              |                   |
                   {{},{},2},          {{tree + 3},{tree + 4},3},
    //                                  |
    //                          -----------------
    //                          |               |
                               { {},{},4 },    {{},{},5}
    };
    Yc::binary_tree<int> b{ &simple_binary_tree_node::val,&simple_binary_tree_node::children,&tree[0] };
    Yc::binary_tree<int>::edge_proxy p = b.root();
    p.go_right().go_left();
    std::print("b的右孩子的左孩子是{}", *p);
}
```

输出

```
b的右孩子的左孩子是4
```
