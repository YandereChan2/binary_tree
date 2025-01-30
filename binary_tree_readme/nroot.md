# `Yc::binary_tree<T,Allocator>::nroot`, # `Yc::binary_tree<T,Allocator>::cnroot`

```C++
node_proxy nroot()noexcept; //(1)
```

```C++
node_const_proxy nroot()const noexcept; //(2)
```

```C++
node_const_proxy cnroot()const noexcept; //(3)
```

返回指代 `binary_tree` 对象内部所保有的 _树位置_ 上 _二叉树_ 的代理对象。如果该 _树位置_ 暂时没有 _二叉树_ ，那么返回不指代 _二叉树_ 的代理对象。

## 参数

（无）

## 返回值

指代 `binary_tree` 对象内部所保有的 _树位置_ 上 _二叉树_ 的代理对象

## 复杂度

常数

## 异常

（无）
