# `Yc::swap(Yc::parent_aware_binary_tree)`

```C++
template<class T, class Alloc>
void swap(binary_tree<T, Alloc>& l, binary_tree<T, Alloc>& r)noexcept;
```

执行 [`l.swap(r)`](swap.md) 。

此函数适用于[实参依赖查找(ADL)](https://zh.cppreference.com/w/cpp/language/adl)。

## 参数

||||
|-:|-|:-|
|`l` , `r`|-|要交换内容的 `binary_tree` |

## 返回值

（无）

## 复杂度

常数。

## 异常

（无）
