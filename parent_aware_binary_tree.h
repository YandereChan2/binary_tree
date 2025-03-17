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
        struct ptr_node
        {
            ptr_node* ptr{};
        };
        struct parent_aware_binary_tree_node_base
        {
            ptr_node ptr[3];
        };
        template<class T>
        struct parent_aware_binary_tree_node : parent_aware_binary_tree_node_base
        {
            data_t<T> data;
        };
        namespace
        {
            ptr_node* find_this(ptr_node* p)noexcept
            {
                if (p->ptr)
                {
                    return ((p->ptr) + 2)->ptr;
                }
                ++p;
                if (!p->ptr)
                {
                    return p - 1;
                }
                if ((p->ptr + 2)->ptr == p - 1)
                {
                    return p - 1;
                }
                return p - 2;
            }
        }
        template<class T>
        class parent_aware_binary_tree_edge_const_proxy;
        template<class T>
        class parent_aware_binary_tree_node_proxy;
        template<class T>
        class parent_aware_binary_tree_node_const_proxy;
        template<class T>
        class parent_aware_binary_tree_edge_proxy
        {
            ptr_node* ptr{};
        public:
            parent_aware_binary_tree_edge_proxy() = default;
            parent_aware_binary_tree_edge_proxy(const parent_aware_binary_tree_edge_proxy&) = default;
            parent_aware_binary_tree_edge_proxy& operator=(const parent_aware_binary_tree_edge_proxy&) = default;
            parent_aware_binary_tree_edge_proxy(ptr_node* ptr)noexcept : ptr{ ptr }
            {
            }
            bool valid()const noexcept
            {
                return ptr;
            }
            bool null()const noexcept
            {
                return !ptr->ptr;
            }
            explicit operator bool()const noexcept
            {
                return valid() && (!null());
            }
            parent_aware_binary_tree_edge_proxy& go_left()noexcept
            {
                ptr = ptr->ptr;
                return *this;
            }
            parent_aware_binary_tree_edge_proxy& go_right()noexcept
            {
                ptr = ptr->ptr + 1;
                return *this;
            }
            parent_aware_binary_tree_edge_proxy& go_up()noexcept
            {
                auto ptr2 = (find_this(ptr) + 2)->ptr;
                if (ptr2->ptr == ptr)
                {
                    ptr = ptr2;
                }
                else
                {
                    ptr = ptr2 + 1;
                }
                return *this;
            }
            T& operator*()noexcept
            {
                return ((parent_aware_binary_tree_node<T>*)(parent_aware_binary_tree_node_base*)ptr->ptr)->data.data;
            }
            T* operator->()noexcept
            {
                return std::addressof(((parent_aware_binary_tree_node<T>*)(parent_aware_binary_tree_node_base*)ptr->ptr)->data.data);
            }
            std::pair<parent_aware_binary_tree_edge_proxy, parent_aware_binary_tree_edge_proxy> get_children()const noexcept
            {
                std::pair<parent_aware_binary_tree_edge_proxy, parent_aware_binary_tree_edge_proxy> ret{ *this, *this };
                ret.first.go_left();
                ret.second.go_right();
                return ret;
            }
            friend bool operator==(parent_aware_binary_tree_edge_proxy l, parent_aware_binary_tree_edge_proxy r)noexcept
            {
                return l.ptr == r.ptr;
            }
            friend struct std::hash<parent_aware_binary_tree_edge_proxy>;
            friend class parent_aware_binary_tree_edge_const_proxy<T>;
            template<class T1, class Alloc>
            friend class Yc::parent_aware_binary_tree;
            friend class parent_aware_binary_tree_node_proxy<T>;
            using value_type = T;
        };
    }
}
template<class T>
struct std::hash<Yc::details::parent_aware_binary_tree_edge_proxy<T>>
{
    size_t operator()(const Yc::details::parent_aware_binary_tree_edge_proxy<T>& para)const noexcept
    {
        return std::hash<Yc::details::ptr_node*>{}(para.ptr);
    }
};
namespace Yc
{
    namespace details
    {
        template<class T>
        class parent_aware_binary_tree_edge_const_proxy
        {
            ptr_node* ptr{};
        public:
            parent_aware_binary_tree_edge_const_proxy() = default;
            parent_aware_binary_tree_edge_const_proxy(const parent_aware_binary_tree_edge_const_proxy&) = default;
            parent_aware_binary_tree_edge_const_proxy& operator=(const parent_aware_binary_tree_edge_const_proxy&) = default;
            parent_aware_binary_tree_edge_const_proxy(parent_aware_binary_tree_edge_proxy p)noexcept : ptr{ p.ptr }
            {
            }
            parent_aware_binary_tree_edge_const_proxy(ptr_node* ptr)noexcept : ptr{ ptr }
            {
            }
            bool valid()const noexcept
            {
                return ptr;
            }
            bool null()const noexcept
            {
                return !ptr->ptr;
            }
            explicit operator bool()const noexcept
            {
                return valid() && (!null());
            }
            parent_aware_binary_tree_edge_const_proxy& go_left()noexcept
            {
                ptr = ptr->ptr;
                return *this;
            }
            parent_aware_binary_tree_edge_const_proxy& go_right()noexcept
            {
                ptr = ptr->ptr + 1;
                return *this;
            }
            parent_aware_binary_tree_edge_const_proxy& go_up()noexcept
            {
                auto ptr2 = (find_this(ptr) + 2)->ptr;
                if (ptr2->ptr == ptr)
                {
                    ptr = ptr2;
                }
                else
                {
                    ptr = ptr2 + 1;
                }
                return *this;
            }
            const T& operator*()noexcept
            {
                return ((parent_aware_binary_tree_node<T>*)(parent_aware_binary_tree_node_base*)ptr->ptr)->data.data;
            }
            const T* operator->()noexcept
            {
                return std::addressof(((parent_aware_binary_tree_node<T>*)(parent_aware_binary_tree_node_base*)ptr->ptr)->data.data);
            }
            std::pair<parent_aware_binary_tree_edge_const_proxy, parent_aware_binary_tree_edge_const_proxy> get_children()const noexcept
            {
                std::pair<parent_aware_binary_tree_edge_const_proxy, parent_aware_binary_tree_edge_const_proxy> ret{ *this, *this };
                ret.first.go_left();
                ret.second.go_right();
                return ret;
            }
            friend bool operator==(parent_aware_binary_tree_edge_const_proxy l, parent_aware_binary_tree_edge_const_proxy r)noexcept
            {
                return l.ptr == r.ptr;
            }
            friend struct std::hash<parent_aware_binary_tree_edge_const_proxy>;
            template<class T1, class Alloc>
            friend class Yc::parent_aware_binary_tree;
            friend class parent_aware_binary_tree_node_const_proxy<T>;
            using value_type = const T;
        };
    }
}
template<class T>
struct std::hash<Yc::details::parent_aware_binary_tree_edge_const_proxy<T>>
{
    size_t operator()(const Yc::details::parent_aware_binary_tree_edge_const_proxy<T>& para)const noexcept
    {
        return std::hash<Yc::details::ptr_node*>{}(para.ptr);
    }
};
namespace Yc
{
    namespace details
    {
        template<class T>
        class parent_aware_binary_tree_node_proxy
        {
            ptr_node* ptr{};
        public:
            parent_aware_binary_tree_node_proxy() = default;
            parent_aware_binary_tree_node_proxy(const parent_aware_binary_tree_node_proxy&) = default;
            parent_aware_binary_tree_node_proxy& operator=(const parent_aware_binary_tree_node_proxy&) = default;
            parent_aware_binary_tree_node_proxy(ptr_node* ptr)noexcept :ptr{ ptr }
            {
            }
            parent_aware_binary_tree_node_proxy(parent_aware_binary_tree_edge_proxy<T> p)noexcept : ptr{p.ptr->ptr}
            {
            }
            bool null()const noexcept
            {
                return ptr == nullptr;
            }
            explicit operator bool()const noexcept
            {
                return ptr;
            }
            parent_aware_binary_tree_node_proxy& go_left()noexcept
            {
                ptr = ptr->ptr;
                return *this;
            }
            parent_aware_binary_tree_node_proxy& go_right()noexcept
            {
                ptr = (ptr + 1)->ptr;
                return *this;
            }
            parent_aware_binary_tree_node_proxy& go_up()noexcept
            {
                ptr = (ptr + 2)->ptr;
                return *this;
            }
            T& operator*()const noexcept
            {
                return ((parent_aware_binary_tree_node<T>*)(parent_aware_binary_tree_node_base*)ptr)->data.data;
            }
            T* operator*()const noexcept
            {
                return std::addressof(((parent_aware_binary_tree_node<T>*)(parent_aware_binary_tree_node_base*)ptr)->data.data);
            }
            std::pair<parent_aware_binary_tree_edge_proxy<T>, parent_aware_binary_tree_edge_proxy<T>> get_children()const noexcept
            {
                std::pair<parent_aware_binary_tree_edge_proxy, parent_aware_binary_tree_edge_proxy> ret{ ptr, ptr + 1 };
                return ret;
            }
            operator parent_aware_binary_tree_edge_proxy<T>()const noexcept
            {
                parent_aware_binary_tree_node_proxy tmp = *this;
                tmp.go_up();
                auto [l, r] = tmp.get_children();
                return (static_cast<parent_aware_binary_tree_node_proxy>(l) == *this) ? l : r;
            }
            friend bool operator==(parent_aware_binary_tree_node_proxy l, parent_aware_binary_tree_node_proxy r)noexcept
            {
                return l.ptr == r.ptr;
            }
            friend struct std::hash<parent_aware_binary_tree_node_proxy>;
            friend class parent_aware_binary_tree_node_const_proxy<T>;
            using value_type = T;
        };
    }
}
template<class T>
struct std::hash<Yc::details::parent_aware_binary_tree_node_proxy<T>>
{
    size_t operator()(const Yc::details::parent_aware_binary_tree_node_proxy<T>& p)const noexcept
    {
        return std::hash<Yc::details::ptr_node*>{}(p.ptr);
    }
};
namespace Yc
{
    namespace details
    {
        template<class T>
        class parent_aware_binary_tree_node_const_proxy
        {
            ptr_node* ptr{};
        public:
            parent_aware_binary_tree_node_const_proxy() = default;
            parent_aware_binary_tree_node_const_proxy(const parent_aware_binary_tree_node_const_proxy&) = default;
            parent_aware_binary_tree_node_const_proxy& operator=(const parent_aware_binary_tree_node_const_proxy&) = default;
            parent_aware_binary_tree_node_const_proxy(ptr_node* ptr)noexcept :ptr{ ptr }
            {
            }
            parent_aware_binary_tree_node_const_proxy(parent_aware_binary_tree_edge_const_proxy<T> p)noexcept : ptr{ p.ptr->ptr }
            {
            }
            bool null()const noexcept
            {
                return ptr == nullptr;
            }
            explicit operator bool()const noexcept
            {
                return ptr;
            }
            parent_aware_binary_tree_node_const_proxy& go_left()noexcept
            {
                ptr = ptr->ptr;
                return *this;
            }
            parent_aware_binary_tree_node_const_proxy& go_right()noexcept
            {
                ptr = (ptr + 1)->ptr;
                return *this;
            }
            parent_aware_binary_tree_node_const_proxy& go_up()noexcept
            {
                ptr = (ptr + 2)->ptr;
                return *this;
            }
            const T& operator*()const noexcept
            {
                return ((parent_aware_binary_tree_node<T>*)(parent_aware_binary_tree_node_base*)ptr)->data.data;
            }
            const T* operator*()const noexcept
            {
                return std::addressof(((parent_aware_binary_tree_node<T>*)(parent_aware_binary_tree_node_base*)ptr)->data.data);
            }
            std::pair<parent_aware_binary_tree_edge_const_proxy<T>, parent_aware_binary_tree_edge_const_proxy<T>> get_children()const noexcept
            {
                std::pair<parent_aware_binary_tree_edge_const_proxy, parent_aware_binary_tree_edge_const_proxy> ret{ ptr, ptr + 1 };
                return ret;
            }
            operator parent_aware_binary_tree_edge_const_proxy<T>()const noexcept
            {
                parent_aware_binary_tree_node_const_proxy tmp = *this;
                tmp.go_up();
                auto [l, r] = tmp.get_children();
                return (static_cast<parent_aware_binary_tree_node_const_proxy>(l) == *this) ? l : r;
            }
            friend bool operator==(parent_aware_binary_tree_node_const_proxy l, parent_aware_binary_tree_node_const_proxy r)noexcept
            {
                return l.ptr == r.ptr;
            }
            friend struct std::hash<parent_aware_binary_tree_node_const_proxy>;
            using value_type = const T;
        };
    }
}
template<class T>
struct std::hash<Yc::details::parent_aware_binary_tree_node_const_proxy<T>>
{
    size_t operator()(const Yc::details::parent_aware_binary_tree_node_const_proxy<T>& p)const noexcept
    {
        return std::hash<Yc::details::ptr_node*>{}(p.ptr);
    }
};
namespace Yc
{
    namespace details
    {
        struct parent_aware_binary_tree_get_children_t
        {
            template<class T>
            auto operator()(parent_aware_binary_tree_edge_proxy<T> p)const noexcept
            {
                return p.get_children();
            }
            template<class T>
            auto operator()(parent_aware_binary_tree_edge_const_proxy<T> p)const noexcept
            {
                return p.get_children();
            }
            template<class T>
            auto operator()(parent_aware_binary_tree_node_proxy<T> p)const noexcept
            {
                return p.get_children();
            }
            template<class T>
            auto operator()(parent_aware_binary_tree_node_const_proxy<T> p)const noexcept
            {
                return p.get_children();
            }
        };
    }
    namespace parent_aware_binary_tree_functional
    {
        constexpr inline Yc::details::parent_aware_binary_tree_get_children_t get_children{};
    }
    template<class T, class Alloc = std::allocator<T>>
    class parent_aware_binary_tree
    {
        using node_ptr = details::parent_aware_binary_tree_node_base*;
        using node_type = details::parent_aware_binary_tree_node<T>;
        using node_allocator = std::allocator_traits<Alloc>::template rebind_alloc<node_type>;
        [[no_unique_address]] Alloc alloc;
        details::ptr_node root_node[2]{};
        node_ptr allocate_node()
        {
            node_allocator a{ alloc };
            node_ptr p = std::allocator_traits<node_allocator>::allocate(a, 1);
            new (p)node_type;
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
        using edge_proxy = details::parent_aware_binary_tree_edge_proxy<T>;
        using edge_const_proxy = details::parent_aware_binary_tree_edge_const_proxy<T>;
        using node_proxy = details::parent_aware_binary_tree_node_proxy<T>;
        using node_const_proxy = details::parent_aware_binary_tree_node_const_proxy<T>;
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
            ) {
            emplace(p, std::invoke(vg, h));
            //auto&& [lh, rh] = std::invoke(cg, h);
            (bool)h;
        }
        parent_aware_binary_tree recur_and_write(
            edge_const_proxy p,
            ValueGetter vg,
            ChildrenGetter cg,
            InitializeHandle h
        )
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
        parent_aware_binary_tree() = default;
        explicit parent_aware_binary_tree(const Alloc& a)noexcept :alloc{ a }
        {
        }
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
        parent_aware_binary_tree(const parent_aware_binary_tree& b) :parent_aware_binary_tree{ b.alloc }
        {
            auto copier = [](edge_const_proxy p) -> const T&
                {
                    return *p;
                };
            recur_and_write
            (root(), copier, binary_tree_funtional::get_children, b.croot());
        }
        parent_aware_binary_tree& operator=(const parent_aware_binary_tree& b)
        {
            Alloc tmp{ alloc };
            struct _guard
            {
                parent_aware_binary_tree* t{};
                Alloc* tmp{};
                ~_guard()
                {
                    if (t)
                    {
                        t->alloc = *tmp;
                    }
                }
            }guard{ this, &tmp };
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
        parent_aware_binary_tree(parent_aware_binary_tree&& b)noexcept :alloc{b.get_allocator()}
        {
            if (b.empty())
            {
                return;
            }
            root_node[0].ptr = std::exchange(b.root_node[0].ptr, {});
            (root_node[0].ptr->ptr + 2)->ptr = root_node;
        }
        parent_aware_binary_tree& operator=(parent_aware_binary_tree&& b)noexcept(
            std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value ||
            std::allocator_traits<Alloc>::is_always_equal::value)
        {
            if constexpr
                (std::allocator_traits<Alloc>::propagate_on_container_move_assignment::value)
            {
                using namespace std;
                swap(alloc, b.alloc);
                swap_sub_tree(root, b.root());
                return *this;
            }
            else
            {
                if (alloc == b.alloc)
                {
                    swap_sub_tree(root, b.root());
                    return *this;
                }
                auto vg = [](edge_proxy p) -> T&&
                {
                    return (T&&)*p;
                };
                parent_aware_binary_tree tmp{ vg, parent_aware_binary_tree_functional::get_children, b.root(), alloc };
                swap(tmp);
                return *this;
            }
        }
        bool empty()const noexcept
        {
            return root_node[0].ptr == nullptr;
        }
        edge_proxy root()noexcept
        {
            return { (details::ptr_node*)root_node };
        }
        edge_const_proxy root()const noexcept
        {
            return { (details::ptr_node*)root_node };
        }
        edge_const_proxy croot()const noexcept
        {
            return { (details::ptr_node*)root_node };
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
        
        parent_aware_binary_tree cut(edge_const_proxy p)noexcept
        {
            if (p.null())
            {
                return parent_aware_binary_tree{ alloc };
            }
            parent_aware_binary_tree ret{ alloc };
            root_node[0].ptr = std::exchange(p.ptr->ptr, {});
            (root_node[0].ptr->ptr + 2)->ptr = root_node;
        }
        void erase(edge_const_proxy p)noexcept
        {
            if (!p.null())
            {
                auto [l, r] = p.get_children();
                erase(l);
                erase(r);
                details::ptr_node* ptr = p.ptr;
                std::allocator_traits<Alloc>::destroy(alloc, (T*)p.operator->());
                deallocate_node((node_ptr)ptr->ptr);
                ptr->ptr = nullptr;
            }
        }
        void clear()noexcept
        {
            erase(root());
        }
        template<class... Args>
        parent_aware_binary_tree emplace(edge_const_proxy p, Args... args)
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
                    parent_aware_binary_tree* t;
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
            parent_aware_binary_tree ret{ cut(p) };
            details::ptr_node* ptr = p.ptr;
            ptr->ptr = new_node->ptr;
            new_node->ptr[2].ptr = details::find_this(ptr);
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
            parent_aware_binary_tree t1{ cut(from) }, ret{ cut(to) };
            details::ptr_node* ptr = to.ptr;
            ptr->ptr = std::exchange(t1.root_node[0].ptr, {});
            (ptr->ptr + 2)->ptr = details::find_this(ptr);
            return ret;
        }
        parent_aware_binary_tree splice(edge_const_proxy to, parent_aware_binary_tree& from)noexcept
        {
            parent_aware_binary_tree &t1 = from, ret{ cut(to) };
            details::ptr_node* ptr = to.ptr;
            ptr->ptr = std::exchange(t1.root_node[0].ptr, {});
            (ptr->ptr + 2)->ptr = details::find_this(ptr);
            return ret;
        }
        static void swap_sub_tree(edge_const_proxy l, edge_const_proxy r)noexcept
        {
            details::ptr_node *p1 = l.ptr , *p2 = r.ptr;
            if (p1->ptr == nullptr && p2->ptr == nullptr)
            {
                return;
            }
            if (p1->ptr == nullptr)
            {
                std::swap(p1->ptr, p2->ptr);
                p1->ptr[2] = details::find_this(p1);
                return;
            }
            if (p2->ptr == nullptr)
            {
                std::swap(p1->ptr, p2->ptr);
                p2->ptr[2] = details::find_this(p2);
                return;
            }
            std::swap(p1->ptr, p2->ptr);
            std::swap(p1->ptr[2], p2->ptr[2]);
        }
        void swap(parent_aware_binary_tree& other)noexcept
        {
            if constexpr (std::allocator_traits<Alloc>::propagate_on_container_swap::value)
            {
                using namespace std;
                swap(alloc, other.alloc);
            }
            swap_sub_tree(root(), other.root());
        }
        ~parent_aware_binary_tree()
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
            parent_aware_binary_tree t1 = cut(q); // <D>-<rC>-<E>
            parent_aware_binary_tree t2 = splice(p, t1.root()); //<B>-<rA>
            //     |
            //    <C>                 <A>
            //   /   \                /
            //  <D>  <E>            <B>
            q = p;
            q.go_left();
            parent_aware_binary_tree t3 = splice(q, t2.root());
            q.go_right();
            splice(q, t3.root());
        }
        void right_rotate(edge_const_proxy p)noexcept
        {
            edge_const_proxy q = p;
            q.go_left();
            parent_aware_binary_tree t1 = cut(q);
            parent_aware_binary_tree t2 = splice(p, t1.root());
            q = p;
            q.go_right();
            parent_aware_binary_tree t3 = splice(q, t2.root());
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
        static UnaryFunc pre_order_visit(node_proxy p, UnaryFunc func, Proj proj = {})
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
}