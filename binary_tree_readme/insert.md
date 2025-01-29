# `Yc::binary_tree<T,Allocator>::insert`

```C++
binary_tree insert(edge_const_proxy p, const T& value); //(1)
```

```C++
binary_tree insert(edge_const_proxy p, T&& value); //(2)
```

在 `p` 所指代的 _树位置_ 建立一个 _二叉树_ 。

- (1) 使用复制语义插入 `value` 的副本。
- (2) 使用移动语义插入 `value` 的副本。

如果 `p` 所指代的 _树位置_ 原有一个 _二叉树_ ，那么通过返回值将其转移到 `*this` 外部。

任何既有代理对象都不会失效。

## 参数

||||
|-:|-|:-|
|`p`|-|想要建立新 _二叉树_ 的 _树位置_ |
|`value`|-|要复制的对象。|

### 类型要求

- 要使用重载 (1) ， `T` 必须满足 [_可复制插入(CopyInsertable)_](https://zh.cppreference.com/w/cpp/named_req/CopyInsertable) 。
- 要使用重载 (2) ， `T` 必须满足 [_可移动插入(MoveInsertable)_](https://zh.cppreference.com/w/cpp/named_req/CopyInsertable) 。

## 返回值

原先该 _树位置_ 上的 _二叉树_ 。如果原先该 _树位置_ 无 _二叉树_ ，那么返回一个空的 `binary_tree` 对象。

## 复杂度

常数。

## 异常

若抛出了异常（例如由构造函数），则保留 `binary_tree` 不修改，如同未曾调用过此函数（[强异常保证](https://zh.cppreference.com/w/cpp/language/exceptions)）。

如果不能分配内存，则抛出 [`std::bad_alloc`](https://zh.cppreference.com/w/cpp/memory/new/bad_alloc) 。

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
        std::cout << '\n';
        auto p = b.root();
        p.go_right();
        auto sub_tree = b.insert(p, 8); //通过返回值给出原先的子树
        print_sub_tree(b.root(), cg);
        std::cout << '\n';
        print_sub_tree(sub_tree.root(), cg);
    }
    std::cout << "\n================================================================\n";
}
```

### 输出

```plain text
================================================================
{1{2}{3{4}{5}}}
{1{2}{8}}
{3{4}{5}}
================================================================

```
