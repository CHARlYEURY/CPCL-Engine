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
	// independant a chaque type de comp
	static constexpr uint32_t NONE = UINT32_MAX;
	std::vector<T> _components; // contient les composant 
	std::vector<Entity> _owners; // contient l'entity qui a le composant 
	std::vector<uint32_t> _sparse; // contient la position et si il y a un composant du pool ou non 

public: 

	T& add(Entity e, T comp) {
		if (e >= _sparse.size()) _sparse.resize(e + 1, NONE); // agrandie si pas la place
		_sparse[e] = (uint32_t)_components.size();
		_owners.push_back(e); // parallelle avec component en taille 
		_components.push_back(std::move(comp)); // comp stocker dans le vector 
		return _components.back();
	}

	bool has (Entity e) const {
		return e < _sparse.size() && _sparse[e] != NONE;
	}

	T& get(Entity e) {
		return _components[_sparse[e]]; // donne la position et le comp 
		// attention a le faire uniquement sur un entité qui a un comp sinon on est hors limite (logique) et sa casse, faire un has avant pour tester
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
		_sparse[e] = NONE; // on diminue pas la taille de sparce, simplement on met la position a NONE car le component n'est plus la
	}

	const std::vector<Entity>& owners() const { return _owners; }

};


