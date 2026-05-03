#pragma once
#include <memory>
#include <type_traits>
namespace Yc
{
    template<class T, class Cookie>
    class value_with_cookie
    {
        static_assert(std::is_trivially_copyable_v<Cookie> && std::is_trivially_constructible_v<Cookie>);
        union u_t
        {
            T value;
            ~u_t() {}
        }u;
        mutable Cookie cookie_value{};
    public:
        T& value()noexcept
        {
            return u.value;
        }
        const T& value()const noexcept
        {
            return u.value;
        }

        Cookie& cookie()const noexcept
        {
            return cookie_value;
        }
    };

    // 主模板
    template<class Allocator ,class = void>
    class allocator_with_cookie_base
    {
    public:
        allocator_with_cookie_base() = default;
        allocator_with_cookie_base(const Allocator&)noexcept
        {
        }
        static Allocator get_allocator()noexcept
        {
            return {};
        }
    };

    // 有状态分配器的特化
    template<class Allocator>
    class allocator_with_cookie_base
    <
        Allocator,
        std::void_t<std::enable_if_t<!std::allocator_traits<Allocator>::is_always_equal::value>>
    >
    {
        Allocator alloc{};
    public:
        allocator_with_cookie_base() = default;
        allocator_with_cookie_base(const Allocator& a)noexcept :alloc{a}
        {
        }
        Allocator& get_allocator()noexcept
        {
            return alloc;
        }

        const Allocator& get_allocator()const noexcept
        {
            return alloc;
        }
    };

    // 从一般的分配器制作value_with_cookie类型的分配器
    template<class U, class Allocator, class Cookie>
    class allocator_with_cookie : public allocator_with_cookie_base<Allocator>
    {
        // 如果是原分配器的带cookie的分配器，那么为true
        static constexpr inline bool is_cookie_allocator =
            std::is_same_v<U, value_with_cookie<typename std::allocator_traits<Allocator>::value_type, Cookie>>;
        using infact_allocator = std::allocator_traits<Allocator>::rebind_alloc<U>;
    public:
        template<class T>
        allocator_with_cookie(const allocator_with_cookie<T, Allocator, Cookie>& a)noexcept :allocator_with_cookie_base<Allocator>{a.get_allocator()}
        {
        }
        using allocator_with_cookie_base::allocator_with_cookie_base;
        using pointer = std::allocator_traits<infact_allocator>::pointer;
        using const_pointer = std::allocator_traits<infact_allocator>::const_pointer;
        using void_pointer = std::allocator_traits<infact_allocator>::void_pointer;
        using const_void_pointer = std::allocator_traits<infact_allocator>::const_void_pointer;
        using value_type = U;
        using size_type = std::allocator_traits<infact_allocator>::size_type;
        using difference_type = std::allocator_traits<infact_allocator>::difference_type;
        using propagate_on_container_copy_assignment = std::allocator_traits<infact_allocator>::propagate_on_container_copy_assignment;
        using propagate_on_container_move_assignment = std::allocator_traits<infact_allocator>::propagate_on_container_move_assignment;
        using propagate_on_container_swap = std::allocator_traits<infact_allocator>::propagate_on_container_swap;
        // rebind由allocator_traits自动化处理，这里不提供
        allocator_with_cookie select_on_container_copy_construction()const noexcept
        {
            auto&& alloc = get_allocator();
            return { std::allocator_traits<Allocator>::select_on_container_copy_construction(alloc) };
        }

        auto allocate(size_type n)
        {
            auto&& alloc = get_allocator();
            return std::allocator_traits<infact_allocator>::allocate(alloc, n);
        }

        auto allocate(size_type n, const_void_pointer cvp)
        {
            auto&& alloc = get_allocator();
            return std::allocator_traits<infact_allocator>::allocate(alloc, n, cvp);
        }

        auto allocate_at_least(size_type n)
        {
            auto&& alloc = get_allocator();
            return std::allocator_traits<infact_allocator>::allocate_at_least(alloc, n);
        }

        void deallocate(pointer p, size_type n)noexcept
        {
            auto&& alloc = get_allocator();
            std::allocator_traits<infact_allocator>::deallocate(alloc, p, n);
        }

        auto max_size()const noexcept
        {
            auto&& alloc = get_allocator();
            return std::allocator_traits<infact_allocator>::max_size(alloc);
        }

        template<class T, class... Args>
        void construct(T* xp, Args&&... args)
        {
            if constexpr (is_cookie_allocator && std::is_same_v<T, U>)
            {
                auto&& alloc = get_allocator();
                value_with_cookie<typename std::allocator_traits<Allocator>::value_type, cookie>* p = xp;
                std::allocator_traits<Allocator>::construct(alloc, std::addressof(p->value()), std::forward<Args>(args)...);
            }
            else
            {
                typename std::allocator_traits<Allocator>::template rebind_alloc<T> alloc{ get_allocator() };
                typename std::allocator_traits<Allocator>::template rebind_traits<T>::construct(alloc, xp, std::forward<Args>(args)...);
            }
        }

        template<class T>
        void destroy(T* xp)
        {
            if constexpr (is_cookie_allocator && std::is_same_v<T, U>)
            {
                auto&& alloc = get_allocator();
                value_with_cookie<typename std::allocator_traits<Allocator>::value_type, cookie>* p = xp;
                std::allocator_traits<Allocator>::destroy(alloc, std::addressof(p->value()));
            }
            else
            {
                typename std::allocator_traits<Allocator>::template rebind_alloc<T> alloc{ get_allocator() };
                typename std::allocator_traits<Allocator>::template rebind_traits<T>::destroy(alloc, xp);
            }
        }

        friend bool operator==(allocator_with_cookie, allocator_with_cookie) = default;
    };

}