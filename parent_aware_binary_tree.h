// 带父节点指针的二叉树库
// 核心抽象：
//   - edge_proxy / edge_const_proxy：代表父节点到子节点的"指针槽位"，
//     同时支持导航（go_left/go_right/go_up）和修改（emplace/erase 等）
//   - node_proxy / node_const_proxy：直接代表树节点
//   - 内部使用一个哨兵根节点（root_node，只使用 left 指向实际树根），
//     使根节点也有 parent，统一处理边界情况
// 支持：分配器感知、复制/移动语义、递归构造、旋转、切割/拼接等操作
#pragma once
#include <memory>
#include <type_traits>
#include <functional>
#include <utility>
#include "union_data.h"

namespace Yc
{
    template<class T, class Alloc>
    class parent_aware_binary_tree;
    namespace details
    {
        // ===================================================================
        // 节点层次结构（Nested Base Classes）：
        //   parent_aware_binary_tree_empty_placeholder       （空类型，用于类型擦除）
        //   └─ parent_aware_binary_tree_node_base1<Alloc>    （left 指针）
        //       └─ parent_aware_binary_tree_node_base<Alloc> （right、parent 指针）
        //           └─ parent_aware_binary_tree_node<T,Alloc>（存储用户数据 data_t<T>）
        //
        // 设计意图：通过继承链分离指针成员，使得 edge_proxy 可以安全地进行
        //          指针到成员的偏移转换，同时利用空基类优化减少内存开销。
        //          分配器绑定在最顶层占位类型，统一管理所有节点的内存。
        // ===================================================================

        // 最底层基类：空类型占位符，用于统一 allocator rebound 的目标类型
        struct parent_aware_binary_tree_empty_placeholder
        {
        };
        // 节点基类第一层：持有 left 子节点指针
        template<class Alloc>
        struct parent_aware_binary_tree_node_base1: parent_aware_binary_tree_empty_placeholder
        {
            static_assert(std::is_same_v<typename std::allocator_traits<Alloc>::value_type,
                parent_aware_binary_tree_empty_placeholder>);
            using pointer = std::allocator_traits<Alloc>::pointer;
            pointer left{};
        };

        // 节点基类第二层：新增 right 子节点指针和 parent 父节点指针
        template<class Alloc>
        struct parent_aware_binary_tree_node_base : parent_aware_binary_tree_node_base1<Alloc>
        {
            using pointer = std::allocator_traits<Alloc>::pointer;
            pointer right{};
            pointer parent{};
        };

        // 完整节点：继承全部指针并附加用户数据
        template<class T, class Alloc>
        struct parent_aware_binary_tree_node : parent_aware_binary_tree_node_base<Alloc>
        {
            data_t<T> data;
        };
        
        template<class T, class Alloc>
        class parent_aware_binary_tree_edge_const_proxy;
        template<class T, class Alloc>
        class parent_aware_binary_tree_node_proxy;
        template<class T, class Alloc>
        class parent_aware_binary_tree_node_const_proxy;

        // ===================================================================
        // edge_proxy：代表父节点到子节点之间的"边"（指针槽位）
        //
        // 核心思想：二叉树中每个非根节点都通过父节点的一个指针成员
        // （left 或 right）被引用。edge_proxy 封装了这个指针槽位，
        // 由 parent（父节点指针）和 ch（指向 left/right 的成员指针）组成。
        //
        // 通过这个抽象，所有树操作（插入、删除、旋转等）都统一在"边"的
        // 层面进行，无需区分根节点和普通节点——哨兵 root_node 的存在使
        // 树的根也有一条对应的边。
        //
        // 命名约定：
        //   go_* ：原地修改自身，返回 *this（链式调用）
        //   get_*：返回新的 proxy 对象，不修改自身
        // ===================================================================
        template<class T, class Alloc>
        class parent_aware_binary_tree_edge_proxy
        {
            using pointer_to_pointer = std::allocator_traits<Alloc>::template rebind_traits<typename std::allocator_traits<Alloc>::pointer>::pointer;
            using pointer = std::allocator_traits<Alloc>::pointer;
            // l, r：分别指向节点基类中 left, right 成员的成员指针常量
            static constexpr inline pointer parent_aware_binary_tree_node_base1<Alloc>::*
                l = &parent_aware_binary_tree_node_base1<Alloc>::left;
            static constexpr inline pointer parent_aware_binary_tree_node_base1<Alloc>::*
                r = (pointer parent_aware_binary_tree_node_base1<Alloc>::*)
                &parent_aware_binary_tree_node_base<Alloc>::right;
            // ch：当前边引用的是左子还是右子（指向 l 或 r）
            pointer parent_aware_binary_tree_node_base1<Alloc>::* ch = l;
            // parent：父节点指针
            pointer parent{};
            // 获取子节点指针的引用（即指针槽位本身）
            pointer& child()const noexcept
            {
                return static_cast<parent_aware_binary_tree_node_base1<Alloc>&>(*parent).*ch;
            }
        public:
            parent_aware_binary_tree_edge_proxy() = default;
            parent_aware_binary_tree_edge_proxy(const parent_aware_binary_tree_edge_proxy&) = default;
            // 构造一条从 parent 出发、方向为 left_flag 的边
            parent_aware_binary_tree_edge_proxy(pointer ptr, bool left_flag)noexcept:parent{ptr}, ch{left_flag ? l : r}
            { }
            parent_aware_binary_tree_edge_proxy& operator=(const parent_aware_binary_tree_edge_proxy&) = default;
            // 边是否有效（parent 非空）
            bool valid()const noexcept
            {
                return (parent != nullptr);
            }
            // 边所指的子节点是否为空
            bool null()const noexcept
            {
                return (child() == nullptr);
            }
            // 边有效且子节点非空
            explicit operator bool()const noexcept
            {
                return valid() && (!null());
            }
            // 移动到左子节点
            parent_aware_binary_tree_edge_proxy& go_left()noexcept
            {
                parent = child();
                ch = l;
                return *this;
            }
            // 移动到右子节点
            parent_aware_binary_tree_edge_proxy& go_right()noexcept
            {
                parent = child();
                ch = r;
                return *this;
            }
            // 移动到父节点
            parent_aware_binary_tree_edge_proxy& go_up()noexcept
            {
                pointer parent_of_parent = static_cast<parent_aware_binary_tree_node_base<Alloc>&>(*parent).parent;
                ch = static_cast<parent_aware_binary_tree_node_base1<Alloc>&>(*parent_of_parent).left == parent ? l : r;
                parent = parent_of_parent;
                return *this;
            }
            // 解引用：获取子节点中存储的值的引用
            T& operator*()const noexcept
            {
                return static_cast<parent_aware_binary_tree_node<T, Alloc>&>(*child()).data.data;
            }
            T* operator->()const noexcept
            {
                return std::addressof(static_cast<parent_aware_binary_tree_node<T, Alloc>&>(*child()).data.data);
            }
            // 获取左子边（不修改自身）
            parent_aware_binary_tree_edge_proxy get_left()const noexcept
            {
                return { child(), true };
            }
            // 获取右子边（不修改自身）
            parent_aware_binary_tree_edge_proxy get_right()const noexcept
            {
                return { child(), false };
            }
            // 获取父边（不修改自身）
            parent_aware_binary_tree_edge_proxy get_parent()const noexcept
            {
                pointer parent_of_parent = static_cast<parent_aware_binary_tree_node_base<Alloc>&>(*parent).parent;
                return { parent_of_parent, static_cast<parent_aware_binary_tree_node_base1<Alloc>&>(*parent_of_parent).left == parent };
            }
            // 获取左右子边
            std::pair<parent_aware_binary_tree_edge_proxy, parent_aware_binary_tree_edge_proxy> get_children()const noexcept
            {
                return { get_left(), get_right() };
            }
            // 比较两条边是否指向同一个指针槽位（而非比较值）
            friend bool operator==(parent_aware_binary_tree_edge_proxy l, parent_aware_binary_tree_edge_proxy r)noexcept = default;
            friend struct std::hash<parent_aware_binary_tree_edge_proxy>;
            friend class parent_aware_binary_tree_edge_const_proxy<T, Alloc>;
            template<class T1, class Alloc1>
            friend class Yc::parent_aware_binary_tree;
            friend class parent_aware_binary_tree_node_proxy<T, Alloc>;
            using value_type = T;
        };
    }
}

