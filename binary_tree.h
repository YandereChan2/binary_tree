// 二叉树库（无父节点指针版本）
// 与 parent_aware_binary_tree 相比，本实现不维护父节点指针，
// 因此更轻量，但无法进行 go_up 导航和某些需要父指针的操作。
//
// 核心抽象：
//   - edge_proxy / edge_const_proxy：封装一个指向子节点指针的"指针槽位"（pointer_to_pointer），
//     支持导航（go_left/go_right）和修改（emplace/erase 等）
//   - node_proxy / node_const_proxy：直接代表树节点
//
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
    class binary_tree;
    namespace details
    {
        // ===================================================================
        // 节点层次结构：
        //   binary_tree_empty_placeholder       （空类型，用于类型擦除）
        //   └─ binary_tree_node_base<Alloc>     （left、right 指针）
        //       └─ binary_tree_node<T, Alloc>   （存储用户数据 data_t<T>）
        //
        // 与 parent_aware_binary_tree 不同，本层次结构更简单：
        // 无 parent 指针，无 node_base1 分层。
        // ===================================================================

        // 最底层基类：空类型占位符，用于统一 allocator rebound 的目标类型
        struct binary_tree_empty_placeholder
        { };
        // 节点基类：持有 left、right 子节点指针
        template<class Alloc>
        struct binary_tree_node_base : binary_tree_empty_placeholder
        {
            static_assert(std::is_same_v<typename std::allocator_traits<Alloc>::value_type, binary_tree_empty_placeholder>);
            using pointer = std::allocator_traits<Alloc>::pointer;
            pointer left;
            pointer right;
        };
        // 完整节点：继承全部指针并附加用户数据
        template<class T, class Alloc>
        struct binary_tree_node : binary_tree_node_base<Alloc>
        {
            data_t<T> data;
        };
        template<class T, class Alloc>
        class binary_tree_edge_const_proxy;
        template<class T, class Alloc>
        class binary_tree_node_proxy;
        template<class T, class Alloc>
        class binary_tree_node_const_proxy;
        // ===================================================================
        // edge_proxy：代表父节点到子节点之间的"边"（指针槽位）
        //
        // 与 parent_aware_binary_tree 版本不同，这里的边直接通过
        // pointer_to_pointer（指向 left/right 指针的指针）实现，
        // 而非使用成员指针。这使得实现更简单，但代价是每次导航
        // 需要通过 pointer_traits 重新计算地址。
        //
        // 命名约定：
        //   go_* ：原地修改自身，返回 *this（链式调用）
        //   get_*：返回新的 proxy 对象，不修改自身
        // ===================================================================
        template<class T, class Alloc>
        class binary_tree_edge_proxy
        {
            using pointer_to_pointer = std::allocator_traits<Alloc>::template rebind_traits<typename std::allocator_traits<Alloc>::pointer>::pointer;
            using pointer = std::allocator_traits<Alloc>::pointer;
            
            // ptr：指向节点中 left（或 right）成员的指针，
            //      即 **ptr 得到 binary_tree_empty_placeholder&
            pointer_to_pointer ptr{};
        public:
            binary_tree_edge_proxy() = default;
            binary_tree_edge_proxy(const binary_tree_edge_proxy&) = default;
            binary_tree_edge_proxy(pointer_to_pointer ptr)noexcept :ptr{ptr}
            {
            }
            binary_tree_edge_proxy& operator=(const binary_tree_edge_proxy&) = default;
            // 边是否有效（ptr 非空）
            bool valid()const noexcept
            {
                return (bool)ptr;
            }
            // 边所指的子节点是否为空
            bool null()const noexcept
            {
                return !(bool)*ptr;
            }
            // 边有效且子节点非空
            explicit operator bool()const noexcept
            {
                return valid() && (!null());
            }
            // 移动到左子节点（通过 pointer_traits 计算左子指针的地址）
            binary_tree_edge_proxy& go_left()noexcept
            {
                ptr = std::pointer_traits<pointer_to_pointer>::pointer_to(static_cast<binary_tree_node<T, Alloc>&>(**ptr).left);
                return *this;
            }
            // 移动到右子节点
            binary_tree_edge_proxy& go_right()noexcept
            {
                ptr = std::pointer_traits<pointer_to_pointer>::pointer_to(static_cast<binary_tree_node<T, Alloc>&>(**ptr).right);
                return *this;
            }
            // 解引用：获取子节点中存储的值的引用
            T& operator*()const noexcept
            {
                return static_cast<binary_tree_node<T, Alloc>&>(**ptr).data.data;
            }
            T* operator->()const noexcept
            {
                return std::addressof(static_cast<binary_tree_node<T, Alloc>&>(**ptr).data.data);
            }
            // 获取左子边（不修改自身）
            binary_tree_edge_proxy get_left()const noexcept
            {
                return { std::pointer_traits<pointer_to_pointer>::pointer_to(static_cast<binary_tree_node<T, Alloc>&>(**ptr).left) };
            }
            // 获取右子边（不修改自身）
            binary_tree_edge_proxy get_right()const noexcept
            {
                return { std::pointer_traits<pointer_to_pointer>::pointer_to(static_cast<binary_tree_node<T, Alloc>&>(**ptr).right) };
            }
            // 获取左右子边
            std::pair<binary_tree_edge_proxy, binary_tree_edge_proxy> get_children()const noexcept
            {
                return {get_left(), get_right()};
            }
            // 比较两条边是否指向同一个指针槽位
            friend bool operator==(binary_tree_edge_proxy l, binary_tree_edge_proxy r)noexcept
            {
                return l.ptr == r.ptr;
            }
            friend struct std::hash<binary_tree_edge_proxy>;
            friend class binary_tree_edge_const_proxy<T, Alloc>;
            template<class T1, class Alloc1>
            friend class Yc::binary_tree;
            friend class binary_tree_node_proxy<T, Alloc>;
            // 与迭代器约定一致：const 性由 reference 表达，value_type 不带 const
            using value_type = T;
            using reference = T&;
        };
    }
}
// std::hash 特化：哈希"槽位地址"，无效边（默认构造的代理）返回 0。
// 直接委托给缀饰指针自身的 std::hash（与 node 代理一致），不做 addressof 转换，
// 因此要求 std::hash<pointer> 可用。
template<class T, class Alloc>
struct std::hash<Yc::details::binary_tree_edge_proxy<T, Alloc>>
{
    size_t operator()(const Yc::details::binary_tree_edge_proxy<T, Alloc>& para)const noexcept
    {
        if (!para.valid())
            return 0;
        return std::hash<decltype(para.ptr)>{}(para.ptr);
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
        class binary_tree_edge_const_proxy
        {
            using pointer_to_pointer = std::allocator_traits<Alloc>::template rebind_traits<typename std::allocator_traits<Alloc>::pointer>::pointer;
            using pointer = std::allocator_traits<Alloc>::pointer;

            // ptr：指向节点中 left（或 right）成员的指针
            pointer_to_pointer ptr{};
            // 内部工厂：由"指针槽位"构造常量边代理。构造函数本身是私有的，
            // 这样 pointer_to_pointer 这个内部类型不会构成一条对外的隐式转换路径。
            static binary_tree_edge_const_proxy make(pointer_to_pointer p)noexcept
            {
                return binary_tree_edge_const_proxy{ p };
            }
        public:
            binary_tree_edge_const_proxy() = default;
            binary_tree_edge_const_proxy(const binary_tree_edge_const_proxy&) = default;
            binary_tree_edge_const_proxy(const binary_tree_edge_proxy<T, Alloc>& p)noexcept : ptr{ p.ptr }
            {
            }
            binary_tree_edge_const_proxy& operator=(const binary_tree_edge_const_proxy&) = default;
            bool valid()const noexcept
            {
                return (bool)ptr;
            }
            bool null()const noexcept
            {
                return !(bool)*ptr;
            }
            explicit operator bool()const noexcept
            {
                return valid() && (!null());
            }
            binary_tree_edge_const_proxy& go_left()noexcept
            {
                ptr = std::pointer_traits<pointer_to_pointer>::pointer_to(static_cast<binary_tree_node<T, Alloc>&>(**ptr).left);
                return *this;
            }
            binary_tree_edge_const_proxy& go_right()noexcept
            {
                ptr = std::pointer_traits<pointer_to_pointer>::pointer_to(static_cast<binary_tree_node<T, Alloc>&>(**ptr).right);
                return *this;
            }
            const T& operator*()const noexcept
            {
                return static_cast<binary_tree_node<T, Alloc>&>(**ptr).data.data;
            }
            const T* operator->()const noexcept
            {
                return std::addressof(static_cast<binary_tree_node<T, Alloc>&>(**ptr).data.data);
            }
            binary_tree_edge_const_proxy get_left()const noexcept
            {
                return make(std::pointer_traits<pointer_to_pointer>::pointer_to(static_cast<binary_tree_node<T, Alloc>&>(**ptr).left));
            }
            binary_tree_edge_const_proxy get_right()const noexcept
            {
                return make(std::pointer_traits<pointer_to_pointer>::pointer_to(static_cast<binary_tree_node<T, Alloc>&>(**ptr).right));
            }
            std::pair<binary_tree_edge_const_proxy, binary_tree_edge_const_proxy> get_children()const noexcept
            {
                return { get_left(), get_right() };
            }
            friend bool operator==(binary_tree_edge_const_proxy l, binary_tree_edge_const_proxy r)noexcept
            {
                return l.ptr == r.ptr;
            }
            friend struct std::hash<binary_tree_edge_const_proxy>;
            template<class T1, class Alloc1>
            friend class Yc::binary_tree;
            friend class binary_tree_node_const_proxy<T, Alloc>;
            // 与迭代器约定一致：const 性由 reference 表达，value_type 不带 const
            using value_type = T;
            using reference = const T&;
        };
    }
}
// std::hash 特化：哈希"槽位地址"，无效边（默认构造的代理）返回 0。
// 直接委托给缀饰指针自身的 std::hash（与 node 代理一致），不做 addressof 转换，
// 因此要求 std::hash<pointer> 可用。
template<class T, class Alloc>
struct std::hash<Yc::details::binary_tree_edge_const_proxy<T, Alloc>>
{
    size_t operator()(const Yc::details::binary_tree_edge_const_proxy<T, Alloc>& para)const noexcept
    {
        if (!para.valid())
            return 0;
        return std::hash<decltype(para.ptr)>{}(para.ptr);
    }
};
namespace Yc
{
    namespace details
    {
        // ===================================================================
        // node_proxy：直接代表树节点
        // 与 edge_proxy 不同，node_proxy 直接持有节点指针，
        // 支持 go_left/go_right 导航和值访问。
        // 注意：无父指针，故无 go_up。
        // ===================================================================
        template<class T, class Alloc>
        class binary_tree_node_proxy
        {
            using pointer = std::allocator_traits<Alloc>::pointer;
            using pointer_to_pointer = std::allocator_traits<Alloc>::template rebind_traits<typename std::allocator_traits<Alloc>::pointer>::pointer;
            // ptr：指向节点对象本身（*ptr 得到 binary_tree_empty_placeholder&）
            pointer ptr{};
        public:
            binary_tree_node_proxy() = default;
            binary_tree_node_proxy(pointer ptr)noexcept :ptr{ ptr }
            {
            }
            binary_tree_node_proxy(binary_tree_edge_proxy<T, Alloc> p)noexcept :ptr{ *p.ptr }
            {
            }
            binary_tree_node_proxy(const binary_tree_node_proxy&) = default;
            binary_tree_node_proxy& operator=(const binary_tree_node_proxy&) = default;
            bool null()const noexcept
            {
                return ptr == nullptr;
            }
            explicit operator bool()const noexcept
            {
                return !null();
            }
            T& operator*()const noexcept
            {
                return static_cast<binary_tree_node<T, Alloc>&>(*ptr).data.data;
            }
            T* operator->()const noexcept
            {
                return std::addressof(static_cast<binary_tree_node<T, Alloc>&>(*ptr).data.data);
            }
            binary_tree_node_proxy& go_left()noexcept
            {
                ptr = static_cast<binary_tree_node<T, Alloc>&>(*ptr).left;
                return *this;
            }
            binary_tree_node_proxy& go_right()noexcept
            {
                ptr = static_cast<binary_tree_node<T, Alloc>&>(*ptr).right;
                return *this;
            }
            binary_tree_node_proxy get_left()const noexcept
            {
                return { static_cast<binary_tree_node<T, Alloc>&>(*ptr).left };
            }
            binary_tree_node_proxy get_right()const noexcept
            {
                return { static_cast<binary_tree_node<T, Alloc>&>(*ptr).right };
            }
            std::pair<binary_tree_edge_proxy<T, Alloc>, binary_tree_edge_proxy<T, Alloc>> get_children()const noexcept
            {
                return
                {
                    {std::pointer_traits<pointer_to_pointer>::pointer_to(static_cast<binary_tree_node<T, Alloc>&>(*ptr).left)},
                    {std::pointer_traits<pointer_to_pointer>::pointer_to(static_cast<binary_tree_node<T, Alloc>&>(*ptr).right)}
                };
            }
            friend bool operator==(binary_tree_node_proxy l, binary_tree_node_proxy r)noexcept
            {
                return l.ptr == r.ptr;
            }
            friend struct std::hash<binary_tree_node_proxy>;
            friend class binary_tree_node_const_proxy<T, Alloc>;
            // 与迭代器约定一致：const 性由 reference 表达，value_type 不带 const
            using value_type = T;
            using reference = T&;
        };
    }
}
// std::hash 特化：基于 ptr 地址进行哈希
template<class T, class Alloc>
struct std::hash<Yc::details::binary_tree_node_proxy<T, Alloc>>
{
    size_t operator()(const Yc::details::binary_tree_node_proxy<T, Alloc>& p)const noexcept
    {
        return std::hash<decltype(p.ptr)>{}(p.ptr);
    }
};
namespace Yc
{
    namespace details
    {
        // node_const_proxy：node_proxy 的常量版本
        template<class T, class Alloc>
        class binary_tree_node_const_proxy
        {
            using pointer = std::allocator_traits<Alloc>::pointer;
            using pointer_to_pointer = std::allocator_traits<Alloc>::template rebind_traits<typename std::allocator_traits<Alloc>::pointer>::pointer;
            // ptr：指向节点对象本身
            pointer ptr{};
        public:
            binary_tree_node_const_proxy() = default;
            binary_tree_node_const_proxy(pointer ptr)noexcept :ptr{ ptr }
            {
            }
            binary_tree_node_const_proxy(binary_tree_edge_const_proxy<T, Alloc> p)noexcept :ptr{ *p.ptr }
            {
            }
            binary_tree_node_const_proxy(binary_tree_node_proxy<T, Alloc> p)noexcept :ptr{ p.ptr }
            {
            }
            binary_tree_node_const_proxy(const binary_tree_node_const_proxy&) = default;
            binary_tree_node_const_proxy& operator=(const binary_tree_node_const_proxy&) = default;
            bool null()const noexcept
            {
                return ptr == nullptr;
            }
            explicit operator bool()const noexcept
            {
                return !null();
            }
            const T& operator*()const noexcept
            {
                return static_cast<binary_tree_node<T, Alloc>&>(*ptr).data.data;
            }
            const T* operator->()const noexcept
            {
                return std::addressof(static_cast<binary_tree_node<T, Alloc>&>(*ptr).data.data);
            }
            binary_tree_node_const_proxy& go_left()noexcept
            {
                ptr = static_cast<binary_tree_node<T, Alloc>&>(*ptr).left;
                return *this;
            }
            binary_tree_node_const_proxy& go_right()noexcept
            {
                ptr = static_cast<binary_tree_node<T, Alloc>&>(*ptr).right;
                return *this;
            }
            binary_tree_node_const_proxy get_left()const noexcept
            {
                return { static_cast<binary_tree_node<T, Alloc>&>(*ptr).left };
            }
            binary_tree_node_const_proxy get_right()const noexcept
            {
                return { static_cast<binary_tree_node<T, Alloc>&>(*ptr).right };
            }
            std::pair<binary_tree_edge_const_proxy<T, Alloc>, binary_tree_edge_const_proxy<T, Alloc>> get_children()const noexcept
            {
                return
                {
                    {std::pointer_traits<pointer_to_pointer>::pointer_to(static_cast<binary_tree_node<T, Alloc>&>(*ptr).left)},
                    {std::pointer_traits<pointer_to_pointer>::pointer_to(static_cast<binary_tree_node<T, Alloc>&>(*ptr).right)}
                };
            }
            friend bool operator==(binary_tree_node_const_proxy l, binary_tree_node_const_proxy r)noexcept
            {
                return l.ptr == r.ptr;
            }
            friend struct std::hash<binary_tree_node_const_proxy>;
            // 与迭代器约定一致：const 性由 reference 表达，value_type 不带 const
            using value_type = T;
            using reference = const T&;
        };
    }
}
// std::hash 特化：基于 ptr 地址进行哈希
template<class T, class Alloc>
struct std::hash<Yc::details::binary_tree_node_const_proxy<T, Alloc>>
{
    size_t operator()(const Yc::details::binary_tree_node_const_proxy<T, Alloc>& p)const noexcept
    {
        return std::hash<decltype(p.ptr)>{}(p.ptr);
    }
};
namespace Yc
{
    namespace details
    {
        // 多态 get_children 函数对象
        struct binary_tree_get_children_t
        {
            template<class T, class Alloc>
            auto operator()(binary_tree_edge_proxy<T, Alloc> p)const noexcept
            {
                return p.get_children();
            }
            template<class T, class Alloc>
            auto operator()(binary_tree_edge_const_proxy<T, Alloc> p)const noexcept
            {
                return p.get_children();
            }
            template<class T, class Alloc>
            auto operator()(binary_tree_node_proxy<T, Alloc> p)const noexcept
            {
                return p.get_children();
            }
            template<class T, class Alloc>
            auto operator()(binary_tree_node_const_proxy<T, Alloc> p)const noexcept
            {
                return p.get_children();
            }
        };
    }
    namespace binary_tree_functional
    {
        constexpr inline Yc::details::binary_tree_get_children_t get_children{};
    }
    // 相邻情形检查掩码：与 parent_aware_binary_tree.h 共用同一份定义。
    // 两个头文件可能被同一个翻译单元同时包含，故用宏守卫避免重定义。
#ifndef YC_CHILD_CHECK_DEFINED
#define YC_CHILD_CHECK_DEFINED
    enum child_check
    {
        no_check = 0,
        check_left = 1,
        check_right = 2,
        check_left_rev = 4,
        check_right_rev = 8,
        check_all = 15, // check_all == check_left | check_right | check_left_rev | check_right_rev
    };
#endif
    // ===================================================================
    // binary_tree：无父节点指针的二叉树容器
    //
    // 与 parent_aware_binary_tree 的区别：
    //   1. 无 parent 指针，节点更小，但不能 go_up 导航
    //   2. 无哨兵节点，直接由 root_ptr（fancy pointer）持有树根
    //   3. edge_proxy 使用 pointer_to_pointer 而非成员指针
    //   4. 移动构造/赋值更简单（移动分配器 + 交换根指针）
    //
    // 主要操作：
    //   - 导航：root(), nroot()
    //   - 修改：emplace, insert, erase, cut, splice
    //   - 旋转：left_rotate, right_rotate, checked_left_rotate, checked_right_rotate
    //   - 交换：swap, swap_sub_tree, swap_node（带 child_check）
    //   - 遍历：for_each
    //   - 递归构造：recur_and_write
    // ===================================================================
    template<class T, class Alloc = std::allocator<T>>
    class binary_tree
    {
        using node_pointer = std::allocator_traits<Alloc>::template rebind_traits<details::binary_tree_empty_placeholder>::pointer;
        using pointer_to_node_pointer = std::allocator_traits<Alloc>::template rebind_traits<node_pointer>::pointer;
        using placeholder_alloc = std::allocator_traits<Alloc>::template rebind_alloc<details::binary_tree_empty_placeholder>;
        using node_type = details::binary_tree_node<T, placeholder_alloc>;
        using node_allocator = std::allocator_traits<Alloc>::template rebind_alloc<node_type>;
        // 分配器（使用 [[no_unique_address]] 以零开销存储无状态分配器）
        [[no_unique_address]] Alloc alloc{};

