// ============================================================
//  ComponentPool.h
// ------------------------------------------------------------
//  le ComponentPool<T> stocke TOUS les composants d'un meme type T
//  (ex tous les Transform) pour toutes les entités qui en ont un.
//
//  structure utilisée : "sparse set". 3 tableaux :
//
//     _components : les composants, collés les uns aux autres (pas de trous)
//     _owners     : _owners[i] = l'entité a qui appartient _components[i]
//     _sparse     : _sparse[entité] = position de son composant dans _components
//                   (ou NONE si l'entité n'a pas ce composant)
//
//     exemple : entités 2 et 5 ont un Spin
//        _components = [ spin_de_2 , spin_de_5 ]
//        _owners     = [     2     ,     5     ]
//        _sparse     = [NONE, NONE, 0, NONE, NONE, 1]   (index = numero d'entité)
//
//  avantages : has / get / add / remove en O(1), et parcourir tous les composants
//  est tres rapide car ils sont contigus en memoire.
//
//  utilisé par : Registry (un pool par type de composant)
// ============================================================
#pragma once
#include "ecs/Entity.h"
#include <vector>
#include <utility>


// ------------------------------------------------------------
//  IPool : interface commune a tous les pools
//  probleme : ComponentPool<Transform> et ComponentPool<Spin> sont des types
//  DIFFERENTS, donc le Registry pourrait pas les mettre dans un meme tableau.
//  solution : ils heritent tous de IPool, et le Registry stocke des pointeurs IPool.
//  le destructeur virtual est obligatoire : sinon quand le unique_ptr<IPool> detruit
//  le pool, le destructeur du vrai ComponentPool<T> serait pas appelé (fuite memoire)
// ------------------------------------------------------------
class IPool
{
public:
	virtual ~IPool() = default;
	virtual void remove(Entity entity) = 0;   // =0 : chaque pool doit l'implementer (permet de supprimer sans connaitre T)

};


template<typename T>
class ComponentPool : public IPool
{
private:

	static constexpr uint32_t NONE = UINT32_MAX;   // valeur "pas de composant" dans _sparse
	std::vector<T> _components;       // les composants, sans trous
	std::vector<Entity> _owners;      // _owners[i] = proprietaire de _components[i] (meme indices)
	std::vector<uint32_t> _sparse;    // entité -> index dans _components (ou NONE)

public:

	// ------------------------------------------------------------
	//  add : attache un composant a l'entité e
	//  1) _sparse est agrandi si l'entité a un numero plus grand que sa taille
	//     (les nouvelles cases sont remplies avec NONE)
	//  2) on note a quel index le composant va etre rangé
	//  3) on ajoute l'entité et le composant a la fin des tableaux
	//  renvoie une reference sur le composant stocké
	//  attention : la reference peut devenir invalide si on ajoute un autre composant du meme type
	//  (le vector peut se reallouer), donc faut pas la garder longtemps
	//  attention 2 : pas de verification si e a deja ce composant
	// ------------------------------------------------------------
	T& add(Entity e, T comp) {
		if (e >= _sparse.size()) _sparse.resize(e + 1, NONE);
		_sparse[e] = (uint32_t)_components.size();
		_owners.push_back(e);
		_components.push_back(std::move(comp));
		return _components.back();
	}

	// l'entité a t'elle ce composant ? on verifie d'abord que e rentre dans _sparse
	// (sinon on lirait en dehors du tableau) puis que la case n'est pas NONE
	bool has(Entity e) const {
		return e < _sparse.size() && _sparse[e] != NONE;
	}

	// renvoie le composant de l'entité e : _sparse donne l'index, _components donne la donnée
	// pas de verification ici : faut appeler has() avant si on est pas sur (Registry::each le fait)
	T& get(Entity e) {
		return _components[_sparse[e]];
	}

	// ------------------------------------------------------------
	//  remove : retire le composant de l'entité e
	//  si on faisait un erase() au milieu du vector, tous les composants suivants seraient
	//  decalés (lent) et il faudrait corriger _sparse pour chacun.
	//  astuce "swap and pop" : on met le DERNIER composant a la place de celui qu'on supprime,
	//  puis on retire la derniere case. un seul composant bouge, pas de trou.
	// ------------------------------------------------------------
	void remove(Entity e) override {

		if (!has(e)) return;      // rien a supprimer
		uint32_t index = _sparse[e];                          // ou est le composant a supprimer
		uint32_t lastIndex = (uint32_t)_components.size() - 1; // ou est le dernier


		// si c'est deja le dernier pas besoin de deplacer quoi que ce soit
		if (index != lastIndex) {
			_components[index] = std::move(_components[lastIndex]);   // le dernier composant prend la place
			Entity lastOwner = _owners[lastIndex];                    // a qui appartenait le dernier ?
			_owners[index] = lastOwner;                               // son proprietaire suit son composant
			_sparse[lastOwner] = index;                               // et _sparse doit pointer sur sa nouvelle position
		}
		// on enleve la derniere case (soit l'ancien dernier deja copié, soit celui qu'on voulait supprimer)
		_components.pop_back();
		_owners.pop_back();
		_sparse[e] = NONE;       // e n'a plus ce composant
	}

	// liste des entités qui ont ce composant, utilisée par Registry::each pour savoir qui parcourir
	const std::vector<Entity>& owners() const { return _owners; }

};