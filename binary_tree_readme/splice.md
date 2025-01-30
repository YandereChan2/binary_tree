# `Yc::binary_tree<T,Allocator>::splice`

```C++
binary_tree splice(edge_const_proxy to, edge_const_proxy from)noexcept; //(1)
```

```C++
binary_tree splice(edge_const_proxy to, binary_tree& from)noexcept; //(2)
```

```C++
binary_tree splice(edge_const_proxy to, binary_tree&& from)noexcept; //(3)
```

将来自 `from` 的 _二叉树_ 转移至 `to` 所指代的 _树位置_ 。使用返回值将 `to` 处原先的 _二叉树_ 转移至外部。

- (1) 转移的是处于 `from` 所指代的 _树位置_ 的 _二叉树_ 。
- (2,3) 转移的是 `from` 内部 _树位置_ 上的 _二叉树_ 。

如果 `from` 不能提供 _二叉树_ ，那么操作后 `to` 处将没有 _二叉树_ 。

如果 `to` 不从 `root()` 可达，或者作为 _二叉树_ 来源的 `binary_tree` 对象的分配器不和 `get_allocator()` 比较相等，或者 `to` 从 `from` / `from.root()` 可达，那么行为未定义。

任何既有代理对象都不会失效。

## 参数

||||
|-:|-|:-|
|`to`|-|转移的目标 _树位置_ 。|
|`from`|-|提供 _二叉树_ 的来源。|

## 返回值

原先该 _树位置_ 上的 _二叉树_ 。如果原先该 _树位置_ 无 _二叉树_ ，那么返回一个空的 `binary_tree` 对象。

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
        //切出根节点的右子节点
        auto rb = b.splice(r, l); //左节点转移到右节点，右节点转移到外面
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
{1{2  }{3{4  }{5  }}}
{1 {2  }}
{3{4  }{5  }}
================================================================

```
