// 二叉树库（无父节点指针版本）
// 与 parent_aware_binary_tree 相比，本实现不维护父节点指针，
// 因此更轻量，但无法进行 go_up 导航和某些需要父指针的操作。
//
// 分层：
//   - intrusive_binary_tree.h 提供侵入式二叉树：节点钩子、槽位、旋转、
//     交换、切割/拼接、拆树等纯结构操作；它不接触 T，也不分配内存，
//     公开接口只使用 edge_proxy / node_proxy，不暴露内部指针类型。
//   - 本文件是"拥有型适配器"：binary_tree 内部持有一棵侵入式树，自己负责
//     节点内存的分配、T 的构造/析构与回收；公开的 edge_proxy / node_proxy
//     在侵入式 proxy 外再包一层，提供 T 的解引用。
#pragma once

// MSVC 不支持标准 [[no_unique_address]]，要用它的 [[msvc::no_unique_address]]
#ifndef YC_NO_UNIQUE_ADDRESS
#if defined(_MSC_VER)
#define YC_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
#define YC_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif
#endif

#include <memory>
#include <type_traits>
#include <functional>
#include <utility>
#include "intrusive_binary_tree.h"
#include "union_data.h"

namespace Yc
{
    template<class T, class Alloc>
    class binary_tree;
    namespace details
    {
        // 保留旧的内部名字；新的节点钩子由侵入式层提供。
        struct binary_tree_empty_placeholder
        { };
        // 完整节点：继承侵入式钩子并附加用户数据
        template<class T, class vptr>
        struct binary_tree_node : intrusive_binary_tree_node_base<vptr>
        {
            data_t<T> data;
        };

        template<class T, class vptr>
        class binary_tree_edge_const_proxy;
        template<class T, class vptr>
        class binary_tree_node_proxy;
        template<class T, class vptr>
        class binary_tree_node_const_proxy;

        // ===================================================================
        // edge_proxy：包装侵入式边 proxy，并补上 T 的解引用。
        // 指针 / 槽位类型完全留在侵入式层，不出现在公开签名中。
        // ===================================================================
        template<class T, class vptr>
        class binary_tree_edge_proxy
        {
            using intrusive_edge = intrusive_binary_tree_edge_proxy<vptr>;
            using node_type = binary_tree_node<T, vptr>;

            intrusive_edge e_{};
        private:
            binary_tree_edge_proxy(intrusive_edge e)noexcept : e_{ e }
            {
            }
            intrusive_edge intrusive()const noexcept
            {
                return e_;
            }
        public:
            binary_tree_edge_proxy() = default;
            bool valid()const noexcept
            {
                return e_.valid();
            }
            bool null()const noexcept
            {
                return e_.null();
            }
            explicit operator bool()const noexcept
            {
                return static_cast<bool>(e_);
            }
            binary_tree_edge_proxy& go_left()noexcept
            {
                e_.go_left();
                return *this;
            }
            binary_tree_edge_proxy& go_right()noexcept
            {
                e_.go_right();
                return *this;
            }
            T& operator*()const noexcept
            {
                return static_cast<node_type&>(*e_).data.data;
            }
            T* operator->()const noexcept
            {
                return std::addressof(static_cast<node_type&>(*e_).data.data);
            }
            binary_tree_edge_proxy get_left()const noexcept
            {
                return { e_.get_left() };
            }
            binary_tree_edge_proxy get_right()const noexcept
            {
                return { e_.get_right() };
            }
            std::pair<binary_tree_edge_proxy, binary_tree_edge_proxy> get_children()const noexcept
            {
                return { get_left(), get_right() };
            }
            friend bool operator==(binary_tree_edge_proxy l, binary_tree_edge_proxy r)noexcept
            {
                return l.e_ == r.e_;
            }
            friend struct std::hash<binary_tree_edge_proxy>;
            friend class binary_tree_edge_const_proxy<T, vptr>;
            friend class binary_tree_node_proxy<T, vptr>;
            template<class T1, class Alloc1>
            friend class Yc::binary_tree;
            using value_type = T;
            using reference = T&;
        };
    }
}