// std::hash 特化：基于子节点地址进行哈希
template<class T, class Alloc>
struct std::hash<Yc::details::parent_aware_binary_tree_edge_proxy<T, Alloc>>
{
    size_t operator()(const Yc::details::parent_aware_binary_tree_edge_proxy<T, Alloc>& para)const noexcept
    {
        if (!para.valid())
            return 0;
        return std::hash<std::remove_cvref_t<decltype(std::addressof(para.child()))>>{}(std::addressof(para.child()));
    }
};

namespace Yc
{
    namespace details
    {
        // edge_const_proxy：edge_proxy 的常量版本
        // 与 edge_proxy 结构相同，但解引用返回 const T&，
        // 且可从 edge_proxy 隐式转换而来
        template<class T, class Alloc>
        class parent_aware_binary_tree_edge_const_proxy
        {
            using pointer_to_pointer = std::allocator_traits<Alloc>::template rebind_traits<typename std::allocator_traits<Alloc>::pointer>::pointer;
            using pointer = std::allocator_traits<Alloc>::pointer;

            static constexpr inline pointer parent_aware_binary_tree_node_base1<Alloc>::*
                l = &parent_aware_binary_tree_node_base1<Alloc>::left;
            static constexpr inline pointer parent_aware_binary_tree_node_base1<Alloc>::*
                r = (pointer parent_aware_binary_tree_node_base1<Alloc>::*)
                & parent_aware_binary_tree_node_base<Alloc>::right;
            pointer parent_aware_binary_tree_node_base1<Alloc>::* ch = l;
            pointer parent{};
            pointer& child()const noexcept
            {
                return static_cast<parent_aware_binary_tree_node_base1<Alloc>&>(*parent).* ch;
            }
        public:
            parent_aware_binary_tree_edge_const_proxy() = default;
            parent_aware_binary_tree_edge_const_proxy(const parent_aware_binary_tree_edge_const_proxy&) = default;
            parent_aware_binary_tree_edge_const_proxy(pointer ptr, bool left_flag)noexcept :parent{ ptr }, ch{ left_flag ? l : r }
            {
            }
            parent_aware_binary_tree_edge_const_proxy(parent_aware_binary_tree_edge_proxy<T, Alloc> p)noexcept :
                parent_aware_binary_tree_edge_const_proxy{p.parent, p.ch == p.l}
            {}
            parent_aware_binary_tree_edge_const_proxy& operator=(const parent_aware_binary_tree_edge_const_proxy&) = default;
            bool valid()const noexcept
            {
                return bool( parent );
            }
            bool null()const noexcept
            {
                return !bool( child() );
            }
            explicit operator bool()const noexcept
            {
                return valid() && (!null());
            }
            parent_aware_binary_tree_edge_const_proxy& go_left()noexcept
            {
                parent = child();
                ch = l;
                return *this;
            }
            parent_aware_binary_tree_edge_const_proxy& go_right()noexcept
            {
                parent = child();
                ch = r;
                return *this;
            }
            parent_aware_binary_tree_edge_const_proxy& go_up()noexcept
            {
                pointer parent_of_parent = static_cast<parent_aware_binary_tree_node_base<Alloc>&>(*parent).parent;
                ch = static_cast<parent_aware_binary_tree_node_base1<Alloc>&>(*parent_of_parent).left == parent ? l : r;
                parent = parent_of_parent;
                return *this;
            }
            const T& operator*()const noexcept
            {
                return static_cast<parent_aware_binary_tree_node<T, Alloc>&>(*child()).data.data;
            }
            const T* operator->()const noexcept
            {
                return std::addressof(static_cast<parent_aware_binary_tree_node<T, Alloc>&>(*child()).data.data);
            }
            parent_aware_binary_tree_edge_const_proxy get_left()const noexcept
            {
                return { child(), true };
            }
            parent_aware_binary_tree_edge_const_proxy get_right()const noexcept
            {
                return { child(), false };
            }
            parent_aware_binary_tree_edge_const_proxy get_parent()const noexcept
            {
                pointer parent_of_parent = static_cast<parent_aware_binary_tree_node_base<Alloc>&>(*parent).parent;
                return { parent_of_parent, static_cast<parent_aware_binary_tree_node_base1<Alloc>&>(*parent_of_parent).left == parent };
            }
            std::pair<parent_aware_binary_tree_edge_const_proxy, parent_aware_binary_tree_edge_const_proxy> get_children()const noexcept
            {
                return { get_left(), get_right() };
            }
            friend bool operator==(parent_aware_binary_tree_edge_const_proxy, parent_aware_binary_tree_edge_const_proxy)noexcept = default;
            friend struct std::hash<parent_aware_binary_tree_edge_const_proxy>;
            template<class T1, class Alloc1>
            friend class Yc::parent_aware_binary_tree;
            friend class parent_aware_binary_tree_node_const_proxy<T, Alloc>;
            using value_type = T;
        };
    }
}

