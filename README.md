非侵入式的RAII风格的泛型二叉树实现，旨在通过合理的封装实现二叉树子树操作无需内存管理，且不泄露。

# 目标

- 和C++标准库保持风格一致
- 零开销原则
- 支持分配器语义
- head-only

# 非目标

- 这不是一个序列容器库，因此不提供迭代器功能，不假定遍历顺序
- 这不是平衡树或者二叉搜索树的公共基座，是普通的二叉树操作库
- 当前要求C++版本是C++23，可能考虑前移到C++20，但是不会进一步向前兼容。

# 主要代码架构

主要发布件为`binary_tree.h`和`parent_aware_binary_tree.h`，这两个头文件依赖`union_data.h`，将文件直接放在头文件目录即可使用，无需配置构建选项。两个库对应设施的文档可以在binary_tree_readme和parent_aware_binary_tree_readme目录下查阅。

另外，为了验证二叉树库的封装能力，本仓库实现了基于红黑树、AVL、SBT、FHQ treap四种平衡树搭建的旨在提供和`std::set`同样接口的类模板作为POC项目。如果有兴趣继续完善或者进行测试可以参考下面的包含关系进行引入。

`rb_set.h`,`avl_set.h`,`sbt_set.h`,`fhq_treap_set.h`为最终头文件，也是include的目标，它们依赖三叉链法二叉树基座库`parent_aware_binary_tree.h`,公共迭代器实现`set_common.h`以及分配器元信息注入机制`cookie_allocator.h`。这些头文件都是自包含的。

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