template<class T, class vptr>
struct std::hash<Yc::details::binary_tree_edge_proxy<T, vptr>>
{
    size_t operator()(const Yc::details::binary_tree_edge_proxy<T, vptr>& p)const noexcept
    {
        if (!p.valid())
        {
            return 0;
        }
        return std::hash<Yc::intrusive_binary_tree_edge_proxy<vptr>>{}(p.intrusive());
    }
};

namespace Yc
{
    namespace details
    {
        // edge_const_proxy：edge_proxy 的常量版本
        template<class T, class vptr>
        class binary_tree_edge_const_proxy
        {
            using intrusive_edge = intrusive_binary_tree_edge_proxy<vptr>;
            using node_type = binary_tree_node<T, vptr>;

            intrusive_edge e_{};
            static binary_tree_edge_const_proxy make(intrusive_edge e)noexcept
            {
                return binary_tree_edge_const_proxy{ e };
            }
            binary_tree_edge_const_proxy(intrusive_edge e)noexcept : e_{ e }
            {
            }
            intrusive_edge intrusive()const noexcept
            {
                return e_;
            }
        public:
            binary_tree_edge_const_proxy() = default;
            binary_tree_edge_const_proxy(binary_tree_edge_proxy<T, vptr> p)noexcept
                : e_{ p.intrusive() }
            {
            }
            bool valid()const noexcept
            {
                return e_.valid();
            }
            bool null()const noexcept
            {
                return e_.null();
            }
            explicit operator bool()const noexcept
            {
                return static_cast<bool>(e_);
            }
            binary_tree_edge_const_proxy& go_left()noexcept
            {
                e_.go_left();
                return *this;
            }
            binary_tree_edge_const_proxy& go_right()noexcept
            {
                e_.go_right();
                return *this;
            }
            const T& operator*()const noexcept
            {
                return static_cast<node_type&>(*e_).data.data;
            }
            const T* operator->()const noexcept
            {
                return std::addressof(static_cast<node_type&>(*e_).data.data);
            }
            binary_tree_edge_const_proxy get_left()const noexcept
            {
                return make(e_.get_left());
            }
            binary_tree_edge_const_proxy get_right()const noexcept
            {
                return make(e_.get_right());
            }
            std::pair<binary_tree_edge_const_proxy, binary_tree_edge_const_proxy> get_children()const noexcept
            {
                return { get_left(), get_right() };
            }
            friend bool operator==(binary_tree_edge_const_proxy l, binary_tree_edge_const_proxy r)noexcept
            {
                return l.e_ == r.e_;
            }
            friend struct std::hash<binary_tree_edge_const_proxy>;
            friend class binary_tree_node_const_proxy<T, vptr>;
            template<class T1, class Alloc1>
            friend class Yc::binary_tree;
            using value_type = T;
            using reference = const T&;
        };
    }
}

template<class T, class vptr>
struct std::hash<Yc::details::binary_tree_edge_const_proxy<T, vptr>>
{
    size_t operator()(const Yc::details::binary_tree_edge_const_proxy<T, vptr>& p)const noexcept
    {
        if (!p.valid())
        {
            return 0;
        }
        return std::hash<Yc::intrusive_binary_tree_edge_proxy<vptr>>{}(p.intrusive());
    }
};

namespace Yc
{
    namespace details
    {
        // node_proxy：包装侵入式节点 proxy，并补上 T 的解引用。
        template<class T, class vptr>
        class binary_tree_node_proxy
        {
            using intrusive_node = intrusive_binary_tree_node_proxy<vptr>;
            using node_type = binary_tree_node<T, vptr>;

