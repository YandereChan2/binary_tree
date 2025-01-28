# Yc::binary_tree<T, Allocator>::clear

```C++
void clear()noexcept
```

销毁 `binary_tree` 对象内部 _树位置_ 上的 _二叉树_ ，会一并销毁该 _二叉树_ 的左右  _树位置_ 上的 _二叉树_ ，如果有。此函数如同于调用 `erase(root())` 。

## 参数

（无）

## 返回值

（无）

## 复杂度

与实际销毁的 _二叉树_ 的数量成正比。

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
                                                    { {},{},4 },     {{},{},5}
        };
        Yc::binary_tree<int> b{&simple_binary_tree_node::val,&simple_binary_tree_node::children,&tree[0]};
        print_sub_tree(b.root(), cg);
        std::cout << '\n';
        b.clear();
        print_sub_tree(b.root(), cg);
    }
    std::cout << "\n================================================================\n";
}
```

```plain text
================================================================
{1{2}{3{4}{5}}}

================================================================

```
