#pragma once

#include <memory>



template <typename T>
struct SharedPtrSingleton
{
	using valute_type = T;
	using sp_t        = std::shared_ptr<valute_type>;

	static
	sp_t GetSpInstance()
	{
		static sp_t sp_instance;
		if (not sp_instance) { sp_instance.reset(new valute_type{}); }
		return sp_instance;
	}
};
