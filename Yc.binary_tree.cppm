// ===========================================================================
// Yc.binary_tree.cppm
//
// 具名模块接口：导出 binary_tree.h 的全部声明。
//
// 用法（与 intrusive 模块一致，不要在同一程序里混用 include 与 import）：
//   * #include "binary_tree.h"
//   * import "binary_tree.h";     （标题单元）
//   * import Yc.binary_tree;       （本文件）
//
// 依赖的处理：
//   * intrusive_binary_tree.h 由 import Yc.intrusive_binary_tree; 提供；
//   * union_data.h 只在模块内部使用（data_t 是节点存储的实现细节），直接放在
//     本接口单元的全局模块片段里文本包含，既不导出、也不单独成模块。
//     两个树模块各自包含同一份 union_data.h，得到的是全局模块里同一个 data_t。
// 这里先定义 YC_BINARY_TREE_MODULE，让 binary_tree.h 跳过它自己的两个
// #include，避免 intrusive 的声明在多个模块里各定义一份。
//
// 构建（MSVC）：
//   cl /std:c++latest /EHsc /utf-8 /c /interface /TpYc.binary_tree.cppm
//      /ifcSearchDir . /ifcOutput Yc.binary_tree.ifc
// 其中 /ifcSearchDir 用于找到 import 引入的 Yc.intrusive_binary_tree 的 .ifc。
// ===========================================================================
module;

// 标准库头文件放在全局模块片段，理由同 Yc.intrusive_binary_tree.cppm。
// union_data.h 放在这里：它的声明属于全局模块，不会被 export 出去。
#include <cstddef>
#include <functional>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>
#include "union_data.h"

export module Yc.binary_tree;

import Yc.intrusive_binary_tree;

#define YC_BINARY_TREE_MODULE 1

export
{
#include "binary_tree.h"
}
