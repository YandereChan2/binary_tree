# Yc::binary_tree<T, Allocator>::empty

```C++
bool empty()const noexcept
```

返回 `binary_tree` 对象内部 _树位置_ 上是否没有 _二叉树_ 。如同于 `root().null()` 。

## 参数

（无）

## 返回值

如果没有，则为 `true`

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
                        //                          |               |
                                                    { {},{},4 },    {{},{},5}
        };
        Yc::binary_tree<int> b{&simple_binary_tree_node::val,&simple_binary_tree_node::children,&tree[0]};
        print_sub_tree(b.root(), cg);
        std::cout << '\n' << b.empty();
    }
    std::cout << "\n================================================================\n";
    {
        Yc::binary_tree<int> b{};
        print_sub_tree(b.root(), cg);
        std::cout << '\n' << b.empty();
    }
    std::cout << "\n================================================================\n";
}
```

### 输出

```plain text
================================================================
{1{2}{3{4}{5}}}
0
================================================================

1
================================================================

```
