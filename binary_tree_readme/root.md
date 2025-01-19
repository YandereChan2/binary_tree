# `Yc::binary_tree<T,Allocator>::root`, # `Yc::binary_tree<T,Allocator>::croot`

```C++
edge_proxy root()noexcept; //(1)
```

```C++
edge_const_proxy root()const noexcept; //(2)
```

```C++
edge_const_proxy croot()const noexcept; //(3)
```

返回指代 `binary_tree` 对象内部所保有的 _树位置_ 的代理对象。该 _树位置_ 的生存期和 `binary_tree` 对象一致。

## 参数

（无）

## 返回值

指代 `binary_tree` 对象内部所保有的 _树位置_ 的代理对象

## 复杂度

常数

## 异常

（无）
