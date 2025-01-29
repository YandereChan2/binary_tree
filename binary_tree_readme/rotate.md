# `Yc::binary_tree<T,Allocator>::left_rotate`,`Yc::binary_tree<T,Allocator>::right_rotate`,`Yc::binary_tree<T,Allocator>::checked_left_rotate`,`Yc::binary_tree<T,Allocator>::checked_right_rotate`

```C++
void left_rotate(edge_const_proxy p)noexcept //(1)
```

```C++
void right_rotate(edge_const_proxy p)noexcept //(2)
```

```C++
bool checked_left_rotate(edge_const_proxy p)noexcept //(3)
```

```C++
bool checked_right_rotate(edge_const_proxy p)noexcept //(4)
```

- (1,2) 无条件在 `p` 所指代的 _树位置_ 执行旋转操作。
- (3,4) 尝试进行旋转，如果旋转的先决条件不满足则返回 `false` ，否则返回 `true`。

```plain text
     |                         |
    <A>                       <C>
   /   \                      / \
  <B>  <C>     =====>       <A> <E>
       / \                  / \
     <D> <E>              <B> <D>
```

旋转操作的语义如图所示（图中展示的是左旋）。旋转的先决条件指的是旋转前， A 所在的 _树位置_ 和 C 所在的 _树位置_ 必须有 _二叉树_ 。

在不能旋转的地方进行旋转会导致[未定义行为](https://zh.cppreference.com/w/cpp/language/ub)。

## 参数

||||
|-:|-|:-|
|`p`|-|想要进行旋转的 _树位置_ 。|

## 返回值

- (1,2) （无）
- (3,4) 旋转能否进行。

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
        std::cout << '\n';
        b.left_rotate(b.root());
        print_sub_tree(b.root(), cg);
    }
    std::cout << "\n================================================================\n";
}
```

### 输出

```plain text
================================================================
{1{2}{3{4}{5}}}
{3{1{2}{4}}{5}}
================================================================

```
