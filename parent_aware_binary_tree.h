// 带父节点指针的二叉树库
//
// 分层：
//   - intrusive_binary_tree.h 提供侵入式父指针二叉树：节点钩子、只含 left
//     的哨兵、parent 链维护、旋转、交换、切割/拼接、拆树等纯结构操作；
//     它不接触 T，也不分配内存，公开接口只使用 edge_proxy / node_proxy。
//   - 本文件是"拥有型适配器"：parent_aware_binary_tree 内部持有一棵侵入式
//     父指针树，自己负责节点内存的分配、T 的构造/析构与回收；公开 proxy
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
    class parent_aware_binary_tree;
    namespace details
    {
        template<class T, class vptr>
        struct parent_aware_binary_tree_node
            : intrusive_parent_aware_binary_tree_node_base<vptr>
        {
            data_t<T> data;
        };

        template<class T, class vptr>
        class parent_aware_binary_tree_edge_const_proxy;
        template<class T, class vptr>
        class parent_aware_binary_tree_node_proxy;
        template<class T, class vptr>
        class parent_aware_binary_tree_node_const_proxy;

        // ===================================================================
        // edge_proxy：包装侵入式父指针边 proxy，并补上 T 的解引用。
        // ===================================================================
        template<class T, class vptr>
        class parent_aware_binary_tree_edge_proxy
        {
            using intrusive_edge = intrusive_parent_aware_binary_tree_edge_proxy<vptr>;
            using node_type = parent_aware_binary_tree_node<T, vptr>;

            intrusive_edge e_{};
        private:
            parent_aware_binary_tree_edge_proxy(intrusive_edge e)noexcept : e_{ e }
            {
            }
            intrusive_edge intrusive()const noexcept
            {
                return e_;
            }
        public:
            parent_aware_binary_tree_edge_proxy() = default;
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
            parent_aware_binary_tree_edge_proxy& go_left()noexcept
            {
                e_.go_left();
                return *this;
            }
            parent_aware_binary_tree_edge_proxy& go_right()noexcept
            {
                e_.go_right();
                return *this;
            }
            parent_aware_binary_tree_edge_proxy& go_up()noexcept
            {
                e_.go_up();
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
            parent_aware_binary_tree_edge_proxy get_left()const noexcept
            {
                return { e_.get_left() };
            }
            parent_aware_binary_tree_edge_proxy get_right()const noexcept
            {
                return { e_.get_right() };
            }
            parent_aware_binary_tree_edge_proxy get_parent()const noexcept
            {
                return { e_.get_parent() };
            }
            std::pair<parent_aware_binary_tree_edge_proxy,
                parent_aware_binary_tree_edge_proxy> get_children()const noexcept
            {
                return { get_left(), get_right() };
            }
            friend bool operator==(parent_aware_binary_tree_edge_proxy,
                parent_aware_binary_tree_edge_proxy)noexcept = default;
            friend struct std::hash<parent_aware_binary_tree_edge_proxy>;
            friend class parent_aware_binary_tree_edge_const_proxy<T, vptr>;
            friend class parent_aware_binary_tree_node_proxy<T, vptr>;
            template<class T1, class Alloc1>
            friend class Yc::parent_aware_binary_tree;
            using value_type = T;
            using reference = T&;
        };
    }
}

template<class T, class vptr>
struct std::hash<Yc::details::parent_aware_binary_tree_edge_proxy<T, vptr>>
{
    size_t operator()(const Yc::details::parent_aware_binary_tree_edge_proxy<T, vptr>& p)const noexcept
    {
        if (!p.valid())
        {
            return 0;
        }
        return std::hash<Yc::intrusive_parent_aware_binary_tree_edge_proxy<vptr>>{}(p.intrusive());
    }
};

namespace Yc
{
    namespace details
    {
        // edge_const_proxy：edge_proxy 的常量版本
        template<class T, class vptr>
        class parent_aware_binary_tree_edge_const_proxy
        {
            using intrusive_edge = intrusive_parent_aware_binary_tree_edge_proxy<vptr>;
            using node_type = parent_aware_binary_tree_node<T, vptr>;