// std::hash 特化：基于子节点地址进行哈希
template<class T, class Alloc>
struct std::hash<Yc::details::parent_aware_binary_tree_edge_const_proxy<T, Alloc>>
{
    size_t operator()(const Yc::details::parent_aware_binary_tree_edge_const_proxy<T, Alloc>& para)const noexcept
    {
        if (!para.valid())
            return 0;
        return std::hash<std::remove_cvref_t<decltype(std::addressof(para.child()))>>{}(std::addressof(para.child()));
    }
};

namespace Yc
{
    namespace details
    {
        // ===================================================================
        // node_proxy：直接代表树节点
        // 与 edge_proxy 不同，node_proxy 直接持有节点指针（而非指向指针的指针），
        // 支持 go_left/go_right/go_up 导航和值访问。
        // 可通过转换运算符转为 edge_proxy（利用 parent 指针重建边）。
        // ===================================================================
        template<class T, class Alloc>
        class parent_aware_binary_tree_node_proxy
        {
            using pointer_to_pointer = std::allocator_traits<Alloc>::template rebind_traits<typename std::allocator_traits<Alloc>::pointer>::pointer;
            using pointer = std::allocator_traits<Alloc>::pointer;

            pointer node{};
        public:
            parent_aware_binary_tree_node_proxy() = default;
            parent_aware_binary_tree_node_proxy(const parent_aware_binary_tree_node_proxy&) = default;
            parent_aware_binary_tree_node_proxy(pointer node)noexcept:node{node}
            {
            }
            parent_aware_binary_tree_node_proxy(parent_aware_binary_tree_edge_proxy<T, Alloc> p)noexcept :node{p.child()}
            {
            }
            parent_aware_binary_tree_node_proxy& operator=(const parent_aware_binary_tree_node_proxy&) = default;
            bool null()const noexcept
            {
                return node == nullptr;
            }
            explicit operator bool()const noexcept
            {
                return !null();
            }
            parent_aware_binary_tree_node_proxy& go_left()noexcept
            {
                node = static_cast<parent_aware_binary_tree_node_base1<Alloc>&>(*node).left;
                return *this;
            }
            parent_aware_binary_tree_node_proxy& go_right()noexcept
            {
                node = static_cast<parent_aware_binary_tree_node_base<Alloc>&>(*node).right;
                return *this;
            }
            parent_aware_binary_tree_node_proxy& go_up()noexcept
            {
                node = static_cast<parent_aware_binary_tree_node_base<Alloc>&>(*node).parent;
                return *this;
            }
            T& operator*()const noexcept
            {
                return static_cast<parent_aware_binary_tree_node<T, Alloc>&>(*node).data.data;
            }
            T* operator->()const noexcept
            {
                return std::addressof(static_cast<parent_aware_binary_tree_node<T, Alloc>&>(*node).data.data);
            }
            parent_aware_binary_tree_node_proxy get_left()const noexcept
            {
                return { static_cast<parent_aware_binary_tree_node_base1<Alloc>&>(*node).left };
            }
            parent_aware_binary_tree_node_proxy get_right()const noexcept
            {
                return { static_cast<parent_aware_binary_tree_node_base<Alloc>&>(*node).right };
            }
            parent_aware_binary_tree_node_proxy get_parent()const noexcept
            {
                return { static_cast<parent_aware_binary_tree_node_base<Alloc>&>(*node).parent };
            }
            std::pair<parent_aware_binary_tree_edge_proxy<T, Alloc>, parent_aware_binary_tree_edge_proxy<T, Alloc>> get_children()const noexcept
            {
                return { {node, true}, {node, false} };
            }
            operator parent_aware_binary_tree_edge_proxy<T, Alloc>()const noexcept
            {
                pointer parent = static_cast<parent_aware_binary_tree_node_base<Alloc>&>(*node).parent;
                if (static_cast<parent_aware_binary_tree_node_base1<Alloc>&>(*parent).left == node)
                {
                    return { parent, true };
                }
                else
                {
                    return { parent, false };
                }
            }
            friend bool operator==(parent_aware_binary_tree_node_proxy l, parent_aware_binary_tree_node_proxy r)noexcept = default;
            friend struct std::hash<parent_aware_binary_tree_node_proxy>;
            friend class parent_aware_binary_tree_node_const_proxy<T, Alloc>;
            using value_type = T;
            template<class T1, class Alloc1>
            friend class Yc::parent_aware_binary_tree;
        };
    }
}

