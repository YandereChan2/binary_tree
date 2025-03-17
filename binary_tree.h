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
        struct binary_tree_node_base
        {
            binary_tree_node_base* left{};
            binary_tree_node_base* right{};
        };
        template<class T>
        struct binary_tree_node : binary_tree_node_base
        {
            data_t<T> data;
        };
        template<class T>
        class binary_tree_edge_const_proxy;
        template<class T>
        class binary_tree_node_proxy;
        template<class T>
        class binary_tree_node_const_proxy;
        template<class T>
        class binary_tree_edge_proxy
        {
            binary_tree_node_base** ptr{};
        public:
            binary_tree_edge_proxy() = default;
            binary_tree_edge_proxy(const binary_tree_edge_proxy&) = default;
            binary_tree_edge_proxy(binary_tree_node_base** ptr)noexcept :ptr{ ptr }
            {
            }
            binary_tree_edge_proxy& operator=(const binary_tree_edge_proxy&) = default;
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
            binary_tree_edge_proxy& go_left()noexcept
            {
                ptr = &((*ptr)->left);
                return *this;
            }
            binary_tree_edge_proxy& go_right()noexcept
            {
                ptr = &((*ptr)->right);
                return *this;
            }
            T& operator*()const noexcept
            {
                return ((binary_tree_node<T>*)(*ptr))->data.data;
            }
            T* operator->()const noexcept
            {
                return std::addressof(((binary_tree_node<T>*)(*ptr))->data.data);
            }
            std::pair<binary_tree_edge_proxy, binary_tree_edge_proxy> get_children()const noexcept
            {
                std::pair<binary_tree_edge_proxy, binary_tree_edge_proxy> ret{ *this, *this };
                ret.first.go_left();
                ret.second.go_right();
                return ret;
            }
            friend bool operator==(binary_tree_edge_proxy l, binary_tree_edge_proxy r)noexcept
            {
                return l.ptr == r.ptr;
            }
            friend struct std::hash<binary_tree_edge_proxy>;
            friend class binary_tree_edge_const_proxy<T>;
            template<class T1, class Alloc>
            friend class Yc::binary_tree;
            friend class binary_tree_node_proxy<T>;
            using value_type = T;
        };
    }
}
template<class T>
struct std::hash<Yc::details::binary_tree_edge_proxy<T>>
{
    size_t operator()(const Yc::details::binary_tree_edge_proxy<T>& para)const noexcept
    {
        return std::hash<Yc::details::binary_tree_node_base**>{}(para.ptr);
    }
};
namespace Yc
{
    namespace details
    {
        template<class T>
        class binary_tree_edge_const_proxy
        {
            binary_tree_node_base** ptr{};
        public:
            binary_tree_edge_const_proxy() = default;
            binary_tree_edge_const_proxy(const binary_tree_edge_const_proxy&) = default;
            binary_tree_edge_const_proxy(const binary_tree_edge_proxy<T>& p)noexcept : ptr{ p.ptr }
            {
            }
            binary_tree_edge_const_proxy(binary_tree_node_base** ptr)noexcept :ptr{ ptr }
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
                ptr = &((*ptr)->left);
                return *this;
            }
            binary_tree_edge_const_proxy& go_right()noexcept
            {
                ptr = &((*ptr)->right);
                return *this;
            }
            const T& operator*()const noexcept
            {
                return ((binary_tree_node<T>*)(*ptr))->data.data;
            }
            const T* operator->()const noexcept
            {
                return std::addressof(((binary_tree_node<T>*)(*ptr))->data.data);
            }
            std::pair<binary_tree_edge_const_proxy, binary_tree_edge_const_proxy> get_children()const noexcept
            {
                std::pair<binary_tree_edge_const_proxy, binary_tree_edge_const_proxy> ret{ *this, *this };
                ret.first.go_left();
                ret.second.go_right();
                return ret;
            }
            friend bool operator==(binary_tree_edge_const_proxy l, binary_tree_edge_const_proxy r)noexcept
            {
                return l.ptr == r.ptr;
            }
            friend struct std::hash<binary_tree_edge_const_proxy>;
            template<class T1, class Alloc>
            friend class Yc::binary_tree;
            using value_type = const T;
        };
    }
}
template<class T>
struct std::hash<Yc::details::binary_tree_edge_const_proxy<T>>
{
    size_t operator()(const Yc::details::binary_tree_edge_const_proxy<T>& para)const noexcept
    {
        return std::hash<Yc::details::binary_tree_node_base**>{}(para.ptr);
    }
};
namespace Yc
{
    namespace details
    {
        template<class T>
        class binary_tree_node_proxy
        {
            binary_tree_node_base* ptr{};
        public:
            binary_tree_node_proxy() = default;
            binary_tree_node_proxy(binary_tree_node<T>* ptr)noexcept :ptr{ ptr }
            {
            }
            binary_tree_node_proxy(binary_tree_edge_proxy<T> p)noexcept :ptr{ *p.ptr }
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
                return ((binary_tree_node<T>*)ptr)->data.data;
            }
            T* operator->()const noexcept
            {
                return std::addressof(((binary_tree_node<T>*)ptr)->data.data);
            }
            binary_tree_node_proxy& go_left()noexcept
            {
                ptr = ptr->left;
                return *this;
            }
            binary_tree_node_proxy& go_right()noexcept
            {
                ptr = ptr->right;
                return *this;
            }
            std::pair<binary_tree_edge_proxy<T>, binary_tree_edge_proxy<T>> get_children()const
            {
                return { {&ptr->left},{&ptr->right}};
            }
            friend bool operator==(binary_tree_node_proxy l, binary_tree_node_proxy r)noexcept
            {
                return l.ptr == r.ptr;
            }
            friend struct std::hash<binary_tree_node_proxy>;
            friend class binary_tree_node_const_proxy<T>;
            using value_type = T;
        };
    }
}
template<class T>
struct std::hash<Yc::details::binary_tree_node_proxy<T>>
{
    size_t operator()(const Yc::details::binary_tree_node_proxy<T>& p)const noexcept
    {
        return std::hash<Yc::details::binary_tree_node_base*>{}(p.ptr);
    }
};
namespace Yc
{
    namespace details
    {
        template<class T>
        class binary_tree_node_const_proxy
        {
            binary_tree_node_base* ptr{};
        public:
            binary_tree_node_const_proxy() = default;
            binary_tree_node_const_proxy(binary_tree_node_base* ptr)noexcept :ptr{ ptr }
            {
            }
            binary_tree_node_const_proxy(binary_tree_edge_const_proxy<T> p)noexcept :ptr{ *p.ptr }
            {
            }
            binary_tree_node_const_proxy(binary_tree_node_proxy<T> p)noexcept :ptr{ p.ptr }
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
                return ((binary_tree_node<T>*)ptr)->data.data;
            }
            const T* operator->()const noexcept
            {
                return std::addressof(((binary_tree_node<T>*)ptr)->data.data);
            }
            binary_tree_node_const_proxy& go_left()noexcept
            {
                ptr = ptr->left;
                return *this;
            }
            binary_tree_node_const_proxy& go_right()noexcept
            {
                ptr = ptr->right;
                return *this;
            }
            std::pair<binary_tree_edge_const_proxy<T>, binary_tree_edge_const_proxy<T>> get_children()const
            {
                return { {&ptr->left},{&ptr->right}};
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
template<class T>
struct std::hash<Yc::details::binary_tree_node_const_proxy<T>>
{
    size_t operator()(const Yc::details::binary_tree_node_const_proxy<T>& p)const noexcept
    {
        return std::hash<Yc::details::binary_tree_node_base*>{}(p.ptr);
    }
};
namespace Yc
{
    namespace details
    {
        struct binary_tree_get_children_t
        {
            template<class T>
            auto operator()(binary_tree_edge_proxy<T> p)const noexcept
            {
                return p.get_children();
            }
            template<class T>
            auto operator()(binary_tree_edge_const_proxy<T> p)const noexcept
            {
                return p.get_children();
            }
            template<class T>
            auto operator()(binary_tree_node_proxy<T> p)const noexcept
            {
                return p.get_children();
            }
            template<class T>
            auto operator()(binary_tree_node_const_proxy<T> p)const noexcept
            {
                return p.get_children();
            }
        };
    }
    namespace binary_tree_functional
    {
        constexpr inline Yc::details::binary_tree_get_children_t get_children{};
    }
    template<class T, class Alloc = std::allocator<T>>
    class binary_tree
    {
        using node_ptr = details::binary_tree_node_base*;
        using node_type = details::binary_tree_node<T>;
        using node_allocator = std::allocator_traits<Alloc>::template rebind_alloc<node_type>;
        [[no_unique_address]] Alloc alloc{};
        node_ptr root_ptr{};
        node_ptr allocate_node()
        {
            node_allocator a{ alloc };
            node_ptr p = std::allocator_traits<node_allocator>::allocate(a, 1);
            new (p)node_type;
            p->left = nullptr;
            p->right = nullptr;
            return p;
        }
        void deallocate_node(node_ptr p)noexcept
        {
            std::destroy_at((node_type*)p);
            node_allocator a{ alloc };
            return std::allocator_traits<node_allocator>::deallocate(a, (node_type*)p, 1);
        }
    public:
        Alloc get_allocator()const noexcept
        {
            return alloc;
        }
        using value_type = T;
        using reference = T&;
        using edge_proxy = details::binary_tree_edge_proxy<T>;
        using edge_const_proxy = details::binary_tree_edge_const_proxy<T>;
        using node_proxy = details::binary_tree_node_proxy<T>;
        using node_const_proxy = details::binary_tree_node_const_proxy<T>;
    private:
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
        template<
            class ValueGetter,
            class ChildrenGetter,
            class InitializeHandle
        >
        void recur_and_overwrite_impl(
            edge_const_proxy p,
            ValueGetter& vg,
            ChildrenGetter& cg,
            InitializeHandle h
        )
        {
            derecur:
            if (!h)
            {
                erase(p);
                return;
            }
            if (p)
            {
                T& t = (T&)*p;
                if constexpr (requires{
                    t = std::invoke(vg, h);
                } && noexcept(t = std::invoke(vg, h)))
                {
                    t = std::invoke(vg, h);
                }
                else if constexpr (requires{
                    t = T{ std::invoke(vg,h) };
                } && noexcept(t = T{ std::invoke(vg,h) }))
                {
                    t = T{ std::invoke(vg,h) };
                }
                else
                {
                    std::allocator_traits<Alloc>::destroy(alloc, (T*)p.operator->());
                    static_assert(std::is_nothrow_constructible_v<T, decltype(std::invoke(vg, h))>);
                    std::allocator_traits<Alloc>::construct(alloc, (T*)p.operator->(), std::invoke(vg, h));
                }
            }
            else
            {
                emplace(p, std::invoke(vg, h));
            }
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
            recur_and_overwrite_impl(l, vg, cg, lh);
            goto tmp;
        }
    public:
        template<
            class ValueGetter,
            class ChildrenGetter,
            class InitializeHandle
        >requires requires (
            edge_const_proxy p,
            ValueGetter vg,
            ChildrenGetter cg,
            InitializeHandle h,
            binary_tree b
            ) {
            b.emplace(p, std::invoke(vg, h));
            //auto&& [lh, rh] = std::invoke(cg, h);
            (bool)h;
        }
        binary_tree recur_and_write(
            edge_const_proxy p,
            ValueGetter vg,
            ChildrenGetter cg,
            InitializeHandle h
        )
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
            }guard{this, p, &ret};
            if(h)
            {
                recur_and_write_impl(p, vg, cg, h);
            }
            guard.t = nullptr;
            return ret;
        }
        template<
            class ValueGetter,
            class ChildrenGetter,
            class InitializeHandle
        >requires requires (
            edge_const_proxy p,
            ValueGetter vg,
            ChildrenGetter cg,
            InitializeHandle h
            ) {
            emplace(p, std::invoke(vg, h));
            //auto&& [lh, rh] = std::invoke(cg, h);
            (bool)h;
        }
        void recur_and_overwrite(
            edge_const_proxy p,
            ValueGetter vg,
            ChildrenGetter cg,
            InitializeHandle h
        )
        {
            if (h)
            {
                recur_and_overwrite_impl(p, vg, cg, h);
            }
        }
        binary_tree() = default;
        explicit binary_tree(const Alloc& a)noexcept :alloc{a}
        {
        }
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
        binary_tree(const binary_tree& b) :binary_tree{ b.alloc }
        {
            auto copier = [](edge_const_proxy p) -> const T&
                {
                    return *p;
                };
            recur_and_write
                (root(), copier, binary_tree_functional::get_children, b.croot());
        }
        binary_tree& operator=(const binary_tree& b)
        {
            Alloc tmp{ alloc };
            struct _guard
            {
                binary_tree* t{};
                Alloc* tmp{};
                ~_guard()
                {
                    if (t)
                    {
                        t->alloc = *tmp;
                    }
                }
            }guard{this, &tmp};
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
        binary_tree(binary_tree&& b)noexcept :
            root_ptr{ std::exchange(b.root_ptr, {}) }, alloc{ std::move(b.alloc) }
        {
        }
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
        edge_proxy root()noexcept
        {
            return { (node_ptr*)&root_ptr };
        }
        edge_const_proxy root()const noexcept
        {
            return { (node_ptr*)&root_ptr };
        }
        edge_const_proxy croot()const noexcept
        {
            return { (node_ptr*)&root_ptr };
        }
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
        binary_tree cut(edge_const_proxy p)noexcept
        {
            node_ptr* ptr = p.ptr;
            binary_tree ret{ alloc };
            ret.root_ptr = std::exchange(*ptr, {});
            return ret;
        }
        void erase(edge_const_proxy p)
        {
            if (!p.null())
            {
                auto [l, r] = p.get_children();
                erase(l);
                erase(r);
                node_ptr* ptr = p.ptr;
                std::allocator_traits<Alloc>::destroy(alloc, (T*)p.operator->());
                deallocate_node(*ptr);
                *ptr = nullptr;
            }
        }
        void clear()noexcept
        {
            erase(root());
        }
        template<class... Args>
        binary_tree emplace(edge_const_proxy p, Args&&... args)
        {
            node_ptr new_node = allocate_node();
            node_allocator na{ alloc };
            if constexpr (std::is_nothrow_constructible_v<T, Args...>)
            {
                std::allocator_traits<Alloc>::
                    construct(alloc, std::addressof(((node_type*)new_node)->data.data), std::forward<Args>(args)...);
            }
            else
            {
                struct guard_t
                {
                    binary_tree* t;
                    node_ptr ptr;
                    ~guard_t()
                    {
                        if (ptr != nullptr)
                        {
                            t->deallocate_node(ptr);
                        }
                    }
                }guard{ this,new_node };
                std::allocator_traits<Alloc>::
                    construct(na, std::addressof(((node_type*)new_node)->data.data), std::forward<Args>(args)...);
                guard.ptr = nullptr;
            }
            binary_tree ret{ cut(p) };
            node_ptr* ptr = p.ptr;
            *ptr = new_node;
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
        binary_tree splice(edge_const_proxy to, edge_const_proxy from)noexcept
        {
            binary_tree ret{ cut(to) };
            *(to.ptr) = std::exchange(*(from.ptr), {});
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
        static void swap_sub_tree(edge_const_proxy l, edge_const_proxy r)noexcept
        {
            std::swap(*(l.ptr), *(r.ptr));
        }
        void swap(binary_tree& other)noexcept
        {
            if constexpr (std::allocator_traits<Alloc>::propagate_on_container_swap::value)
            {
                using namespace std;
                swap(alloc, other.alloc);
            }
            swap_sub_tree(root(), other.root());
        }
        ~binary_tree()
        {
            clear();
        }
        void left_rotate(edge_const_proxy p)noexcept
        {
            //     |                         |
            //    <A>                       <C>
            //   /   \                      / \
            //  <B>  <C>     =====>       <A> <E>
            //       / \                  / \
            //     <D> <E>              <B> <D>
            edge_const_proxy q = p;
            q.go_right();
            binary_tree t1 = cut(q); // <D>-<rC>-<E>
            binary_tree t2 = splice(p, t1.root()); //<B>-<rA>
            //     |
            //    <C>                 <A>
            //   /   \                /
            //  <D>  <E>            <B>
            q = p;
            q.go_left();
            binary_tree t3 = splice(q, t2.root());
            q.go_right();
            splice(q, t3.root());
        }
        void right_rotate(edge_const_proxy p)noexcept
        {
            edge_const_proxy q = p;
            q.go_left();
            binary_tree t1 = cut(q);
            binary_tree t2 = splice(p, t1.root());
            q = p;
            q.go_right();
            binary_tree t3 = splice(q, t2.root());
            q.go_left();
            splice(q, t3.root());
        }
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
    private:
        template<class UnaryFunc, class Proj>
        static void pre_order_visit_impl(node_proxy p, UnaryFunc func, Proj proj)
            noexcept(noexcept(std::invoke(func, std::invoke(proj, std::declval<T&>()))))
        {
            derecur:
            std::invoke(func, std::invoke(proj, *p));
            auto [l, r] = p.get_children();
            bool lb = (bool)l, rb = (bool)r;
            if (!(lb && rb))
            {
                if (lb)
                {
                    p = l;
                    goto derecur;
                }
                if (rb)
                {
                    tmp:
                    p = r;
                    goto derecur;
                }
                return;
            }
            pre_order_visit_impl<UnaryFunc, Proj>(l, func, proj);
            goto tmp;
        }
        template<class UnaryFunc, class Proj>
        static void pre_order_visit_impl(node_const_proxy p, UnaryFunc func, Proj proj)
            noexcept(noexcept(std::invoke(func, std::invoke(proj, std::declval<const T&>()))))
        {
        derecur:
            std::invoke(func, std::invoke(proj, *p));
            auto [l, r] = p.get_children();
            bool lb = (bool)l, rb = (bool)r;
            if (!(lb && rb))
            {
                if (lb)
                {
                    p = l;
                    goto derecur;
                }
                if (rb)
                {
                tmp:
                    p = r;
                    goto derecur;
                }
                return;
            }
            pre_order_visit_impl<UnaryFunc, Proj>(l, func, proj);
            goto tmp;
        }
        template<class UnaryFunc, class Proj>
        static void in_order_visit_impl(node_proxy p, UnaryFunc func, Proj proj)
            noexcept(noexcept(std::invoke(func, std::invoke(proj, std::declval<T&>()))))
        {
            derecur:
            auto [l, r] = p.get_children();
            if (l)
            {
                in_order_visit_impl<UnaryFunc, Proj>(l, func, proj);
            }
            std::invoke(func, std::invoke(proj, *p));
            if (r)
            {
                p = r;
                goto derecur;
            }
        }
        template<class UnaryFunc, class Proj>
        static void in_order_visit_impl(node_const_proxy p, UnaryFunc func, Proj proj)
            noexcept(noexcept(std::invoke(func, std::invoke(proj, std::declval<T&>()))))
        {
        derecur:
            auto [l, r] = p.get_children();
            if (l)
            {
                in_order_visit_impl<UnaryFunc, Proj>(l, func, proj);
            }
            std::invoke(func, std::invoke(proj, *p));
            if (r)
            {
                p = r;
                goto derecur;
            }
        }
        template<class UnaryFunc, class Proj>
        static void post_order_visit_impl(node_proxy p, UnaryFunc func, Proj proj)
            noexcept(noexcept(std::invoke(func, std::invoke(proj, std::declval<T&>()))))
        {
            auto [l, r] = p.get_children();
            if (l)
            {
                post_order_visit_impl<UnaryFunc, Proj>(l, func, proj);
            }
            if (r)
            {
                post_order_visit_impl<UnaryFunc, Proj>(r, func, proj);
            }
            std::invoke(func, std::invoke(proj, *p));
        }
        template<class UnaryFunc, class Proj>
        static void post_order_visit_impl(node_const_proxy p, UnaryFunc func, Proj proj)
            noexcept(noexcept(std::invoke(func, std::invoke(proj, std::declval<T&>()))))
        {
            auto [l, r] = p.get_children();
            if (l)
            {
                post_order_visit_impl<UnaryFunc, Proj>(l, func, proj);
            }
            if (r)
            {
                post_order_visit_impl<UnaryFunc, Proj>(r, func, proj);
            }
            std::invoke(func, std::invoke(proj, *p));
        }
    public:
        template<class UnaryFunc, class Proj = std::identity>
        static UnaryFunc pre_order_visit(node_proxy p ,UnaryFunc func, Proj proj = {})
            noexcept(noexcept(std::invoke(func, std::invoke(proj, std::declval<T&>()))))
        {
            if (p)
            {
                pre_order_visit_impl(p, std::ref(func), std::ref(proj));
            }
            return func;
        }
        template<class UnaryFunc, class Proj = std::identity>
        static UnaryFunc pre_order_visit(node_const_proxy p, UnaryFunc func, Proj proj = {}) 
            noexcept(noexcept(std::invoke(func, std::invoke(proj, std::declval<const T&>()))))
        {
            if (p)
            {
                pre_order_visit_impl(p, std::ref(func), std::ref(proj));
            }
            return func;
        }
        template<class UnaryFunc, class Proj = std::identity>
        static UnaryFunc in_order_visit(node_proxy p, UnaryFunc func, Proj proj = {})
            noexcept(noexcept(std::invoke(func, std::invoke(proj, std::declval<T&>()))))
        {
            if (p)
            {
                in_order_visit_impl(p, std::ref(func), std::ref(proj));
            }
            return func;
        }
        template<class UnaryFunc, class Proj = std::identity>
        static UnaryFunc in_order_visit(node_const_proxy p, UnaryFunc func, Proj proj = {})
            noexcept(noexcept(std::invoke(func, std::invoke(proj, std::declval<const T&>()))))
        {
            if (p)
            {
                in_order_visit_impl(p, std::ref(func), std::ref(proj));
            }
            return func;
        }
        template<class UnaryFunc, class Proj = std::identity>
        static UnaryFunc post_order_visit(node_proxy p, UnaryFunc func, Proj proj = {})
            noexcept(noexcept(std::invoke(func, std::invoke(proj, std::declval<T&>()))))
        {
            if (p)
            {
                post_order_visit_impl(p, std::ref(func), std::ref(proj));
            }
            return func;
        }
        template<class UnaryFunc, class Proj = std::identity>
        static UnaryFunc post_order_visit(node_const_proxy p, UnaryFunc func, Proj proj = {})
            noexcept(noexcept(std::invoke(func, std::invoke(proj, std::declval<const T&>()))))
        {
            if (p)
            {
                post_order_visit_impl(p, std::ref(func), std::ref(proj));
            }
            return func;
        }
    };
    template<class T>
    void swap(binary_tree<T>& l, binary_tree<T>& r)noexcept
    {
        l.swap(r);
    }
}