            intrusive_node n_{};
        private:
            binary_tree_node_proxy(intrusive_node n)noexcept : n_{ n }
            {
            }
            intrusive_node intrusive()const noexcept
            {
                return n_;
            }
        public:
            binary_tree_node_proxy() = default;
            binary_tree_node_proxy(binary_tree_edge_proxy<T, vptr> p)noexcept
                : n_{ p.intrusive() }
            {
            }
            bool null()const noexcept
            {
                return n_.null();
            }
            explicit operator bool()const noexcept
            {
                return static_cast<bool>(n_);
            }
            T& operator*()const noexcept
            {
                return static_cast<node_type&>(*n_).data.data;
            }
            T* operator->()const noexcept
            {
                return std::addressof(static_cast<node_type&>(*n_).data.data);
            }
            binary_tree_node_proxy& go_left()noexcept
            {
                n_.go_left();
                return *this;
            }
            binary_tree_node_proxy& go_right()noexcept
            {
                n_.go_right();
                return *this;
            }
            binary_tree_node_proxy get_left()const noexcept
            {
                return { n_.get_left() };
            }
            binary_tree_node_proxy get_right()const noexcept
            {
                return { n_.get_right() };
            }
            std::pair<binary_tree_edge_proxy<T, vptr>, binary_tree_edge_proxy<T, vptr>> get_children()const noexcept
            {
                return { binary_tree_edge_proxy<T, vptr>{
                             intrusive_binary_tree_edge_proxy<vptr>{ n_, true } },
                         binary_tree_edge_proxy<T, vptr>{
                             intrusive_binary_tree_edge_proxy<vptr>{ n_, false } } };
            }
            friend bool operator==(binary_tree_node_proxy l, binary_tree_node_proxy r)noexcept
            {
                return l.n_ == r.n_;
            }
            friend struct std::hash<binary_tree_node_proxy>;
            friend class binary_tree_node_const_proxy<T, vptr>;
            template<class T1, class Alloc1>
            friend class Yc::binary_tree;
            using value_type = T;
            using reference = T&;
        };
    }
}

template<class T, class vptr>
struct std::hash<Yc::details::binary_tree_node_proxy<T, vptr>>
{
    size_t operator()(const Yc::details::binary_tree_node_proxy<T, vptr>& p)const noexcept
    {
        if (!p)
        {
            return 0;
        }
        return std::hash<Yc::intrusive_binary_tree_node_proxy<vptr>>{}(p.intrusive());
    }
};

namespace Yc
{
    namespace details
    {
        // node_const_proxy：node_proxy 的常量版本
        template<class T, class vptr>
        class binary_tree_node_const_proxy
        {
            using intrusive_node = intrusive_binary_tree_node_proxy<vptr>;
            using node_type = binary_tree_node<T, vptr>;

            intrusive_node n_{};
        private:
            binary_tree_node_const_proxy(intrusive_node n)noexcept : n_{ n }
            {
            }
            intrusive_node intrusive()const noexcept
            {
                return n_;
            }
        public:
            binary_tree_node_const_proxy() = default;
            binary_tree_node_const_proxy(binary_tree_edge_const_proxy<T, vptr> p)noexcept
                : n_{ p.intrusive() }
            {
            }
            binary_tree_node_const_proxy(binary_tree_node_proxy<T, vptr> p)noexcept
                : n_{ p.intrusive() }
            {
            }
            bool null()const noexcept
            {
                return n_.null();
            }
            explicit operator bool()const noexcept
            {
                return static_cast<bool>(n_);
            }
            const T& operator*()const noexcept
            {
                return static_cast<node_type&>(*n_).data.data;
            }
            const T* operator->()const noexcept
            {
                return std::addressof(static_cast<node_type&>(*n_).data.data);
            }
            binary_tree_node_const_proxy& go_left()noexcept
            {
                n_.go_left();
                return *this;
            }
            binary_tree_node_const_proxy& go_right()noexcept
            {
                n_.go_right();
                return *this;
            }
            binary_tree_node_const_proxy get_left()const noexcept
            {
                return { n_.get_left() };
            }
            binary_tree_node_const_proxy get_right()const noexcept
            {
                return { n_.get_right() };
            }
            std::pair<binary_tree_edge_const_proxy<T, vptr>, binary_tree_edge_const_proxy<T, vptr>> get_children()const noexcept
            {
                return { binary_tree_edge_const_proxy<T, vptr>{
                             intrusive_binary_tree_edge_proxy<vptr>{ n_, true } },
                         binary_tree_edge_const_proxy<T, vptr>{
                             intrusive_binary_tree_edge_proxy<vptr>{ n_, false } } };
            }
            friend bool operator==(binary_tree_node_const_proxy l, binary_tree_node_const_proxy r)noexcept
            {
                return l.n_ == r.n_;
            }
            friend struct std::hash<binary_tree_node_const_proxy>;
            template<class T1, class Alloc1>
            friend class Yc::binary_tree;
            using value_type = T;
            using reference = const T&;
        };
    }
}

