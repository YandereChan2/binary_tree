# `Yc::binary_tree<T,Allocator>::cut`

```C++
binary_tree emplace(edge_const_proxy p)noexcept;
```

将 `p` 所指代的 _树位置_ 处的 _二叉树_ 通过返回值转移到外部。

## 参数

||||
|-:|-|:-|
|`p`|-|指代要移除的 _二叉树_ 的 _树位置_ |

## 返回值

原先处于该 _树位置_ 的 _二叉树_ 。如果没有，则返回的 `binary_tree` 对象内部的 _树位置_ 上没有 _二叉树_ 。

## 复杂度

常数

## 异常

（无）

## 示例

```C++
#include "binary_tree.h"
#include <iostream>
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
template<class H, class F>
void print_sub_tree(H root, F cg)
{
    if (root)
    {
        std::cout << '{' << *root;
        auto [l, r] = std::invoke(cg, root);
        print_sub_tree(l, cg);
        print_sub_tree(r, cg);
        std::cout << '}';
    }
}
int main()
{
    std::cout << "================================================================\n";
    auto cg = &Yc::binary_tree<int>::edge_proxy::get_children;
    {
        simple_binary_tree_node tree[]{
                                    {tree + 1,tree + 2,1},
            //                      |
            //              ---------------------
            //              |                    |
                            {{},{},2},           {{tree + 3},{tree + 4},3},
            //                                   |
            //                          -----------------
            //                          |                |
                                        { {},{},4 },    {{},{},5}
        };
        Yc::binary_tree<int> b{ &simple_binary_tree_node::val,&simple_binary_tree_node::children,&tree[0] };
        print_sub_tree(b.root(), cg);
        auto [l, r] = b.root().get_children();
        std::cout << '\n';
        //切出根节点的右子节点
        auto rb = b.cut(r);
        std::cout << r.null() << '\n';
        print_sub_tree(b.root(), cg);
        std::cout << '\n';
        print_sub_tree(rb.root(), cg);
    }
    std::cout << "\n================================================================\n";
}
```

### 输出

```plain text
================================================================
{1{2}{3{4}{5}}}
1
{1{2}}
{3{4}{5}}
================================================================

```