// std::hash 特化：基于所指代节点的地址进行哈希
template<class T, class Alloc>
struct std::hash<Yc::details::parent_aware_binary_tree_node_proxy<T, Alloc>>
{
    size_t operator()(const Yc::details::parent_aware_binary_tree_node_proxy<T, Alloc>& para)const noexcept
    {
        return std::hash<decltype(para.node)>{}(para.node);
    }
};

namespace Yc
{
    namespace details
    {
        // node_const_proxy：node_proxy 的常量版本
        // 解引用返回 const T&，可从 edge_const_proxy 转换而来
        template<class T, class Alloc>
        class parent_aware_binary_tree_node_const_proxy
        {
            using pointer_to_pointer = std::allocator_traits<Alloc>::template rebind_traits<typename std::allocator_traits<Alloc>::pointer>::pointer;
            using pointer = std::allocator_traits<Alloc>::pointer;

            pointer node{};
        public:
            parent_aware_binary_tree_node_const_proxy() = default;
            parent_aware_binary_tree_node_const_proxy(const parent_aware_binary_tree_node_const_proxy&) = default;
            parent_aware_binary_tree_node_const_proxy(pointer node)noexcept :node{ node }
            {}
            parent_aware_binary_tree_node_const_proxy(parent_aware_binary_tree_edge_const_proxy<T, Alloc> p)noexcept :node{ p.child() }
            {}
            parent_aware_binary_tree_node_const_proxy& operator=(const parent_aware_binary_tree_node_const_proxy&) = default;
            bool null()const noexcept
            {
                return node == nullptr;
            }
            explicit operator bool()const noexcept
            {
                return !null();
            }
            parent_aware_binary_tree_node_const_proxy& go_left()noexcept
            {
                node = static_cast<parent_aware_binary_tree_node_base1<Alloc>&>(*node).left;
                return *this;
            }
            parent_aware_binary_tree_node_const_proxy& go_right()noexcept
            {
                node = static_cast<parent_aware_binary_tree_node_base<Alloc>&>(*node).right;
                return *this;
            }
            parent_aware_binary_tree_node_const_proxy& go_up()noexcept
            {
                node = static_cast<parent_aware_binary_tree_node_base<Alloc>&>(*node).parent;
                return *this;
            }
            const T& operator*()const noexcept
            {
                return static_cast<parent_aware_binary_tree_node<T, Alloc>&>(*node).data.data;
            }
            const T* operator->()const noexcept
            {
                return std::addressof(static_cast<parent_aware_binary_tree_node<T, Alloc>&>(*node).data.data);
            }
            parent_aware_binary_tree_node_const_proxy get_left()const noexcept
            {
                return { static_cast<parent_aware_binary_tree_node_base1<Alloc>&>(*node).left };
            }
            parent_aware_binary_tree_node_const_proxy get_right()const noexcept
            {
                return { static_cast<parent_aware_binary_tree_node_base<Alloc>&>(*node).right };
            }
            parent_aware_binary_tree_node_const_proxy get_parent()const noexcept
            {
                return { static_cast<parent_aware_binary_tree_node_base<Alloc>&>(*node).parent };
            }
            std::pair<parent_aware_binary_tree_edge_const_proxy<T, Alloc>, parent_aware_binary_tree_edge_const_proxy<T, Alloc>> get_children()const noexcept
            {
                return { {node, true}, {node, false} };
            }
            operator parent_aware_binary_tree_edge_const_proxy<T, Alloc>()const noexcept
            {
                pointer parent = static_cast<parent_aware_binary_tree_node_base<Alloc>&>(*node).parent;
                if (static_cast<parent_aware_binary_tree_node_base1<Alloc>&>(*parent).left == node)
                {
                    return { parent, true };
                }
                else
                {
                    return { parent, false };
                }
            }
            friend bool operator==(parent_aware_binary_tree_node_const_proxy l, parent_aware_binary_tree_node_const_proxy r)noexcept = default;
            friend struct std::hash<parent_aware_binary_tree_node_const_proxy>;
            using value_type = T;
            template<class T1, class Alloc1>
            friend class Yc::parent_aware_binary_tree;
        };
    }
}

// std::hash 特化：基于所指代节点的地址进行哈希
template<class T, class Alloc>
struct std::hash<Yc::details::parent_aware_binary_tree_node_const_proxy<T, Alloc>>
{
    size_t operator()(const Yc::details::parent_aware_binary_tree_node_const_proxy<T, Alloc>& para)const noexcept
    {
        return std::hash<decltype(para.node)>{}(para.node);
    }
};