template<class T, class vptr>
struct std::hash<Yc::details::binary_tree_node_const_proxy<T, vptr>>
{
    size_t operator()(const Yc::details::binary_tree_node_const_proxy<T, vptr>& p)const noexcept
    {
        if (!p)
        {
            return 0;
        }
        return std::hash<Yc::intrusive_binary_tree_node_proxy<vptr>>{}(p.intrusive());
    }
};

namespace Yc
{
    namespace details
    {
        // 多态 get_children 函数对象
        struct binary_tree_get_children_t
        {
            template<class T, class vptr>
            auto operator()(binary_tree_edge_proxy<T, vptr> p)const noexcept
            {
                return p.get_children();
            }
            template<class T, class vptr>
            auto operator()(binary_tree_edge_const_proxy<T, vptr> p)const noexcept
            {
                return p.get_children();
            }
            template<class T, class vptr>
            auto operator()(binary_tree_node_proxy<T, vptr> p)const noexcept
            {
                return p.get_children();
            }
            template<class T, class vptr>
            auto operator()(binary_tree_node_const_proxy<T, vptr> p)const noexcept
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
    // binary_tree：持有侵入式二叉树的拥有型适配器
    //
    // 与 parent_aware_binary_tree 的区别：
    //   1. 无 parent 指针，节点更小，但不能 go_up 导航
    //   2. 无哨兵节点，直接由侵入式树的根槽位持有树根
    //   3. edge_proxy 由侵入式边 proxy 包装，T 的解引用在外层完成
    //   4. 移动构造/赋值更简单（移动分配器 + 搬移侵入式树）
    //
    // 生命周期：节点由本类分配/构造，erase/clear 时由本类析构 T 并回收；
    // 侵入式树只负责把节点从结构上摘下来，从不 delete 节点。
    // ===================================================================
    template<class T, class Alloc = std::allocator<T>>
    class binary_tree
    {
        // 结构指针的"种子"类型：普通分配器是 T*，缀饰指针分配器是 fancy<T>。
        using vptr = typename std::allocator_traits<Alloc>::pointer;
        using intrusive_tree_type = basic_intrusive_binary_tree<vptr>;
        using node_base = intrusive_binary_tree_node_base<vptr>;
        using node_type = details::binary_tree_node<T, vptr>;
        using node_allocator = std::allocator_traits<Alloc>::template rebind_alloc<node_type>;

        // 分配器（使用 YC_NO_UNIQUE_ADDRESS 以零开销存储无状态分配器）
        YC_NO_UNIQUE_ADDRESS Alloc alloc{};
        // 侵入式二叉树：根槽位 + 全部结构算法，不拥有节点
        intrusive_tree_type tree_{};

