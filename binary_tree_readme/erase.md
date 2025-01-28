# Yc::binary_tree<T, Allocator>::erase

```C++
void erase(edge_const_proxy p)noexcept
```

如果存在，则销毁位于 `p` 所指代的 _树位置_ 上的 _二叉树_ 。调用后，`p` 所指代的 _树位置_ 依然有效，但是其上不再存在 _二叉树_ 。换言之， `p.null() == true` 。

如果 `p` 不从 [`root()`](root.md) 可达，或者 `p` 不指代合法的 _树位置_ ，行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

## 参数

||||
|-:|-|:-|
|`p`|-|指代要移除的 _二叉树_ 的 _树位置_ |

## 返回值

（无）

## 复杂度

与所有由 `p` 可达的 _二叉树_ 的数量成线性关系。

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
void print_sub_tree(H root,F cg)
{
    if (root)
    {
        std::cout << '{'<< *root;
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
                                        {{},{},2},           {{tree+3},{tree+4},3},
                        //                                   |
                        //                          -----------------
                        //                          |                |
                                                    { {},{},4 },    {{},{},5}
        };
        Yc::binary_tree<int> b{&simple_binary_tree_node::val,&simple_binary_tree_node::children,&tree[0]};
        print_sub_tree(b.root(), cg);
        auto [l, r] = b.root().get_children();
        std::cout << '\n';
        //切掉根节点的右子节点
        b.erase(r);
        std::cout << r.null() << '\n';
        print_sub_tree(b.root(), cg);
        b.erase(r);//没有问题
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
================================================================

```
