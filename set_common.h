#pragma once
namespace Yc
{
    namespace details
    {
        template<class T, class Alloc>
        struct set_iterator
        {
            using tree_type = parent_aware_binary_tree<value_with_cookie<T, size_t>,
                allocator_with_cookie<value_with_cookie<T, size_t>, Alloc, size_t>>;
            typename tree_type::node_const_proxy p;
            set_iterator() = default;
            set_iterator(const set_iterator&) = default;
            set_iterator(typename tree_type::node_const_proxy p)noexcept :p{ p }
            {}
            set_iterator& operator=(const set_iterator&) = default;
            friend bool operator==(set_iterator, set_iterator) = default;
            set_iterator& operator++()noexcept
            {
                auto [l, r] = p.get_children();
                if (r)
                {
                    p.go_right();
                    while (true)
                    {
                        auto q = p;
                        q.go_left();
                        if (!q)
                        {
                            return *this;
                        }
                        p = q;
                    }
                }
                else
                {
                    while (true)
                    {
                        auto q = p;
                        p.go_up();
                        auto r = p;
                        r.go_left();
                        if (q == r)
                        {
                            return *this;
                        }
                    }
                }
            }

            set_iterator operator++(int)noexcept
            {
                set_iterator ret = *this;
                ++*this;
                return ret;
            }

            set_iterator& operator--()noexcept
            {
                auto [l, r] = p.get_children();
                if (l)
                {
                    p.go_left();
                    while (true)
                    {
                        auto q = p;
                        q.go_right();
                        if (!q)
                        {
                            return *this;
                        }
                        p = q;
                    }
                }
                else
                {
                    while (true)
                    {
                        auto q = p;
                        p.go_up();
                        auto r = p;
                        r.go_right();
                        if (q == r)
                        {
                            return *this;
                        }
                    }
                }
            }

            set_iterator operator--(int)noexcept
            {
                set_iterator ret = *this;
                --*this;
                return ret;
            }

            const T& operator*()const noexcept
            {
                return p->value();
            }

            const T* operator&()const noexcept
            {
                return std::addressof(p->value());
            }
        };
    }
}