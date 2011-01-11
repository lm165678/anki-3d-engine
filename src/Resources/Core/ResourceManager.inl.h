#include <boost/type_traits.hpp>
#include "Exception.h"


//======================================================================================================================
// loadRsrc                                                                                                            =
//======================================================================================================================
template<typename Type>
typename ResourceManager::Types<Type>::Info& ResourceManager::load(const char* filename)
{
	// Chose container
	typename Types<Type>::Container& c = choseContainer<Type>();

	// Find
	typename Types<Type>::Iterator it = find<Type>(filename, c);

	// If already loaded
	if(it != c.end())
	{
		++it->referenceCounter;
	}
	// else create new, load it and update the container
	else
	{
		Type* newInstance = NULL;

		try
		{
			newInstance = new Type();
			newInstance->load(filename);
		}
		catch(std::exception& e)
		{
			if(newInstance != NULL)
			{
				delete newInstance;
			}
			//throw EXCEPTION("Cannot load resource: " + e.what());
		}

		c.push_back(typename Types<Type>::Info());
		it = c.end();
		--it;

		it->uuid = filename;
		it->referenceCounter = 1;
		it->resource = newInstance;
	}

	return *it;
}


//======================================================================================================================
// unload                                                                                                              =
//======================================================================================================================
template<typename Type>
void ResourceManager::unloadR(const typename Types<Type>::Info& info)
{
	// Chose container
	typename Types<Type>::Container& c = choseContainer<Type>();

	// Find
	typename Types<Type>::Iterator it = find<Type>(info.resource, c);

	// If not found
	if(it == c.end())
	{
		throw EXCEPTION("Resource hook incorrect (\"" + info.uuid + "\")");
	}

	RASSERT_THROW_EXCEPTION(it->uuid != info.uuid);
	RASSERT_THROW_EXCEPTION(it->referenceCounter != info.referenceCounter);

	--it->referenceCounter;

	// Delete the resource
	if(it->referenceCounter == 0)
	{
		delete it->resource;
		c.erase(it);
	}
}


//======================================================================================================================
// find [char*]                                                                                                        =
//======================================================================================================================
template<typename Type>
typename ResourceManager::Types<Type>::Iterator
ResourceManager::find(const char* filename, typename Types<Type>::Container& container)
{
	typename Types<Type>::Iterator it = container.begin();
	for(; it != container.end(); it++)
	{
		if(it->uuid == filename)
		{
			break;
		}
	}

	return it;
}


//======================================================================================================================
// find [Type*]                                                                                                        =
//======================================================================================================================
template<typename Type>
typename ResourceManager::Types<Type>::Iterator
ResourceManager::find(const Type* resource, typename Types<Type>::Container& container)
{
	typename Types<Type>::Iterator it = container.begin();
	for(; it != container.end(); it++)
	{
		if(it->resource == resource)
		{
			break;
		}
	}

	return it;
}