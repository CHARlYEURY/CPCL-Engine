#pragma once
#include "ecs/Entity.h"
#include <vector>
#include <utility>



class IPool
{
public:
	virtual ~IPool() = default;
	virtual void remove(Entity entity) = 0;

};

template<typename T>
class ComponentPool : public IPool
{
private:

	static constexpr uint32_t NONE = UINT32_MAX;
	std::vector<T> _components;
	std::vector<Entity> _owners;
	std::vector<uint32_t> _sparse;

public: 

	T& add(Entity e, T comp) {
		if (e >= _sparse.size()) _sparse.resize(e + 1, NONE);
		_sparse[e] = (uint32_t)_components.size();
		_owners.push_back(e);
		_components.push_back(std::move(comp));
		return _components.back();
	}

	bool has (Entity e) const {
		return e < _sparse.size() && _sparse[e] != NONE;
	}

	T& get(Entity e) {
		return _components[_sparse[e]];
	}

	void remove(Entity e) override {

		if (!has(e)) return;
		uint32_t index = _sparse[e];
		uint32_t lastIndex = (uint32_t)_components.size() - 1;


		// swap and pop pour éviter les trou, prend le dernier index et le met a la place de celui qui est remove

		if (index != lastIndex) {
			_components[index] = std::move(_components[lastIndex]);
			Entity lastOwner = _owners[lastIndex];
			_owners[index] = lastOwner;
			_sparse[lastOwner] = index;
		}
		_components.pop_back();
		_owners.pop_back();
		_sparse[e] = NONE;
	}

	const std::vector<Entity>& owners() const { return _owners; }

};