namespace Yc
{
    namespace details
    {
        // 多态 get_children 函数对象：对四种 proxy 类型统一调用 get_children()
        struct parent_aware_binary_tree_get_children_t
        {
            template<class T, class Alloc>
            auto operator()(parent_aware_binary_tree_edge_proxy<T, Alloc> p)const noexcept
            {
                return p.get_children();
            }
            template<class T, class Alloc>
            auto operator()(parent_aware_binary_tree_edge_const_proxy<T, Alloc> p)const noexcept
            {
                return p.get_children();
            }
            template<class T, class Alloc>
            auto operator()(parent_aware_binary_tree_node_proxy<T, Alloc> p)const noexcept
            {
                return p.get_children();
            }
            template<class T, class Alloc>
            auto operator()(parent_aware_binary_tree_node_const_proxy<T, Alloc> p)const noexcept
            {
                return p.get_children();
            }
        };
    }
    namespace parent_aware_binary_tree_functional
    {
        constexpr inline Yc::details::parent_aware_binary_tree_get_children_t get_children{};
    }
    enum child_check
    {
        no_check = 0,
        check_left = 1,
        check_right = 2,
        check_left_rev = 4,
        check_right_rev = 8,
        check_all = 15, // check_all == check_left | check_right | check_left_rev | check_right_rev
    };
    // ===================================================================
    // parent_aware_binary_tree：带父节点指针的二叉树容器
    //
    // 核心设计：
    //   1. 使用哨兵节点 root_node 作为树的逻辑根的父亲，
    //      这使得根节点的父指针也有效，统一了边界情况处理。
    //   2. 所有公开操作通过 edge_proxy / edge_const_proxy 暴露，
    //      用户不直接操作裸指针。
    //   3. 分配器感知：支持有状态分配器，遵循 allocator_traits 约定。
    //
    // 主要操作：
    //   - 导航：root(), nroot(), parent_of_nroot()
    //   - 修改：emplace, insert, erase, cut, splice
    //   - 旋转：left_rotate, right_rotate, checked_left_rotate, checked_right_rotate
    //   - 交换：swap, swap_sub_tree, swap_node
    //   - 递归构造：recur_and_write（支持从任意树形结构复制/构造）
    // ===================================================================
    template<class T, class Alloc = std::allocator<T>>
    class parent_aware_binary_tree
    {
        // 分配器 rebind：将用户分配器 rebind 到占位类型，统一管理节点内存
        using place_holder_allocator = std::allocator_traits<Alloc>::template
            rebind_alloc<details::parent_aware_binary_tree_empty_placeholder>;
        using node_pointer = std::allocator_traits<place_holder_allocator>::pointer;
        using node_type = details::parent_aware_binary_tree_node<T, place_holder_allocator>;
        using node_allocator = std::allocator_traits<Alloc>::template rebind_alloc<node_type>;
    public:
        using allocator_type = Alloc;
        using value_type = T;
        using reference = T&;
        using edge_proxy = details::parent_aware_binary_tree_edge_proxy<T, place_holder_allocator>;
        using edge_const_proxy = details::parent_aware_binary_tree_edge_const_proxy<T, place_holder_allocator>;
        using node_proxy = details::parent_aware_binary_tree_node_proxy<T, place_holder_allocator>;
        using node_const_proxy = details::parent_aware_binary_tree_node_const_proxy<T, place_holder_allocator>;
    private:
        // 分配器（使用 [[no_unique_address]] 以零开销存储无状态分配器）
        [[no_unique_address]] Alloc alloc{};
        // 哨兵根节点：自身不存储数据，仅通过 left 指针引用实际树根
        // 使根的父指针也有效，统一处理所有边界情况
        details::parent_aware_binary_tree_node_base1<place_holder_allocator> root_node{};
        // 获取占位符对象的 fancy pointer（适配 fancy pointer 类型）
        node_pointer pointer_to(const details::parent_aware_binary_tree_empty_placeholder& ref)const noexcept
        {
            return std::pointer_traits<node_pointer>::pointer_to((details::parent_aware_binary_tree_empty_placeholder&)ref);
        }
        // 分配并构造一个节点（基类部分默认构造，用户数据需另行构造）
        node_pointer allocate_node()
        {
            node_allocator a{ alloc };
            node_pointer p = std::pointer_traits<node_pointer>::pointer_to
                (static_cast<details::parent_aware_binary_tree_empty_placeholder&>(*std::allocator_traits<node_allocator>::allocate(a, 1)));
            std::allocator_traits<node_allocator>::construct(a, (node_type*)std::to_address(p));
            return p;
        }
        // 析构并释放节点
        void deallocate_node(node_pointer p)noexcept
        {
            node_allocator a{ alloc };
            std::allocator_traits<node_allocator>::destroy(a, (node_type*)std::to_address(p));
            return std::allocator_traits<node_allocator>::deallocate(a,
                std::pointer_traits<typename std::allocator_traits<node_allocator>::pointer>::pointer_to(*(node_type*)std::to_address(p)), 1);
        }
    public:
        Alloc get_allocator()const noexcept
        {
            return alloc;
        }
        // 获取哨兵根节点对应的 node_proxy（即树的"父节点的父节点"）
        node_proxy parent_of_nroot()noexcept
        {
            return { pointer_to(root_node) };
        }
        node_const_proxy parent_of_nroot()const noexcept
        {
            return { pointer_to(root_node) };
        }
        node_const_proxy parent_of_cnroot()const noexcept
        {
            return { pointer_to(root_node) };
        }
        // 获取树根的 edge_proxy（哨兵 root_node 到实际树根的边）
        edge_proxy root()noexcept
        {
            return { pointer_to(root_node), true };
        }
        edge_const_proxy root()const noexcept
        {
            return { pointer_to(root_node), true };
        }
        edge_const_proxy croot()const noexcept
        {
            return { pointer_to(root_node), true };
        }
        // 获取树根对应的 node_proxy
        node_proxy nroot()noexcept
        {
            return parent_of_nroot().go_left();
        }
        node_const_proxy nroot()const noexcept
        {
            return parent_of_nroot().go_left();
        }
        node_const_proxy cnroot()const noexcept
        {
            return parent_of_cnroot().go_left();
        }
    private:
        // 递归写入实现（使用 goto 实现尾递归）
        template<
            class ValueGetter,
            class ChildrenGetter,
            class InitializeHandle
        >
        void recur_and_write_impl(
            edge_const_proxy p,
            ValueGetter& vg,
            ChildrenGetter& cg,
            InitializeHandle h
        )
        {
        derecur:
            if (!p.null())
            {
                std::unreachable();
            }
            emplace(p, std::invoke(vg, h));
            auto [lh, rh] = std::invoke(cg, h);
            auto [l, r] = p.get_children();
            bool bl = (bool)lh, br = (bool)rh;
            if (!(bl && br))
            {
                if (bl)
                {
                    p = l;
                    h = lh;
                    goto derecur;
                }
                if (br)
                {
                tmp:
                    p = r;
                    h = rh;
                    goto derecur;
                }
                return;
            }
            recur_and_write_impl(l, vg, cg, lh);
            goto tmp;
        }
    public:
        // 递归写入：从外部树形结构（由 h 表示的句柄）复制数据到当前树
        // vg：值获取器（从句柄提取 T 值）
        // cg：子节点获取器（从句柄提取左右子句柄）
        // h：当前句柄（bool 可转换为 false 表示空）
        // 约束：h 必须可 bool 转换
        template<
            class ValueGetter,
            class ChildrenGetter,
            class InitializeHandle
        >
        parent_aware_binary_tree recur_and_write(
            edge_const_proxy p,
            ValueGetter vg,
            ChildrenGetter cg,
            InitializeHandle h
        )
        {
            // 先切割原有子树，新建一棵临时树，构造完成后拼接回去
            parent_aware_binary_tree ret{ cut(p) };
            // guard：若构造过程中异常，自动将已构造部分拼回原树
            struct _guard
            {
                parent_aware_binary_tree* t;
                edge_const_proxy p;
                parent_aware_binary_tree* r;
                ~_guard()
                {
                    if (t)
                    {
                        t->splice(p, *r);
                    }
                }
            }guard{ this, p, &ret };
            if (h)
            {
                recur_and_write_impl(p, vg, cg, h);
            }
            guard.t = nullptr;  // 成功：解除 guard
            return ret;
        }
        // 从树形结构构造整棵树
        template<
            class ValueGetter,
            class ChildrenGetter,
            class InitializeHandle
        >
        parent_aware_binary_tree(
            ValueGetter vg,
            ChildrenGetter cg,
            InitializeHandle h,
            const Alloc& a = {}
        ) :alloc{ a }
        {
            if (h)
            {
                recur_and_write_impl(root(), vg, cg, h);
            }
        }
        // 拷贝构造：递归复制整棵树的结构和值
        parent_aware_binary_tree(const parent_aware_binary_tree& b) :alloc{ std::allocator_traits<Alloc>::select_on_container_copy_construction(b.alloc) }
        {
            auto copier = [](edge_const_proxy p) -> const T&
                {
                    return *p;
                };
            recur_and_write
                (root(), copier, parent_aware_binary_tree_functional::get_children, b.croot());
        }
        // 拷贝赋值
        parent_aware_binary_tree& operator=(const parent_aware_binary_tree& b)
        {
            Alloc tmp{ alloc };
            parent_aware_binary_tree t1{ cut(root()) };  // 临时持有旧树
            // guard：若复制过程异常，恢复原分配器和旧树
            struct _guard
            {
                parent_aware_binary_tree* t{};
                Alloc* tmp{};
                parent_aware_binary_tree* other{};
                ~_guard()
                {
                    if (t)
                    {
                        t->alloc = *tmp;
                        splice(*this, root(), *other);
                    }
                }
            }guard{ this, &tmp, &t1 };
            if constexpr (
                std::allocator_traits<Alloc>::propagate_on_container_copy_assignment::value)
            {
                alloc = b.alloc;
            }
            auto copier = [](edge_const_proxy p)->const T&
                {
                    return *p;
                };
            recur_and_write(root(), copier, parent_aware_binary_tree_functional::get_children, b.root());
            guard.t = nullptr;
            return *this;
        }
        // 切割：将 p 指向的子树从原树分离，返回一棵包含该子树的新树
        parent_aware_binary_tree cut(edge_const_proxy p)noexcept
        {
            typename edge_const_proxy::pointer& place = p.child();
            parent_aware_binary_tree ret{ alloc };
            ret.root_node.left = std::exchange(place, {});  // 断开原树连接
            
            if (ret.root_node.left)
            {
                static_cast<node_type&>(*ret.root_node.left).parent = pointer_to(ret.root_node);
            }
            return ret;
        }
        