        // 分配并构造一个节点（T 由 emplace 单独构造），返回节点基类引用
        node_base& allocate_node()
        {
            node_allocator a{ alloc };
            auto raw = std::allocator_traits<node_allocator>::allocate(a, 1);
            node_type* p = std::to_address(raw);
            std::allocator_traits<node_allocator>::construct(a, p);
            return static_cast<node_base&>(*p);
        }
        // 析构并释放节点
        void deallocate_node(node_base& n)noexcept
        {
            node_type* raw = static_cast<node_type*>(&n);
            node_allocator a{ alloc };
            std::allocator_traits<node_allocator>::destroy(a, raw);
            std::allocator_traits<node_allocator>::deallocate(a,
                std::pointer_traits<typename std::allocator_traits<node_allocator>::pointer>::
                pointer_to(*raw), 1);
        }
    public:
        // 分配器所提供的指针类型。
        using pointer = std::allocator_traits<Alloc>::pointer;
        Alloc get_allocator()const noexcept
        {
            return alloc;
        }
        using allocator_type = Alloc;
        using value_type = T;
        using reference = T&;
        using edge_proxy = details::binary_tree_edge_proxy<T, vptr>;
        using edge_const_proxy = details::binary_tree_edge_const_proxy<T, vptr>;
        using node_proxy = details::binary_tree_node_proxy<T, vptr>;
        using node_const_proxy = details::binary_tree_node_const_proxy<T, vptr>;
    private:
        // 递归写入实现（使用 goto 实现尾递归）
        template<class ValueGetter, class ChildrenGetter, class InitializeHandle>
        void recur_and_write_impl(edge_const_proxy p, ValueGetter& vg,
            ChildrenGetter& cg, InitializeHandle h)
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
        template<class ValueGetter, class ChildrenGetter, class InitializeHandle>
        binary_tree recur_and_write(edge_const_proxy p, ValueGetter vg,
            ChildrenGetter cg, InitializeHandle h)
        {
            binary_tree ret{ cut(p) };
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
            }guard{ this, p, &ret };
            if (h)
            {
                recur_and_write_impl(p, vg, cg, h);
            }
            guard.t = nullptr;
            return ret;
        }
        binary_tree() = default;
        explicit binary_tree(const Alloc& a)noexcept :alloc{ a }
        {
        }
        template<class ValueGetter, class ChildrenGetter, class InitializeHandle>
        binary_tree(ValueGetter vg, ChildrenGetter cg, InitializeHandle h,
            const Alloc& a = {}) :binary_tree(a)
        {
            if (h)
            {
                recur_and_write_impl(root(), vg, cg, h);
            }
        }
        // 拷贝构造：递归复制整棵树的结构和值
        binary_tree(const binary_tree& b) :binary_tree{
            std::allocator_traits<Alloc>::select_on_container_copy_construction(b.alloc) }
        {
            auto copier = [](edge_const_proxy p) -> const T&
            {
                return *p;
            };
            recur_and_write(root(), copier, binary_tree_functional::get_children, b.croot());
        }
        // 拷贝赋值
        binary_tree& operator=(const binary_tree& b)
        {
            if (this == &b)
            {
                return *this;
            }
            Alloc tmp{ alloc };
            binary_tree t1{ cut(root()) };
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
            }guard{ this, &tmp, &t1 };
            if constexpr (std::allocator_traits<Alloc>::propagate_on_container_copy_assignment::value)
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
        // 移动构造：搬走侵入式树（O(1)）和分配器
        binary_tree(binary_tree&& b)noexcept :
            alloc{ std::move(b.alloc) }, tree_{ std::move(b.tree_) }
        {
        }
        // 移动赋值
        binary_tree& operator=(binary_tree&& b)noexcept(
            std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value ||
            std::allocator_traits<Alloc>::is_always_equal::value)
        {
            if constexpr (std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value)
            {
                std::swap(alloc, b.alloc);
                tree_.swap(b.tree_);
                return *this;
            }
            else
            {
                if (alloc == b.alloc)
                {
                    tree_.swap(b.tree_);
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
            return tree_.empty();
        }
        edge_proxy root()noexcept
        {
            return edge_proxy{ tree_.root() };
        }
        edge_const_proxy root()const noexcept
        {
            return edge_const_proxy{ const_cast<binary_tree*>(this)->tree_.root() };
        }
        edge_const_proxy croot()const noexcept
        {
            return edge_const_proxy{ const_cast<binary_tree*>(this)->tree_.root() };
        }
        node_proxy nroot()noexcept
        {
            return node_proxy{ tree_.nroot() };
        }
        node_const_proxy nroot()const noexcept
        {
            return node_const_proxy{ const_cast<binary_tree*>(this)->tree_.nroot() };
        }
        node_const_proxy cnroot()const noexcept
        {
            return node_const_proxy{ const_cast<binary_tree*>(this)->tree_.nroot() };
        }
        // 切割：将 p 指向的子树从原树分离，返回一棵包含该子树的新树
        binary_tree cut(edge_const_proxy p)noexcept
        {
            binary_tree ret{ alloc };
            ret.tree_ = tree_.cut(p.intrusive());
            return ret;
        }
        // 删除指定边上的节点及其所有子节点
        void erase(edge_const_proxy p)noexcept
        {
            tree_.erase(p.intrusive(), [this](auto np)noexcept
            {
                node_base& n = *np;
                std::allocator_traits<Alloc>::destroy(alloc,
                    std::addressof(static_cast<node_type&>(n).data.data));
                deallocate_node(n);
            });
        }
        void clear()noexcept
        {
            erase(root());
        }

        // 中序遍历，对每个节点的值调用 f；函数对象按值传入、按值返回。
        template<class F>
        F for_each(F f)noexcept(std::is_nothrow_invocable_v<F&, T&>&&
            std::is_nothrow_move_constructible_v<F>)
        {
            for_each_impl<node_proxy>(nroot(), f);
            return f;
        }
        template<class F>
        F for_each(F f)const noexcept(std::is_nothrow_invocable_v<F&, const T&>&&
            std::is_nothrow_move_constructible_v<F>)
        {
            for_each_impl<node_const_proxy>(cnroot(), f);
            return f;
        }

    private:
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
            node_base& new_node = allocate_node();
            if constexpr (std::is_nothrow_constructible_v<T, Args...>)
            {
                std::allocator_traits<Alloc>::construct(alloc,
                    std::addressof(static_cast<node_type&>(new_node).data.data),
                    std::forward<Args>(args)...);
            }
            else
            {
                struct guard_t
                {
                    binary_tree* t;
                    node_base* ptr;
                    ~guard_t()
                    {
                        if (ptr != nullptr)
                        {
                            t->deallocate_node(*ptr);
                        }
                    }
                }guard{ this, &new_node };
                std::allocator_traits<Alloc>::construct(alloc,
                    std::addressof(static_cast<node_type&>(new_node).data.data),
                    std::forward<Args>(args)...);
                guard.ptr = nullptr;
            }
            binary_tree ret{ alloc };
            ret.tree_ = intrusive_tree_type::insert(p.intrusive(), new_node);
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
        binary_tree splice(edge_const_proxy to, edge_const_proxy from)noexcept
        {
            binary_tree ret{ alloc };
            ret.tree_ = intrusive_tree_type::splice(
                to.intrusive(), from.intrusive());
            return ret;
        }
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
            intrusive_tree_type::swap_sub_tree(l.intrusive(), r.intrusive());
        }
        // 交换两棵树
        void swap(binary_tree& other)noexcept
        {
            if constexpr (std::allocator_traits<Alloc>::propagate_on_container_swap::value)
            {
                std::swap(alloc, other.alloc);
            }
            tree_.swap(other.tree_);
        }
        ~binary_tree()
        {
            clear();
        }
        // 左旋：将 p 的右子节点提升为 p 的位置
        void left_rotate(edge_const_proxy p)noexcept
        {
            intrusive_tree_type::left_rotate(p.intrusive());
        }
        void right_rotate(edge_const_proxy p)noexcept
        {
            intrusive_tree_type::right_rotate(p.intrusive());
        }
        bool checked_left_rotate(edge_const_proxy p)noexcept
        {
            return intrusive_tree_type::checked_left_rotate(p.intrusive());
        }
        bool checked_right_rotate(edge_const_proxy p)noexcept
        {
            return intrusive_tree_type::checked_right_rotate(p.intrusive());
        }
        // 交换两个节点：净效果是"两个位置上的值互换、子树留在原位"
        static void swap_node(edge_const_proxy l, edge_const_proxy r)noexcept
        {
            intrusive_tree_type::swap_node(l.intrusive(), r.intrusive());
        }
    };
    // 特化 swap 自由函数
    template<class T, class Alloc>
    void swap(binary_tree<T, Alloc>& l, binary_tree<T, Alloc>& r)noexcept
    {
        l.swap(r);
    }
}
