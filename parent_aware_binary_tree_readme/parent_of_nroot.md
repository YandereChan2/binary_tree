# `Yc::parent_aware_binary_tree<T,Allocator>::parent_of_nroot`, # `Yc::parent_aware_binary_tree<T,Allocator>::parent_of_cnroot`

```C++
node_proxy parent_of_nroot()noexcept; //(1)
```

```C++
node_const_proxy parent_of_nroot()const noexcept; //(2)
```

```C++
node_const_proxy parent_of_cnroot()const noexcept; //(3)
```

返回指代 `parent_aware_binary_tree` 对象内部所保有的 _虚拟二叉树_ 的代理对象。这种代理对象一般起哨位作用。特别的，从最左边的节点进行中序遍历的时候，这个节点可以作为遍历的尾后节点。

这种节点不能向右或向上移动，也不能解引用。

## 参数

（无）

## 返回值

指代 `parent_aware_binary_tree` 对象内部所保有的 _虚拟二叉树_ 的代理对象

## 复杂度

常数

## 异常

（无）