        bool empty()const noexcept
        {
            return root_node.left == nullptr;
        }
        // 删除指定边上的节点及其所有子节点（迭代式，避免递归栈溢出）
        void erase(edge_const_proxy p)noexcept
        {
            if (!p.null())
            {
                typename edge_const_proxy::pointer& place = p.child();
                while (true)
                {
                    auto [l, r] = p.get_children();
                    bool l_null = l.null();
                    bool r_null = r.null();
                    if (l_null && r_null)
                    {
                        std::allocator_traits<Alloc>::destroy(alloc, (T*)p.operator->());
                        deallocate_node(place);
                        place = nullptr;
                        return;
                    }
                    if (!r_null)
                    {
                        erase(l);
                        typename edge_const_proxy::pointer right = r.child();
                        std::allocator_traits<Alloc>::destroy(alloc, (T*)p.operator->());
                        deallocate_node(place);
                        place = right;
                        continue;
                    }
                    if ((!l_null) && r_null)
                    {
                        typename edge_const_proxy::pointer left = l.child();
                        std::allocator_traits<Alloc>::destroy(alloc, (T*)p.operator->());
                        deallocate_node(place);
                        place = left;
                        continue;
                    }
                }
            }
        }
        // 清空整棵树
        void clear()noexcept
        {
            erase(root());
        }