            intrusive_edge e_{};
        private:
            parent_aware_binary_tree_edge_const_proxy(intrusive_edge e)noexcept : e_{ e }
            {
            }
            intrusive_edge intrusive()const noexcept
            {
                return e_;
            }
        public:
            parent_aware_binary_tree_edge_const_proxy() = default;
            parent_aware_binary_tree_edge_const_proxy(
                parent_aware_binary_tree_edge_proxy<T, vptr> p)noexcept
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
            parent_aware_binary_tree_edge_const_proxy& go_left()noexcept
            {
                e_.go_left();
                return *this;
            }
            parent_aware_binary_tree_edge_const_proxy& go_right()noexcept
            {
                e_.go_right();
                return *this;
            }
            parent_aware_binary_tree_edge_const_proxy& go_up()noexcept
            {
                e_.go_up();
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
            parent_aware_binary_tree_edge_const_proxy get_left()const noexcept
            {
                return { e_.get_left() };
            }
            parent_aware_binary_tree_edge_const_proxy get_right()const noexcept
            {
                return { e_.get_right() };
            }
            parent_aware_binary_tree_edge_const_proxy get_parent()const noexcept
            {
                return { e_.get_parent() };
            }
            std::pair<parent_aware_binary_tree_edge_const_proxy,
                parent_aware_binary_tree_edge_const_proxy> get_children()const noexcept
            {
                return { get_left(), get_right() };
            }
            friend bool operator==(parent_aware_binary_tree_edge_const_proxy,
                parent_aware_binary_tree_edge_const_proxy)noexcept = default;
            friend struct std::hash<parent_aware_binary_tree_edge_const_proxy>;
            friend class parent_aware_binary_tree_node_const_proxy<T, vptr>;
            template<class T1, class Alloc1>
            friend class Yc::parent_aware_binary_tree;
            using value_type = T;
            using reference = const T&;
        };
    }
}

template<class T, class vptr>
struct std::hash<Yc::details::parent_aware_binary_tree_edge_const_proxy<T, vptr>>
{
    size_t operator()(const Yc::details::parent_aware_binary_tree_edge_const_proxy<T, vptr>& p)const noexcept
    {
        if (!p.valid())
        {
            return 0;
        }
        return std::hash<Yc::intrusive_parent_aware_binary_tree_edge_proxy<vptr>>{}(p.intrusive());
    }
};

namespace Yc
{
    namespace details
    {
        // node_proxy：包装侵入式节点 proxy，并补上 T 的解引用。
        template<class T, class vptr>
        class parent_aware_binary_tree_node_proxy
        {
            using intrusive_node = intrusive_parent_aware_binary_tree_node_proxy<vptr>;
            using node_type = parent_aware_binary_tree_node<T, vptr>;

            intrusive_node n_{};
        private:
            parent_aware_binary_tree_node_proxy(intrusive_node n)noexcept : n_{ n }
            {
            }
            intrusive_node intrusive()const noexcept
            {
                return n_;
            }
        public:
            parent_aware_binary_tree_node_proxy() = default;
            parent_aware_binary_tree_node_proxy(
                parent_aware_binary_tree_edge_proxy<T, vptr> p)noexcept
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
            parent_aware_binary_tree_node_proxy& go_left()noexcept
            {
                n_.go_left();
                return *this;
            }
            parent_aware_binary_tree_node_proxy& go_right()noexcept
            {
                n_.go_right();
                return *this;
            }
            parent_aware_binary_tree_node_proxy& go_up()noexcept
            {
                n_.go_up();
                return *this;
            }
            parent_aware_binary_tree_node_proxy get_left()const noexcept
            {
                return { n_.get_left() };
            }
            parent_aware_binary_tree_node_proxy get_right()const noexcept
            {
                return { n_.get_right() };
            }
            parent_aware_binary_tree_node_proxy get_parent()const noexcept
            {
                return { n_.get_parent() };
            }
            std::pair<parent_aware_binary_tree_edge_proxy<T, vptr>,
                parent_aware_binary_tree_edge_proxy<T, vptr>> get_children()const noexcept
            {
                return { parent_aware_binary_tree_edge_proxy<T, vptr>{
                             intrusive_parent_aware_binary_tree_edge_proxy<vptr>{ n_, true } },
                         parent_aware_binary_tree_edge_proxy<T, vptr>{
                             intrusive_parent_aware_binary_tree_edge_proxy<vptr>{ n_, false } } };
            }
            operator parent_aware_binary_tree_edge_proxy<T, vptr>()const noexcept
            {
                return { static_cast<intrusive_parent_aware_binary_tree_edge_proxy<vptr>>(n_) };
            }
            friend bool operator==(parent_aware_binary_tree_node_proxy,
                parent_aware_binary_tree_node_proxy)noexcept = default;
            friend struct std::hash<parent_aware_binary_tree_node_proxy>;
            friend class parent_aware_binary_tree_node_const_proxy<T, vptr>;
            template<class T1, class Alloc1>
            friend class Yc::parent_aware_binary_tree;
            using value_type = T;
            using reference = T&;
        };
    }
}

template<class T, class vptr>
struct std::hash<Yc::details::parent_aware_binary_tree_node_proxy<T, vptr>>
{
    size_t operator()(const Yc::details::parent_aware_binary_tree_node_proxy<T, vptr>& p)const noexcept
    {
        if (!p)
        {
            return 0;
        }
        return std::hash<Yc::intrusive_parent_aware_binary_tree_node_proxy<vptr>>{}(p.intrusive());
    }
};

namespace Yc
{
    namespace details
    {
        // node_const_proxy：node_proxy 的常量版本
        template<class T, class vptr>
        class parent_aware_binary_tree_node_const_proxy
        {
            using intrusive_node = intrusive_parent_aware_binary_tree_node_proxy<vptr>;
            using node_type = parent_aware_binary_tree_node<T, vptr>;

