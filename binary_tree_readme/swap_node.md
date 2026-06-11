# `Yc::binary_tree<T, Alloc>::swap_node`

```C++
static void swap_node(edge_const_proxy l, edge_const_proxy r)
```

交换两个 _二叉树_ 节点，同时交换它们各自的子树。相当于先调用 [`swap_sub_tree(l, r)`](swap_sub_tree.md) 交换节点位置，再交换左右子节点位置。

如果两个 _树位置_ 的来源 `binary_tree` 的分配器不比较相等，则行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

## 参数

||||
|-:|-|:-|
|`l` , `r`|-|指代要进行交换的两个 _二叉树_ 对应的 _树位置_ |

## 返回值

（无）

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
    else
    {
        std::cout << ' ';
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
        Yc::binary_tree<int>::swap_node(l, r);
        print_sub_tree(b.root(), cg);
    }
    std::cout << "\n================================================================\n";
}
```

### 输出

```plain text
================================================================
{1{2  }{3{4  }{5  }}}
{1{3{2  }{5  }}{4  }}
================================================================

```
