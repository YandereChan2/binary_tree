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

- `intrusive_binary_tree.h`：侵入式二叉树，只维护链接和结构操作，不拥有节点、不分配内存；公开的结构操作只使用边代理和节点代理。
- `binary_tree.h` / `parent_aware_binary_tree.h`：非侵入式适配器，内部持有一棵侵入式树，负责节点分配、`T` 的构造/析构、拷贝/移动和分配器传播。

非侵入式适配器只暴露自己的代理，不把侵入式代理作为公开接口。四类代理分别是：

- `edge_proxy` / `edge_const_proxy`
- `node_proxy` / `node_const_proxy`

非侵入式树提供 `root` / `croot`、`nroot` / `cnroot`，以及 `cut`、`splice`、`emplace`、`insert`、`erase`、`clear`、旋转、`swap_node`、`for_each` 和 `recur_and_write`；`parent_aware_binary_tree` 还提供 `parent_of_nroot` / `parent_of_cnroot`。

主要发布件：

- `binary_tree.h`
- `parent_aware_binary_tree.h`
- `intrusive_binary_tree.h`
- `union_data.h`

另提供 3 个 C++ 模块接口单元：`Yc.intrusive_binary_tree.cppm`、`Yc.binary_tree.cppm`、`Yc.parent_aware_binary_tree.cppm`，见下文“C++ 模块支持”。

把文件直接放在头文件目录即可使用，无需配置构建选项。文档分别在 `binary_tree_readme`、`parent_aware_binary_tree_readme` 和 `intrusive_binary_tree_readme` 目录下。

为了验证二叉树库的封装能力，本仓库还实现了基于红黑树、AVL、WAVL、LLRB、SBT、FHQ treap 等平衡树搭建的、接口接近 `std::set` 的类模板：

- `rb_set.h`
- `avl_set.h`
- `wavl_set.h`
- `llrb_set.h`
- `sbt_set.h`
- `fhq_treap_set.h`

这些 POC 头文件依赖 `set_common.h` 和 `cookie_allocator.h`。其中 `rb_set.h` / `avl_set.h` 通过 `import "parent_aware_binary_tree.h";`（标题单元）使用树类型，它们仍是普通头文件，不是模块接口单元；其余 POC 头文件仍直接包含 `parent_aware_binary_tree.h`。

# 测试与构建

CMake 提供：

- `test_runner`：GoogleTest 单元测试
- `test_intrusive_binary_tree`：侵入式二叉树独立测试

MSVC 下为这些目标启用 `/utf-8`；可在构建目录运行 `ctest`。

# C++ 模块支持

`intrusive_binary_tree.h`、`binary_tree.h`、`parent_aware_binary_tree.h` 三种用法都支持：

| 用法 | 写法 |
|-|-|
| 传统头文件 | `#include "binary_tree.h"` |
| 标题单元（header unit） | `import "binary_tree.h";` |
| 具名模块 | `import Yc.binary_tree;` |

具名模块的接口单元在仓库根目录：

| 模块 | 导出的头文件 | 接口单元 |
|-|-|-|
| `Yc.intrusive_binary_tree` | `intrusive_binary_tree.h` | `Yc.intrusive_binary_tree.cppm` |
| `Yc.binary_tree` | `binary_tree.h` | `Yc.binary_tree.cppm` |
| `Yc.parent_aware_binary_tree` | `parent_aware_binary_tree.h` | `Yc.parent_aware_binary_tree.cppm` |

头文件本身不含任何 module 声明，所以 `#include` 和标题单元的用法完全不变。模块接口单元把标准库头文件放进全局模块片段（global module fragment），再用 `export { #include "…" }` 把整个头文件导出。`binary_tree` / `parent_aware_binary_tree` 的侵入式层由 `import Yc.intrusive_binary_tree;` 提供；接口单元先 `import` 该模块，再定义 `YC_BINARY_TREE_MODULE` / `YC_PARENT_AWARE_BINARY_TREE_MODULE`，头文件看到该宏就跳过自己的两个 `#include`，保证每个实体只属于一个模块。`union_data.h`（`data_t` 是节点存储的实现细节）只被放进两个树模块接口单元的全局模块片段，既不导出也不单独成模块。

`rb_set.h` / `avl_set.h` 通过 `import "parent_aware_binary_tree.h";` 使用标题单元（使用前需要先为该头文件生成 CMI/IFC）；它们仍是普通头文件，不是模块接口单元。

## 构建与使用