        // 在指定边位置原地构造新节点（已有子树作为切割结果返回）
        template<class... Args>
        parent_aware_binary_tree emplace(edge_const_proxy p, Args&&... args)
        {
            node_pointer new_node = allocate_node();
            if constexpr (std::is_nothrow_constructible_v<T, Args...>)
            {
                std::allocator_traits<Alloc>::
                    construct(alloc, std::addressof(((node_type&)*new_node).data.data), std::forward<Args>(args)...);
            }
            else
            {
                struct guard_t
                {
                    parent_aware_binary_tree* t;
                    node_pointer ptr;
                    ~guard_t()
                    {
                        if (ptr != nullptr)
                        {
                            t->deallocate_node(ptr);
                        }
                    }
                }guard{ this,new_node };
                std::allocator_traits<Alloc>::
                    construct(alloc, std::addressof(((node_type&)*new_node).data.data), std::forward<Args>(args)...);
                guard.ptr = nullptr;
            }
            parent_aware_binary_tree ret{ cut(p) };
            p.child() = new_node;
            static_cast<node_type&>(*new_node).parent = p.parent;
            return ret;
        }
        parent_aware_binary_tree insert(edge_const_proxy p, const T& v)
        {
            return emplace(p, v);
        }
        parent_aware_binary_tree insert(edge_const_proxy p, T&& v)
        {
            return emplace(p, std::move(v));
        }

        // 拼接：将 from 子树移动到 to 位置，原 to 子树作为结果返回
        parent_aware_binary_tree splice(edge_const_proxy to, edge_const_proxy from)noexcept
        {
            parent_aware_binary_tree ret{ cut(to) };
            to.child() = std::exchange(from.child(), {});
            if (to)
            {
                static_cast<node_type&>(*to.child()).parent = to.parent;
            }
            return ret;
        }

        // 拼接整棵树的根
        parent_aware_binary_tree splice(edge_const_proxy p, parent_aware_binary_tree& tree)noexcept
        {
            return splice(p, tree.root());
        }

        parent_aware_binary_tree splice(edge_const_proxy p, parent_aware_binary_tree&& tree)noexcept
        {
            return splice(p, tree.root());
        }

        // 交换两个子树（保持父指针正确）
        static void swap_sub_tree(edge_const_proxy l, edge_const_proxy r)noexcept
        {
            typename edge_const_proxy::pointer& place_l = l.child();
            typename edge_const_proxy::pointer& place_r = r.child();
            std::swap(place_l, place_r);
            if (place_l)
            {
                static_cast<node_type&>(*place_l).parent = l.parent;
            }
            if (place_r)
            {
                static_cast<node_type&>(*place_r).parent = r.parent;
            }
        }

    private:
        static void swap_node_uncheck(edge_const_proxy l, edge_const_proxy r)noexcept
        {
            swap_sub_tree(l, r);
            auto [ll, lr] = l.get_children();
            auto [rl, rr] = r.get_children();
            swap_sub_tree(ll, rl);
            swap_sub_tree(lr, rr);
        }
    public:
        // 交换两个节点（同时交换其子树）
        static void swap_node(edge_const_proxy l, edge_const_proxy r, child_check chk = check_all)noexcept
        {
            if (chk & check_left)
            {
                if (l.get_left() == r)
                {
                    node_const_proxy nl = l;
                    node_const_proxy nr = r;
                    auto tmp = static_cast<node_type&>(*nl.node).right;
                    auto tmp1 = static_cast<node_type&>(*nl.node).parent;
                    auto tmpl = static_cast<node_type&>(*nr.node).left;
                    auto tmpr = static_cast<node_type&>(*nr.node).right;
                    static_cast<node_type&>(*nl.node).left = tmpl;
                    static_cast<node_type&>(*nl.node).right = tmpr;
                    static_cast<node_type&>(*nl.node).parent = nr.node;

                    static_cast<node_type&>(*nr.node).left = nl.node;
                    static_cast<node_type&>(*nr.node).right = tmp;
                    static_cast<node_type&>(*nr.node).parent = tmp1;
                    if (tmpl)
                        static_cast<node_type&>(*tmpl).parent = nl.node;
                    if (tmpr)
                        static_cast<node_type&>(*tmpr).parent = nl.node;
                    if (tmp)
                        static_cast<node_type&>(*tmp).parent = nr.node;
                    l.child() = nr.node;
                    return;
                }
            }
            if (chk & check_right)
            {
                if (l.get_right() == r)
                {
                    node_const_proxy nl = l;
                    node_const_proxy nr = r;
                    auto tmp = static_cast<node_type&>(*nl.node).left;
                    auto tmp1 = static_cast<node_type&>(*nl.node).parent;
                    auto tmpl = static_cast<node_type&>(*nr.node).left;
                    auto tmpr = static_cast<node_type&>(*nr.node).right;
                    static_cast<node_type&>(*nl.node).left = tmpl;
                    static_cast<node_type&>(*nl.node).right = tmpr;
                    static_cast<node_type&>(*nl.node).parent = nr.node;

                    static_cast<node_type&>(*nr.node).left = tmp;
                    static_cast<node_type&>(*nr.node).right = nl.node;
                    static_cast<node_type&>(*nr.node).parent = tmp1;
                    if (tmpl)
                        static_cast<node_type&>(*tmpl).parent = nl.node;
                    if (tmpr)
                        static_cast<node_type&>(*tmpr).parent = nl.node;
                    if (tmp)
                        static_cast<node_type&>(*tmp).parent = nr.node;
                    l.child() = nr.node;
                    return;
                }
            }
            std::swap(l, r);
            if (chk & check_left_rev)
            {
                if (l.get_left() == r)
                {
                    node_const_proxy nl = l;
                    node_const_proxy nr = r;
                    auto tmp = static_cast<node_type&>(*nl.node).right;
                    auto tmp1 = static_cast<node_type&>(*nl.node).parent;
                    auto tmpl = static_cast<node_type&>(*nr.node).left;
                    auto tmpr = static_cast<node_type&>(*nr.node).right;
                    static_cast<node_type&>(*nl.node).left = tmpl;
                    static_cast<node_type&>(*nl.node).right = tmpr;
                    static_cast<node_type&>(*nl.node).parent = nr.node;

                    static_cast<node_type&>(*nr.node).left = nl.node;
                    static_cast<node_type&>(*nr.node).right = tmp;
                    static_cast<node_type&>(*nr.node).parent = tmp1;
                    if (tmpl)
                        static_cast<node_type&>(*tmpl).parent = nl.node;
                    if (tmpr)
                        static_cast<node_type&>(*tmpr).parent = nl.node;
                    if (tmp)
                        static_cast<node_type&>(*tmp).parent = nr.node;
                    l.child() = nr.node;
                    return;
                }
            }
            if (chk & check_right_rev)
            {
                if (l.get_right() == r)
                {
                    node_const_proxy nl = l;
                    node_const_proxy nr = r;
                    auto tmp = static_cast<node_type&>(*nl.node).left;
                    auto tmp1 = static_cast<node_type&>(*nl.node).parent;
                    auto tmpl = static_cast<node_type&>(*nr.node).left;
                    auto tmpr = static_cast<node_type&>(*nr.node).right;
                    static_cast<node_type&>(*nl.node).left = tmpl;
                    static_cast<node_type&>(*nl.node).right = tmpr;
                    static_cast<node_type&>(*nl.node).parent = nr.node;

                    static_cast<node_type&>(*nr.node).left = tmp;
                    static_cast<node_type&>(*nr.node).right = nl.node;
                    static_cast<node_type&>(*nr.node).parent = tmp1;
                    if (tmpl)
                        static_cast<node_type&>(*tmpl).parent = nl.node;
                    if (tmpr)
                        static_cast<node_type&>(*tmpr).parent = nl.node;
                    if (tmp)
                        static_cast<node_type&>(*tmp).parent = nr.node;
                    l.child() = nr.node;
                    return;
                }
            }
            swap_node_uncheck(l, r);
        }

