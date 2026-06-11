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
            template<class T1, class Alloc>
            friend class Yc::binary_tree;
            friend class binary_tree_node_proxy<T, Alloc>;
            using value_type = T;
        };
    }
}
// std::hash 特化：基于 ptr 地址进行哈希
template<class T, class Alloc>
struct std::hash<Yc::details::binary_tree_edge_proxy<T, Alloc>>
{
    size_t operator()(const Yc::details::binary_tree_edge_proxy<T, Alloc>& para)const noexcept
    {
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
        public:
            binary_tree_edge_const_proxy() = default;
            binary_tree_edge_const_proxy(const binary_tree_edge_const_proxy&) = default;
            binary_tree_edge_const_proxy(const binary_tree_edge_proxy<T, Alloc>& p)noexcept : ptr{ p.ptr }
            {
            }
            binary_tree_edge_const_proxy(pointer_to_pointer ptr)noexcept :ptr{ ptr }
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
                return { std::pointer_traits<pointer_to_pointer>::pointer_to(static_cast<binary_tree_node<T, Alloc>&>(**ptr).left) };
            }
            binary_tree_edge_const_proxy get_right()const noexcept
            {
                return { std::pointer_traits<pointer_to_pointer>::pointer_to(static_cast<binary_tree_node<T, Alloc>&>(**ptr).right) };
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
            template<class T1, class Alloc>
            friend class Yc::binary_tree;
            friend class binary_tree_node_const_proxy<T, Alloc>;
            using value_type = const T;
        };
    }
}
// std::hash 特化：基于 ptr 地址进行哈希
template<class T, class Alloc>
struct std::hash<Yc::details::binary_tree_edge_const_proxy<T, Alloc>>
{
    size_t operator()(const Yc::details::binary_tree_edge_const_proxy<T, Alloc>& para)const noexcept
    {
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
            std::pair<binary_tree_edge_proxy<T, Alloc>, binary_tree_edge_proxy<T, Alloc>> get_children()const
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
            using value_type = T;
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
            std::pair<binary_tree_edge_const_proxy<T, Alloc>, binary_tree_edge_const_proxy<T, Alloc>> get_children()const
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
            using value_type = const T;
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
    // ===================================================================
    // binary_tree：无父节点指针的二叉树容器
    //
    // 与 parent_aware_binary_tree 的区别：
    //   1. 无 parent 指针，节点更小，但不能 go_up 导航
    //   2. 无哨兵节点，直接使用 root_ptr 裸指针
    //   3. edge_proxy 使用 pointer_to_pointer 而非成员指针
    //   4. 移动构造/赋值更简单（直接交换指针）
    //
    // 主要操作：
    //   - 导航：root(), nroot()
    //   - 修改：emplace, insert, erase, cut, splice
    //   - 旋转：left_rotate, right_rotate, checked_left_rotate, checked_right_rotate
    //   - 交换：swap, swap_sub_tree, swap_node
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
        node_pointer pointer_to(const details::binary_tree_empty_placeholder& r)const noexcept
        {
            return std::pointer_traits<node_pointer>::pointer_to(r);
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
                *((int*)nullptr);
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
    //    template<
    //        class ValueGetter,
    //        class ChildrenGetter,
    //        class InitializeHandle
    //    >
    //    void recur_and_overwrite_impl(
    //        edge_const_proxy p,
    //        ValueGetter& vg,
    //        ChildrenGetter& cg,
    //        InitializeHandle h
    //    )
    //    {
    //        derecur:
    //        if (!h)
    //        {
    //            erase(p);
    //            return;
    //        }
    //        if (p)
    //        {
    //            T& t = (T&)*p;
    //            if constexpr (requires{
    //                t = std::invoke(vg, h);
    //            } && noexcept(t = std::invoke(vg, h)))
    //            {
    //                t = std::invoke(vg, h);
    //            }
    //            else if constexpr (requires{
    //                t = T{ std::invoke(vg,h) };
    //            } && noexcept(t = T{ std::invoke(vg,h) }))
    //            {
    //                t = T{ std::invoke(vg,h) };
    //            }
    //            else
    //            {
    //                std::allocator_traits<Alloc>::destroy(alloc, (T*)p.operator->());
    //                static_assert(std::is_nothrow_constructible_v<T, decltype(std::invoke(vg, h))>);
    //                std::allocator_traits<Alloc>::construct(alloc, (T*)p.operator->(), std::invoke(vg, h));
    //            }
    //        }
    //        else
    //        {
    //            emplace(p, std::invoke(vg, h));
    //        }
    //        auto [lh, rh] = std::invoke(cg, h);
    //        auto [l, r] = p.get_children();
    //        bool bl = (bool)lh, br = (bool)rh;
    //        if (!(bl && br))
    //        {
    //            if (bl)
    //            {
    //                p = l;
    //                h = lh;
    //                goto derecur;
    //            }
    //            if (br)
    //            {
    //            tmp:
    //                p = r;
    //                h = rh;
    //                goto derecur;
    //            }
    //            return;
    //        }
    //        recur_and_overwrite_impl(l, vg, cg, lh);
    //        goto tmp;
    //    }
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
                        splice(root(), *other);
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
            recur_and_write(root(), copier, &edge_const_proxy::get_children, b.root());
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
                using namespace std;
                swap(alloc, b.alloc);
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
            return { pointer_to(root_ptr) };
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
                using namespace std;
                swap(alloc, other.alloc);
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
        static void swap_node(edge_const_proxy l, edge_const_proxy r)
        {
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