            intrusive_node n_{};
        private:
            parent_aware_binary_tree_node_const_proxy(intrusive_node n)noexcept : n_{ n }
            {
            }
            intrusive_node intrusive()const noexcept
            {
                return n_;
            }
        public:
            parent_aware_binary_tree_node_const_proxy() = default;
            parent_aware_binary_tree_node_const_proxy(
                parent_aware_binary_tree_node_proxy<T, vptr> p)noexcept
                : n_{ p.intrusive() }
            {
            }
            parent_aware_binary_tree_node_const_proxy(
                parent_aware_binary_tree_edge_const_proxy<T, vptr> p)noexcept
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
            parent_aware_binary_tree_node_const_proxy& go_left()noexcept
            {
                n_.go_left();
                return *this;
            }
            parent_aware_binary_tree_node_const_proxy& go_right()noexcept
            {
                n_.go_right();
                return *this;
            }
            parent_aware_binary_tree_node_const_proxy& go_up()noexcept
            {
                n_.go_up();
                return *this;
            }
            parent_aware_binary_tree_node_const_proxy get_left()const noexcept
            {
                return { n_.get_left() };
            }
            parent_aware_binary_tree_node_const_proxy get_right()const noexcept
            {
                return { n_.get_right() };
            }
            parent_aware_binary_tree_node_const_proxy get_parent()const noexcept
            {
                return { n_.get_parent() };
            }
            std::pair<parent_aware_binary_tree_edge_const_proxy<T, vptr>,
                parent_aware_binary_tree_edge_const_proxy<T, vptr>> get_children()const noexcept
            {
                return { parent_aware_binary_tree_edge_const_proxy<T, vptr>{
                             intrusive_parent_aware_binary_tree_edge_proxy<vptr>{ n_, true } },
                         parent_aware_binary_tree_edge_const_proxy<T, vptr>{
                             intrusive_parent_aware_binary_tree_edge_proxy<vptr>{ n_, false } } };
            }
            operator parent_aware_binary_tree_edge_const_proxy<T, vptr>()const noexcept
            {
                return { static_cast<intrusive_parent_aware_binary_tree_edge_proxy<vptr>>(n_) };
            }
            friend bool operator==(parent_aware_binary_tree_node_const_proxy,
                parent_aware_binary_tree_node_const_proxy)noexcept = default;
            friend struct std::hash<parent_aware_binary_tree_node_const_proxy>;
            template<class T1, class Alloc1>
            friend class Yc::parent_aware_binary_tree;
            using value_type = T;
            using reference = const T&;
        };
    }
}

template<class T, class vptr>
struct std::hash<Yc::details::parent_aware_binary_tree_node_const_proxy<T, vptr>>
{
    size_t operator()(const Yc::details::parent_aware_binary_tree_node_const_proxy<T, vptr>& p)const noexcept
    {
        if (!p)
        {
            return 0;
        }
        return std::hash<Yc::intrusive_parent_aware_binary_tree_node_proxy<vptr>>{}(p.intrusive());
    }
};

namespace Yc
{
    namespace details
    {
        // 多态 get_children 函数对象
        struct parent_aware_binary_tree_get_children_t
        {
            template<class T, class vptr>
            auto operator()(parent_aware_binary_tree_edge_proxy<T, vptr> p)const noexcept
            {
                return p.get_children();
            }
            template<class T, class vptr>
            auto operator()(parent_aware_binary_tree_edge_const_proxy<T, vptr> p)const noexcept
            {
                return p.get_children();
            }
            template<class T, class vptr>
            auto operator()(parent_aware_binary_tree_node_proxy<T, vptr> p)const noexcept
            {
                return p.get_children();
            }
            template<class T, class vptr>
            auto operator()(parent_aware_binary_tree_node_const_proxy<T, vptr> p)const noexcept
            {
                return p.get_children();
            }
        };
    }
    namespace parent_aware_binary_tree_functional
    {
        constexpr inline Yc::details::parent_aware_binary_tree_get_children_t get_children{};
    }

