# `Yc::binary_tree<T,Allocator>::binary_tree`

```C++
binary_tree(); //(1)
```

```C++
explicit binary_tree(const Alloc& a); //(2)
```

```C++
binary_tree(const binary_tree& b); //(3)
```

```C++
binary_tree(binary_tree&& b)noexcept; //(4)
```

```C++
template<
    class ValueGetter,
    class ChildrenGetter,
    class InitializeHandle
>
binary_tree(
    ValueGetter vg,
    ChildrenGetter cg,
    InitializeHandle h,
    const Alloc& a = {}
); //(5)
```

从各种数据源构造新 `binary_tree` ，可以使用用户提供的分配器 `alloc`。

- (1) 默认的构造函数。
- (2) 构造使用分配器 `a` 的 `binary_tree` 。
- (3) 复制构造函数，会递归的复制所有子树。
- (4) 移动构造函数，会转移原先的 `binary_tree` 保有的 _二叉树_ 。
- (5) 相当于在构造后使用类似 [`recur_and_write`](recur_and_write.md) 创建 _二叉树_ 。

## 参数

||||
|-:|-|:-|
|`a`|-|使用的分配器|
|`b`|-|想要复制或移动的对象|
|`vg`|-|获取资源值的函数对象。|
|`cg`|-|获取关联子资源的函数对象|
|`h`|-|起始资源句柄|

## 复杂度

- (1,2,4) 常数。
- (3,5) 与创建的 _二叉树_ 数量成正比。

## 异常

如果不能分配内存，则抛出 [`std::bad_alloc`](https://zh.cppreference.com/w/cpp/memory/new/bad_alloc) 。

构建元素或者取得句柄等动作可能产生别的异常。

## 示例

下面这个例子演示了使用复制、移动、递归三种方法构造二叉树。

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
        //              |                   |
                        {{},{},2},          {{tree+3},{tree+4},3},
        //              |
        //      -----------------
        //      |               |
                { {},{},4 },    {{},{},5}
        };
        Yc::binary_tree<int> b{&simple_binary_tree_node::val,&simple_binary_tree_node::children,&tree[0]};
        print_sub_tree(b.root(), cg);
    }
    std::cout << "\n================================================================\n";
    {
        simple_binary_tree_node tree[]{
                                {tree + 1,tree + 2,1},
        //                      |
        //              ---------------------
        //              |                   |
                        {{},{},2},          {{tree+3},{tree+4},3},
        //              |
        //      -----------------
        //      |               |
                { {},{},4 },    {{},{},5}
        };
        Yc::binary_tree<int> b{ &simple_binary_tree_node::val,&simple_binary_tree_node::children,&tree[0] };
        Yc::binary_tree<int> cp{ b };
        print_sub_tree(b.root(), cg);
        std::cout << '\n';
        print_sub_tree(cp.root(), cg);
    }
    std::cout << "\n================================================================\n";
    {
        simple_binary_tree_node tree[]{
                                {tree + 1,tree + 2,1},
        //                      |
        //              ---------------------
        //              |                   |
                        {{},{},2},          {{tree+3},{tree+4},3},
        //              |
        //      -----------------
        //      |               |
                { {},{},4 },    {{},{},5}
        };
        Yc::binary_tree<int> b{ &simple_binary_tree_node::val,&simple_binary_tree_node::children,&tree[0] };
        Yc::binary_tree<int> cp{ std::move(b) };
        print_sub_tree(b.root(), cg);
        std::cout << '\n';
        print_sub_tree(cp.root(), cg);
    }
    std::cout << "\n================================================================\n";
}
```

### 输出

```plain text
================================================================
{1{2}{3{4}{5}}}
================================================================
{1{2}{3{4}{5}}}
{1{2}{3{4}{5}}}
================================================================

{1{2}{3{4}{5}}}
================================================================

```