```bat
:: 编译模块接口单元，得到 .ifc 和 .obj（/interface 声明这是接口单元，/Tp 指定按 C++ 源文件处理）
cl /std:c++latest /EHsc /utf-8 /c /interface /TpYc.intrusive_binary_tree.cppm /ifcOutput Yc.intrusive_binary_tree.ifc
cl /std:c++latest /EHsc /utf-8 /c /interface /TpYc.binary_tree.cppm /ifcSearchDir . /ifcOutput Yc.binary_tree.ifc
cl /std:c++latest /EHsc /utf-8 /c /interface /TpYc.parent_aware_binary_tree.cppm /ifcSearchDir . /ifcOutput Yc.parent_aware_binary_tree.ifc

:: 编译消费方；/ifcSearchDir 让编译器按模块名找到全部 .ifc
cl /std:c++latest /EHsc /utf-8 main.cpp /ifcSearchDir . ^
   Yc.intrusive_binary_tree.obj Yc.binary_tree.obj Yc.parent_aware_binary_tree.obj
```

```C++
#include <vector>   // MSVC 下标准库 #include 要放在 import 之前，见下面注意事项

import Yc.intrusive_binary_tree;
import Yc.binary_tree;
import Yc.parent_aware_binary_tree;

int main()
{
    Yc::binary_tree<int> t;
    t.insert(t.root(), 1);
    t.insert(t.root().get_left(), 2);
    std::vector<int> v;
    t.for_each([&](int& x) { v.push_back(x); });
}
```

标题单元用 `import "binary_tree.h";`，需要编译器先为该头文件生成 IFC。MSVC 的生成 / 使用方式：

```bat
cl /std:c++latest /EHsc /utf-8 /c /exportHeader binary_tree.h /ifcOutput binary_tree.h.ifc
cl /std:c++latest /EHsc /utf-8 main.cpp /headerUnit "binary_tree.h=binary_tree.h.ifc"
```

## 注意事项

- 同一程序里不要混用 `#include` 与 `import` 同一个实体：模块里的实体和全局模块里的实体不是同一个实体，混用属于 IFNDR。
- MSVC 下若某个 TU 既 `import` 具名模块，又在 `import` 之后包含该模块全局模块片段里已经包含过的标准库头文件，会出现 std 重定义（C2011 / C2572 等）。把标准库 `#include` 全部放到 `import` 之前即可；标题单元方式不受此限制。
- clang 的合并式模块构建（`-fmodule-output=`）对 placement new 有可见性缺陷（llvm/llvm-project#218246）：模块 GMF 里的 `<new>` 对导入方不可见，libc++ 消费方实例化 `allocator_traits::construct` 时会要求包含 `<new>`。三个模块接口导出全局 `operator new` / `operator delete` 的 using 声明来规避，`import` 方无需再手动包含 `<new>`；MSVC STL / libstdc++ 不受影响。
- 链接时要把各模块接口单元生成的 `.obj` 一起链接。

# 性能

可以运行 `benchmark_perf.cpp` 对比 `Yc::rb_set` 与 `std::set`。`Ratio = std::set 时间 / rb_set 时间`，大于 1 表示 `rb_set` 更快。

`benchmark_perf.cpp` 通过 `rb_set.h` 使用 `parent_aware_binary_tree.h` 的标题单元，编译前先生成该头文件的 CMI/IFC。

```bat
:: MSVC
cl /std:c++latest /EHsc /utf-8 /c /exportHeader parent_aware_binary_tree.h /ifcOutput parent_aware_binary_tree.h.ifc
cl /std:c++latest /EHsc /utf-8 /O2 benchmark_perf.cpp /headerUnit "parent_aware_binary_tree.h=parent_aware_binary_tree.h.ifc"
```

```bat
:: GCC（-fmodules 在生成 CMI 和编译消费方时都要加）
g++ -std=c++23 -fmodules -x c++-header -c parent_aware_binary_tree.h
g++ -std=c++23 -fmodules -O2 benchmark_perf.cpp -o benchmark_perf
```

测试参数为 `N = 500,000`、50 轮平均；具体数值会随编译器、标准库实现、机器和数据分布变化。

| 测试 | MSVC Ratio | GCC Ratio |
|-:|-:|-:|
| insert_random | 0.98x | 0.86x |
| insert_sorted | 0.74x | 1.12x |
| find | 1.00x | 0.97x |
| erase | 2.43x | 0.97x |
| iteration | 0.69x | 0.96x |
| mixed | 1.11x | 0.93x |

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