        // 交换两棵树
        void swap(parent_aware_binary_tree& other)noexcept
        {
            if constexpr (std::allocator_traits<Alloc>::propagate_on_container_swap::value)
            {
                using namespace std;
                swap(alloc, other.alloc);
            }
            swap_sub_tree(root(), other.root());
        }

        parent_aware_binary_tree() = default;
        // 仅指定分配器的构造
        explicit parent_aware_binary_tree(const Alloc& a)noexcept:alloc{a}
        {
        }
        

        // 移动构造：若分配器支持传播则交换分配器
        parent_aware_binary_tree(parent_aware_binary_tree&& other)noexcept :alloc{other.alloc}
        {
            swap_sub_tree(root(), other.root());
        }
        // 移动赋值
        parent_aware_binary_tree& operator=(parent_aware_binary_tree&& other)noexcept(std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value
            || std::allocator_traits<Alloc>::is_always_equal::value)
        {
            if constexpr
                (std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value)
            {
                using namespace std;
                swap(alloc, other.alloc);
                swap_sub_tree(root(), other.root());
                return *this;
            }
            else
            {
                if (alloc == other.alloc)
                {
                    swap_sub_tree(root(), other.root());
                    return *this;
                }
                auto vg = [](edge_proxy p) -> T&&
                    {
                        return (T&&)*p;
                    };
                parent_aware_binary_tree tmp{ vg, parent_aware_binary_tree_functional::get_children, other.root(), alloc };
                swap_sub_tree(root(), tmp.root());
                return *this;
            }
        }
        // 析构：清空所有节点
        ~parent_aware_binary_tree()
        {
            clear();
        }

        // 左旋：将 p 的右子节点提升为 p 的位置
        //     |                         |
        //    <A>                       <C>
        //   /   \                      / \
        //  <B>  <C>     =====>       <A> <E>
        //       / \                  / \
        //     <D> <E>              <B> <D>
        void left_rotate(edge_const_proxy p)noexcept
        {
            parent_aware_binary_tree t1 = cut(p.get_right()); // <D>-<rC>-<E>
            parent_aware_binary_tree t2 = splice(p, t1.root()); //<B>-<rA>
            //     |
            //    <C>                 <A>
            //   /   \                /
            //  <D>  <E>            <B>
            p.go_left();
            parent_aware_binary_tree t3 = splice(p, t2.root());
            p.go_right();
            splice(p, t3.root());
            if (!t1.empty())
            {
                std::unreachable();
            }
            if (!t2.empty())
            {
                std::unreachable();
            }
            if (!t3.empty())
            {
                std::unreachable();
            }
        }

        // 右旋：左旋的镜像操作，将 p 的左子节点提升为 p 的位置
        void right_rotate(edge_const_proxy p)noexcept
        {
            parent_aware_binary_tree t1 = cut(p.get_left());
            parent_aware_binary_tree t2 = splice(p, t1.root());
            p.go_right();
            parent_aware_binary_tree t3 = splice(p, t2.root());
            p.go_left();
            splice(p, t3.root());
            if (!t1.empty())
            {
                std::unreachable();
            }
            if (!t2.empty())
            {
                std::unreachable();
            }
            if (!t3.empty())
            {
                std::unreachable();
            }
        }

        // 带检查的左旋：若右子节点存在则执行左旋，否则无操作
        bool checked_left_rotate(edge_const_proxy p)noexcept
        {
            edge_const_proxy q = p;
            if (p.null() || (p.go_right(), p.null()))
            {
                return false;
            }
            left_rotate(q);
            return true;
        }

        // 带检查的右旋：若左子节点存在则执行右旋，否则无操作
        bool checked_right_rotate(edge_const_proxy p)noexcept
        {
            edge_const_proxy q = p;
            if (p.null() || (p.go_left(), p.null()))
            {
                return false;
            }
            right_rotate(q);
            return true;
        }

    };
    // 特化 swap 自由函数
    template<class T, class Alloc>
    void swap(parent_aware_binary_tree<T, Alloc>& l, parent_aware_binary_tree<T, Alloc>& r)noexcept
    {
        l.swap(r);
    }
}