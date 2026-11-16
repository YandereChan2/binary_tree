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

将来自 `from` 的 _二叉树_ 转移至 `to` 所指代的 _树位置_ 。使用返回值将 `to` 处
原先的 _二叉树_ 转移至外部。

- (1) 转移的是处于 `from` 所指代的 _树位置_ 的 _二叉树_ 。
- (2,3) 转移的是 `from` 内部 _树位置_ 上的 _二叉树_；操作后 `from` 变为空树。

如果 `from` 不能提供 _二叉树_ ，那么操作后 `to` 处将没有 _二叉树_ 。

**返回值拥有 `to` 处原有的子树。** 如果保留返回值，既有代理对象和元素引用不会失效；
如果丢弃返回值，临时 `binary_tree` 的析构会销毁这棵原子树，指向其中元素的代理和
引用随之失效。

## 前置条件

- `to` 和 `from` 必须是**合法**的 _树位置_ 。
- `to` 必须从 `*this` 的 [`root()`](root.md) 可达；`from` 必须从它所属的树可达。
- `from` 所属的 `binary_tree` 的 [`get_allocator()`](get_allocator.md) 必须与
  `get_allocator()` 比较相等。
- `to` 不能位于 `from` 的子树中；否则行为[未定义](https://zh.cppreference.com/w/cpp/language/ub)。

## 参数

|||
|-:|:-|
|`to`|转移的目标 _树位置_ 。|
|`from`|提供 _二叉树_ 的来源。|

## 返回值

原先 `to` 处 _二叉树_（由返回的 `binary_tree` 拥有）。如果原先该 _树位置_ 无
_二叉树_ ，那么返回一个空的 `binary_tree` 对象。

## 复杂度

常数。

## 异常

（无）。

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