    // ===================================================================
    // parent_aware_binary_tree：持有侵入式父指针树的拥有型适配器
    // ===================================================================
    template<class T, class Alloc = std::allocator<T>>
    class parent_aware_binary_tree
    {
        // 结构指针的"种子"类型：普通分配器是 T*，缀饰指针分配器是 fancy<T>。
        using vptr = typename std::allocator_traits<Alloc>::pointer;
        using intrusive_tree_type = basic_intrusive_parent_aware_binary_tree<vptr>;
        using node_base = intrusive_parent_aware_binary_tree_node_base<vptr>;
        using node_type = details::parent_aware_binary_tree_node<T, vptr>;
        using node_allocator = std::allocator_traits<Alloc>::template rebind_alloc<node_type>;

        // 分配器（使用 YC_NO_UNIQUE_ADDRESS 以零开销存储无状态分配器）
        YC_NO_UNIQUE_ADDRESS Alloc alloc{};
        // 侵入式父指针树：内部含哨兵，不拥有节点
        intrusive_tree_type tree_{};

        // 分配并构造一个节点，返回节点基类引用
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
        using allocator_type = Alloc;
        using value_type = T;
        using reference = T&;
        using pointer = std::allocator_traits<Alloc>::pointer;
        using edge_proxy = details::parent_aware_binary_tree_edge_proxy<T, vptr>;
        using edge_const_proxy = details::parent_aware_binary_tree_edge_const_proxy<T, vptr>;
        using node_proxy = details::parent_aware_binary_tree_node_proxy<T, vptr>;
        using node_const_proxy = details::parent_aware_binary_tree_node_const_proxy<T, vptr>;
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
        template<class ValueGetter, class ChildrenGetter, class InitializeHandle>
        parent_aware_binary_tree recur_and_write(edge_const_proxy p,
            ValueGetter vg, ChildrenGetter cg, InitializeHandle h)
        {
            parent_aware_binary_tree ret{ cut(p) };
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
            guard.t = nullptr;
            return ret;
        }
        parent_aware_binary_tree() = default;
        explicit parent_aware_binary_tree(const Alloc& a)noexcept :alloc{ a }
        {
        }
        template<class ValueGetter, class ChildrenGetter, class InitializeHandle>
        parent_aware_binary_tree(ValueGetter vg, ChildrenGetter cg,
            InitializeHandle h, const Alloc& a = {}) :parent_aware_binary_tree(a)
        {
            if (h)
            {
                recur_and_write_impl(root(), vg, cg, h);
            }
        }
        parent_aware_binary_tree(const parent_aware_binary_tree& b)
            :alloc{ std::allocator_traits<Alloc>::select_on_container_copy_construction(b.alloc) }
        {
            auto copier = [](edge_const_proxy p) -> const T&
            {
                return *p;
            };
            recur_and_write(root(), copier,
                parent_aware_binary_tree_functional::get_children, b.croot());
        }
        parent_aware_binary_tree& operator=(const parent_aware_binary_tree& b)
        {
            if (this == &b)
            {
                return *this;
            }
            Alloc tmp{ alloc };
            parent_aware_binary_tree t1{ cut(root()) };
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
            recur_and_write(root(), copier,
                parent_aware_binary_tree_functional::get_children, b.root());
            guard.t = nullptr;
            return *this;
        }
        parent_aware_binary_tree(parent_aware_binary_tree&& other)noexcept :
            alloc{ std::move(other.alloc) }
        {
            tree_.swap(other.tree_);
        }
        parent_aware_binary_tree& operator=(parent_aware_binary_tree&& other)noexcept(
            std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value ||
            std::allocator_traits<Alloc>::is_always_equal::value)
        {
            if constexpr (std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value)
            {
                std::swap(alloc, other.alloc);
                tree_.swap(other.tree_);
                return *this;
            }
            else
            {
                if (alloc == other.alloc)
                {
                    tree_.swap(other.tree_);
                    return *this;
                }
                auto vg = [](edge_proxy p) -> T&&
                {
                    return (T&&)*p;
                };
                parent_aware_binary_tree tmp{ vg,
                    parent_aware_binary_tree_functional::get_children,
                    other.root(), alloc };
                tree_.swap(tmp.tree_);
                return *this;
            }
        }
        ~parent_aware_binary_tree()
        {
            clear();
        }
        Alloc get_allocator()const noexcept
        {
            return alloc;
        }
        bool empty()const noexcept
        {
            return tree_.empty();
        }
        node_proxy parent_of_nroot()noexcept
        {
            return node_proxy{ tree_.parent_of_nroot() };
        }
        node_const_proxy parent_of_nroot()const noexcept
        {
            return node_const_proxy{
                const_cast<parent_aware_binary_tree*>(this)->tree_.parent_of_nroot() };
        }
        node_const_proxy parent_of_cnroot()const noexcept
        {
            return node_const_proxy{
                const_cast<parent_aware_binary_tree*>(this)->tree_.parent_of_nroot() };
        }
        edge_proxy root()noexcept
        {
            return edge_proxy{ tree_.root() };
        }
        edge_const_proxy root()const noexcept
        {
            return edge_const_proxy{ const_cast<parent_aware_binary_tree*>(this)->tree_.root() };
        }
        edge_const_proxy croot()const noexcept
        {
            return edge_const_proxy{ const_cast<parent_aware_binary_tree*>(this)->tree_.root() };
        }
        node_proxy nroot()noexcept
        {
            return node_proxy{ tree_.nroot() };
        }
        node_const_proxy nroot()const noexcept
        {
            return node_const_proxy{
                const_cast<parent_aware_binary_tree*>(this)->tree_.nroot() };
        }
        node_const_proxy cnroot()const noexcept
        {
            return node_const_proxy{
                const_cast<parent_aware_binary_tree*>(this)->tree_.nroot() };
        }
        parent_aware_binary_tree cut(edge_const_proxy p)noexcept
        {
            parent_aware_binary_tree ret{ alloc };
            ret.tree_ = tree_.cut(p.intrusive());
            return ret;
        }
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
            if (has_l && has_r)
            {
                for_each_impl<Proxy>(l, f);
                std::invoke(f, *p);
                p = Proxy{ r };
                goto derecur;
            }
            std::invoke(f, *p);
            if (has_l)
            {
                p = Proxy{ l };
                goto derecur;
            }
            if (has_r)
            {
                p = Proxy{ r };
                goto derecur;
            }
        }
    public:
        template<class... Args>
        parent_aware_binary_tree emplace(edge_const_proxy p, Args&&... args)
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
                    parent_aware_binary_tree* t;
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
            parent_aware_binary_tree ret{ alloc };
            auto old = intrusive_tree_type::insert(p.intrusive(), new_node);
            ret.tree_.swap(old);
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
        parent_aware_binary_tree splice(edge_const_proxy to, edge_const_proxy from)noexcept
        {
            parent_aware_binary_tree ret{ alloc };
            ret.tree_ = intrusive_tree_type::splice(
                to.intrusive(), from.intrusive());
            return ret;
        }
        parent_aware_binary_tree splice(edge_const_proxy p, parent_aware_binary_tree& tree)noexcept
        {
            return splice(p, tree.root());
        }
        parent_aware_binary_tree splice(edge_const_proxy p, parent_aware_binary_tree&& tree)noexcept
        {
            return splice(p, tree.root());
        }
        static void swap_sub_tree(edge_const_proxy l, edge_const_proxy r)noexcept
        {
            intrusive_tree_type::swap_sub_tree(l.intrusive(), r.intrusive());
        }
        static void swap_node(edge_const_proxy l, edge_const_proxy r,
            child_check chk = check_all)noexcept
        {
            intrusive_tree_type::swap_node(l.intrusive(), r.intrusive(), chk);
        }
        void swap(parent_aware_binary_tree& other)noexcept
        {
            if constexpr (std::allocator_traits<Alloc>::propagate_on_container_swap::value)
            {
                std::swap(alloc, other.alloc);
            }
            tree_.swap(other.tree_);
        }
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
    };
    template<class T, class Alloc>
    void swap(parent_aware_binary_tree<T, Alloc>& l,
        parent_aware_binary_tree<T, Alloc>& r)noexcept
    {
        l.swap(r);
    }
}