        // 根节点指针（无哨兵节点，直接指向树根）
        node_pointer root_ptr{};
        // 获取占位符对象的 fancy pointer
        // 获取占位符对象的 fancy pointer（与 parent_aware_binary_tree 同写法：
        // pointer_traits::pointer_to 要求非 const 引用，此处转掉 const）
        node_pointer pointer_to(const details::binary_tree_empty_placeholder& r)const noexcept
        {
            return std::pointer_traits<node_pointer>::pointer_to((details::binary_tree_empty_placeholder&)r);
        }
        // 获取节点指针的指针（即 pointer_to_pointer）
        pointer_to_node_pointer pointer_to(node_pointer& r)const noexcept
        {
            return std::pointer_traits<pointer_to_node_pointer>::pointer_to(r);
        }
        // 分配并构造一个节点
        node_pointer allocate_node()
        {
            node_allocator a{ alloc };
            node_pointer p = std::pointer_traits<node_pointer>::pointer_to
                (static_cast<details::binary_tree_empty_placeholder&>(*std::allocator_traits<node_allocator>::allocate(a, 1)));
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
        // 分配器所提供的指针类型。分配器先被 rebind 到一个空占位类型以统一管理
        // 节点内存，因此这是模板参数 Alloc（而非某次 rebind 结果）的 pointer；
        // 当它不是裸指针（fancy pointer，如 boost::interprocess::offset_ptr）时，
        // 节点所占据的存储、节点的左右 _树位置_ 与根本身都由它表示。
        using pointer = std::allocator_traits<Alloc>::pointer;
        Alloc get_allocator()const noexcept
        {
            return alloc;
        }
        using allocator_type = Alloc;
        using value_type = T;
        using reference = T&;
        using edge_proxy = details::binary_tree_edge_proxy<T, placeholder_alloc>;
        using edge_const_proxy = details::binary_tree_edge_const_proxy<T, placeholder_alloc>;
        using node_proxy = details::binary_tree_node_proxy<T, placeholder_alloc>;
        using node_const_proxy = details::binary_tree_node_const_proxy<T, placeholder_alloc>;
    private:
        // 递归写入实现（手写迭代模拟递归以避免栈溢出，使用 goto 实现尾递归）
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
        // 递归写入：从外部树形结构复制到当前树
        // 约束：h 必须可 bool 转换，且值类型可通过 emplace 构造
        template<
            class ValueGetter,
            class ChildrenGetter,
            class InitializeHandle
        >
        binary_tree recur_and_write(
            edge_const_proxy p,
            ValueGetter vg,
            ChildrenGetter cg,
            InitializeHandle h
        )
        {
            binary_tree ret{ cut(p) };
            // guard：若构造过程中异常，自动将已构造部分拼回原树
            struct _guard
            {
                binary_tree* t;
                edge_const_proxy p;
                binary_tree* r;
                ~_guard()
                {
                    if (t)
                    {
                        t->splice(p, *r);
                    }
                }
            }guard{this, p, &ret};
            if(h)
            {
                recur_and_write_impl(p, vg, cg, h);
            }
            guard.t = nullptr;
            return ret;
        }
        binary_tree() = default;
        explicit binary_tree(const Alloc& a)noexcept :alloc{a}
        {
        }
        // 从树形结构构造整棵树
        template<
            class ValueGetter,
            class ChildrenGetter,
            class InitializeHandle
        >
        binary_tree(
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
        binary_tree(const binary_tree& b) :binary_tree{ std::allocator_traits<Alloc>::select_on_container_copy_construction(b.alloc) }
        {
            auto copier = [](edge_const_proxy p) -> const T&
                {
                    return *p;
                };
            recur_and_write
                (root(), copier, binary_tree_functional::get_children, b.croot());
        }
        // 拷贝赋值
        binary_tree& operator=(const binary_tree& b)
        {
            if (this == &b)
            {
                return *this;
            }
            Alloc tmp{ alloc };
            binary_tree t1{ cut(root())};
            // guard：若复制过程异常，恢复原分配器和旧树
            struct _guard
            {
                binary_tree* t{};
                Alloc* tmp{};
                binary_tree* other{};
                ~_guard()
                {
                    if (t)
                    {
                        t->alloc = *tmp;
                        t->splice(t->root(), *other);
                    }
                }
            }guard{this, &tmp, &t1};
            if constexpr (
                std::allocator_traits<Alloc>::propagate_on_container_copy_assignment::value)
            {
                alloc = b.alloc;
            }
            auto copier = [](edge_const_proxy p)->const T&
            {
                return *p;
            };
            recur_and_write(root(), copier, binary_tree_functional::get_children, b.root());
            guard.t = nullptr;
            return *this;
        }
        // 移动构造：直接交换根指针和分配器
        binary_tree(binary_tree&& b)noexcept :
            root_ptr{ std::exchange(b.root_ptr, {}) }, alloc{ std::move(b.alloc) }
        {
        }
        // 移动赋值
        binary_tree& operator=(binary_tree&& b)noexcept(
            std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value ||
            std::allocator_traits<Alloc>::is_always_equal::value)
        {
            if constexpr
                (std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value)
            {
                // 必须写 std::swap：类内还有一个成员 swap，未限定的 swap 会先找到它
                std::swap(alloc, b.alloc);
                std::swap(root_ptr, b.root_ptr);
                return *this;
            }
            else
            {
                if (alloc == b.alloc)
                {
                    std::swap(root_ptr, b.root_ptr);
                    return *this;
                }
                auto vg = [](edge_proxy p) -> T&&
                {
                    return (T&&)*p;
                };
                binary_tree tmp{ vg, binary_tree_functional::get_children, b.root(), alloc };
                swap(tmp);
                return *this;
            }
        }
        bool empty()const noexcept
        {
            return root_ptr == nullptr;
        }
        // 获取树根的 edge_proxy（注意：无哨兵节点，空树 root_ptr 为 null）
        edge_proxy root()noexcept
        {
            return { pointer_to(root_ptr) };
        }
        edge_const_proxy root()const noexcept
        {
            // 与 croot()/parent_aware_binary_tree 一致：const 成员函数里 root_ptr 是
            // `node_pointer const`，需转掉 const 才能取到指向槽位的指针
            return { pointer_to((node_pointer&)root_ptr) };
        }
        edge_const_proxy croot()const noexcept
        {
            return { pointer_to((node_pointer&)root_ptr) };
        }
        // 获取树根对应的 node_proxy
        node_proxy nroot()noexcept
        {
            return root();
        }
        node_const_proxy nroot()const noexcept
        {
            return root();
        }
        node_const_proxy cnroot()const noexcept
        {
            return root();
        }
        // 切割：将 p 指向的子树从原树分离，返回一棵包含该子树的新树
        binary_tree cut(edge_const_proxy p)noexcept
        {
            pointer_to_node_pointer ptr = p.ptr;
            binary_tree ret{ alloc };
            ret.root_ptr = std::exchange(*ptr, {});  // 断开原树连接
            return ret;
        }
        // 删除指定边上的节点及其所有子节点（迭代式，避免递归栈溢出）
        void erase(edge_const_proxy p)noexcept
        {
            if (!p.null())
            {
                node_pointer& place = *p.ptr;
                
                while (true) {
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
                        node_pointer right = *r.ptr;
                        std::allocator_traits<Alloc>::destroy(alloc, (T*)p.operator->());
                        deallocate_node(place);
                        place = right;
                        continue;
                    }
                    if ((!l_null) && r_null)
                    {
                        node_pointer left = *l.ptr;
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

        // 中序遍历，对每个节点的值调用 f。语义与 parent_aware_binary_tree::for_each 相同，
        // 供容器在销毁节点前做收尾（例如哈希表先把所有桶摘链，再 clear() 销毁桶节点）。
        // 非 const 树传入 T&，const 树传入 const T&。
        // 与 std::for_each 一致：函数对象按值传入、按值返回，所以有状态函数对象
        // （累加器、计数器之类）遍历完能带着状态出来：
        //     auto acc = t.for_each(accumulator{});
        // 约束：f 不应改变树的结构（增删/旋转），否则遍历位置会失效。
        // 条件性 noexcept：本函数只做遍历、调用 f、再把 f 移出来，因此 noexcept 性质
        // 只取决于这两件事 —— f 能无异常接受 T&、且 f 能无异常地移动构造。
        // f 会抛时异常正常向外传播（树保持完整，因为不改结构；f 的副本随栈展开销毁）。
        template<class F>
        F for_each(F f)noexcept(std::is_nothrow_invocable_v<F&, T&>&& std::is_nothrow_move_constructible_v<F>)
        {
            for_each_impl<node_proxy>(nroot(), f);
            return f;
        }
        template<class F>
        F for_each(F f)const noexcept(std::is_nothrow_invocable_v<F&, const T&>&& std::is_nothrow_move_constructible_v<F>)
        {
            for_each_impl<node_const_proxy>(cnroot(), f);
            return f;
        }

    private:
        // 无分配的中序遍历：
        //   - 有左孩子 -> 中序要求先走完左子树再访问本节点，而"回到本节点"在没有父指针
        //                 的实现里做不到，所以这里必须递归一次；
        //   - 右子树   -> 本节点已访问，直接 goto 循环过去，不递归。
        // 于是递归深度 = 沿途有左孩子的节点数 ≤ 树高，纯右单链完全不递归。
        // （析构走的 erase() 仍是纯迭代，因此深左单链的构造与销毁都不会爆栈。）
        // noexcept 一路到底：谓词只描述"f 能否无异常接受引用"，与递归深度无关，
        // 因此本函数直接沿用与公开 for_each 相同的规格，不做自我判断。这样在 f 不抛
        // （公开入口为 noexcept）时，本函数体里也不会被生成"异常逃逸即 terminate"
        // 的捕获代码；f 会抛时规格为假，异常照常传播。
        template<class Proxy, class F>
        static void for_each_impl(Proxy p, F&& f)
            noexcept(std::is_nothrow_invocable_v<F&, decltype(*p)>)
        {
        derecur:
            if (p.null())
            {
                return;
            }
            auto [l, r] = p.get_children();
            const bool has_l = !l.null();
            const bool has_r = !r.null();
            if (has_l)
            {
                for_each_impl<Proxy>(l, f);
            }
            std::invoke(f, *p);
            if (has_r)
            {
                p = Proxy{ r };
                goto derecur;
            }
        }
    public:
        // 在指定边位置原地构造新节点（已有子树作为切割结果返回）
        template<class... Args>
        binary_tree emplace(edge_const_proxy p, Args&&... args)
        {
            node_pointer new_node = allocate_node();
            // 若 T 的构造不抛异常，直接构造；否则使用 RAII guard 确保异常安全
            if constexpr (std::is_nothrow_constructible_v<T, Args...>)
            {
                std::allocator_traits<Alloc>::
                    construct(alloc, std::addressof(((node_type&)*new_node).data.data), std::forward<Args>(args)...);
            }
            else
            {
                struct guard_t
                {
                    binary_tree* t;
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
            binary_tree ret{ cut(p) };
            *p.ptr = new_node;
            return ret;
        }
        binary_tree insert(edge_const_proxy p, const T& v)
        {
            return emplace(p, v);
        }
        binary_tree insert(edge_const_proxy p, T&& v)
        {
            return emplace(p, std::move(v));
        }
        // 拼接：将 from 子树移动到 to 位置，原 to 子树作为结果返回
        binary_tree splice(edge_const_proxy to ,edge_const_proxy from)noexcept
        {
            binary_tree ret{ cut(to) };
            *(to.ptr) = std::exchange(*(from.ptr), {});
            return ret;
        }
        // 拼接整棵树的根
        binary_tree splice(edge_const_proxy p, binary_tree& tree)noexcept
        {
            return splice(p, tree.root());
        }
        binary_tree splice(edge_const_proxy p, binary_tree&& tree)noexcept
        {
            return splice(p, tree.root());
        }

        // 交换两个子树
        static void swap_sub_tree(edge_const_proxy l, edge_const_proxy r)noexcept
        {
            std::swap(*(l.ptr), *(r.ptr));
        }

        // 交换两棵树
        void swap(binary_tree& other)noexcept
        {
            if constexpr (std::allocator_traits<Alloc>::propagate_on_container_swap::value)
            {
                // 必须写 std::swap：类内的成员 swap 会遮蔽 std::swap
                std::swap(alloc, other.alloc);
            }
            swap_sub_tree(root(), other.root());
        }
        // 析构：清空所有节点
        ~binary_tree()
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
            binary_tree t1 = cut(p.get_right()); // <D>-<rC>-<E>
            binary_tree t2 = splice(p, t1); //<B>-<rA>
            //     |
            //    <C>                 <A>
            //   /   \                /
            //  <D>  <E>            <B>
            p.go_left();
            binary_tree t3 = splice(p, t2);
            p.go_right();
            splice(p, t3);
        }
        // 右旋：左旋的镜像操作
        void right_rotate(edge_const_proxy p)noexcept
        {
            binary_tree t1 = cut(p.get_left());
            binary_tree t2 = splice(p, t1);
            p.go_right();
            binary_tree t3 = splice(p, t2);
            p.go_left();
            splice(p, t3);
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
        // 交换两个节点（同时交换其子树）
        // 与 parent_aware_binary_tree 一致地带 child_check 相邻检查：两个被交换的节点
        // 互为父子时，朴素地交换槽位会破坏结构（互指成环、丢节点），四种相邻形态各有
        // 专门的重连分支。本实现没有父指针，因此只需重连 left/right 槽位，不需要维护 parent。
        static void swap_node(edge_const_proxy l, edge_const_proxy r, child_check chk = check_all)noexcept
        {
            if (chk & check_left)
            {
                if (l.get_left() == r)
                {
                    // r 是 l 的左孩子：把 r 提到 l 的位置，l 变成 r 的左孩子
                    auto lr = l.get_right();
                    auto rl = r.get_left();
                    auto rr = r.get_right();
                    swap_sub_tree(l, r);
                    swap_sub_tree(r, rl);
                    swap_sub_tree(lr, rr);
                    return;
                }
            }
            if (chk & check_right)
            {
                if (l.get_right() == r)
                {
                    // r 是 l 的右孩子：把 r 提到 l 的位置，l 变成 r 的右孩子
                    auto ll = l.get_left();
                    auto rl = r.get_left();
                    auto rr = r.get_right();
                    swap_sub_tree(l, r);
                    swap_sub_tree(r, rr);
                    swap_sub_tree(ll, rl);
                    return;
                }
            }
            // 反转形态：先交换两个代理，即可复用上面的两段重连逻辑
            std::swap(l, r);
            if (chk & check_left_rev)
            {
                if (l.get_left() == r)
                {
                    auto lr = l.get_right();
                    auto rl = r.get_left();
                    auto rr = r.get_right();
                    swap_sub_tree(l, r);
                    swap_sub_tree(r, rl);
                    swap_sub_tree(lr, rr);
                    return;
                }
            }
            if (chk & check_right_rev)
            {
                if (l.get_right() == r)
                {
                    auto ll = l.get_left();
                    auto rl = r.get_left();
                    auto rr = r.get_right();
                    swap_sub_tree(l, r);
                    swap_sub_tree(r, rr);
                    swap_sub_tree(ll, rl);
                    return;
                }
            }
            // 两个节点不相邻（或调用方显式传 no_check）：朴素交换槽位 + 各自子槽位
            swap_sub_tree(l, r);
            auto [ll, lr] = l.get_children();
            auto [rl, rr] = r.get_children();
            swap_sub_tree(ll, rl);
            swap_sub_tree(lr, rr);
        }
    };
    // 特化 swap 自由函数
    template<class T, class Alloc>
    void swap(binary_tree<T, Alloc>& l, binary_tree<T, Alloc>& r)noexcept
    {
        l.swap(r);
    }
}
