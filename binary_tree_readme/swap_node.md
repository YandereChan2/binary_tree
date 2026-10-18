# `Yc::binary_tree<T, Alloc>::swap_node`

```C++
static void swap_node(edge_const_proxy l, edge_const_proxy r)noexcept
```

交换两个 _二叉树_ 节点。实现上是先调用 [`swap_sub_tree(l, r)`](swap_sub_tree.md) 交换两个 _树位置_ 上的节点，再交换这两个节点的左右子节点位置。净效果是**两个 _树位置_ 上的值互换、子树留在原位**（树的形状不变）。

`l` 和 `r` 都必须是指代**存在的节点**的 _树位置_。传入空 _树位置_（`(bool)l == false`，例如对没有左孩子的节点取 `get_left()`）会解引用空节点，行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

两个节点是否互为父子**不需要判断**，也不需要调用方提供任何提示：先交换 _树位置_ 时，父节点里那条指向孩子的 _树位置_ 会短暂地变成自指，紧接着"成对交换孩子 _树位置_"这一步正好把它解成"父节点上位、孩子落回原位"。四种相邻形态（左右各两种顺序）与不相邻形态走的是同一条路径。

`Yc::binary_tree` 的 _二叉树_ 不保有父节点指针，因此交换只涉及左右 _树位置_ 的指向。本函数的接口和语义与 `Yc::parent_aware_binary_tree::swap_node` 一致。

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
{1{3  }{2{4  }{5  }}}
================================================================

```
