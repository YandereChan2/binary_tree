// ===========================================================================
// Yc.intrusive_binary_tree.cppm
//
// 具名模块接口：导出 intrusive_binary_tree.h 的全部声明。
//
// 同一个头文件有三种用法，三者在同一个工程里应保持一致（不要在同一程序里
// 既 #include 又 import 同一个实体）：
//   * #include "intrusive_binary_tree.h"
//   * import "intrusive_binary_tree.h";        （标题单元）
//   * import Yc.intrusive_binary_tree;          （本文件）
//
// 构建（MSVC）：
//   cl /std:c++latest /EHsc /utf-8 /c /interface /TpYc.intrusive_binary_tree.cppm
//      /ifcOutput Yc.intrusive_binary_tree.ifc
// ===========================================================================
module;

// 标准库头文件放在全局模块片段：模块接口单元里的 std 声明属于全局模块，
// 不会被打包进本模块，也不会被 export 出去。
#include <cstddef>
#include <functional>
#include <new>
#include <memory>
#include <type_traits>
#include <utility>

export module Yc.intrusive_binary_tree;

// clang 的模块可见性缺陷让 GMF 里的 <new> 对导入方不可见，而 libc++ 的
// allocator_traits::construct 在实例化点查找 placement new。导出全局
// operator new/delete 的 using 声明，import 方就不必自己包含 <new>。
export using ::operator new;
export using ::operator delete;

export
{
#include "intrusive_binary_tree.h"
}
