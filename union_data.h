#pragma once
#include <type_traits>
namespace Yc
{
	template<class T, class = void>
	union data_t
	{
		T data;
		data_t() {}
		~data_t() {}
	};
	template<class T>
	union data_t<T, std::enable_if_t<
		std::is_trivially_constructible_v<T>&&
		std::is_trivially_copyable_v<T>&&
		std::is_trivially_destructible_v<T>>>
	{
		T data;
	};
}