#include "Systems/HierarchySystem.h"

#include <algorithm>
#include <numeric>

#include "Components/HierarchyComponent.h"
#include "Components/TransformComponent.h"
#include "Managers/ComponentManager.h"

void HierarchySystem::update(ComponentManager<LocalTransformComponent>& inLocalTransforms
	, ComponentManager<WorldTransformComponent>& inWorldTransforms
	, ComponentManager<HierarchyComponent>& inHierarchies)
{
	// 1. Collect roots and group children
	std::vector<size_t> roots;
	// index = hierarchycomponent index
	std::vector<std::vector<Entity>> children(inHierarchies.size());
	for (size_t i = 0; i < inHierarchies.size(); ++i)
	{
		HierarchyComponent* hierarchy = inHierarchies.at(i);
		ensure(hierarchy);

		if (!EntityFuncs::isEntityValid(hierarchy->parent))
		{
			roots.push_back(i);
			continue;
		}

		size_t elementIndex = inHierarchies.getIndex(hierarchy->parent);
		children[elementIndex].push_back(hierarchy->entity);
	}

	// 2. initialize depth for roots
	unsigned int currentDepth = 0;
	std::vector<unsigned int> depths(inHierarchies.size(), 0);
	std::queue<size_t> hierarchyIndexesToCheck;
	for (auto& root : roots)
	{
		hierarchyIndexesToCheck.push(root);
		depths[root] = currentDepth;
	}

	// 3. fill depth in children based on their parent's depth
	while (hierarchyIndexesToCheck.size() > 0)
	{
		size_t parentIndex = hierarchyIndexesToCheck.front();
		hierarchyIndexesToCheck.pop();

		const HierarchyComponent* parentHierarchyComponent = inHierarchies.at(parentIndex);
		if (!parentHierarchyComponent)
		{
			continue;
		}

		Entity childEntity = parentHierarchyComponent->firstChild;
		while (EntityFuncs::isEntityValid(childEntity))
		{
			size_t childIndex = inHierarchies.getIndex(childEntity);
			depths[childIndex] = depths[parentIndex] + 1;
			hierarchyIndexesToCheck.push(childIndex);

			const HierarchyComponent* childHierarchyComponent = inHierarchies.at(childIndex);
			childEntity = childHierarchyComponent->nextSibling;
		}
	}


	// 4. sort indexes, order by depth then by parent
	std::vector<unsigned int> sortedIndexes(inHierarchies.size());
	std::iota(sortedIndexes.begin(), sortedIndexes.end(), 0);
	std::stable_sort(sortedIndexes.begin(), sortedIndexes.end()
		, ([&depths, &inHierarchies](unsigned int a, unsigned int b)
			{
				if (depths[a] < depths[b])
				{
					return true;
				}

				return inHierarchies.at(a)->parent < inHierarchies.at(b)->parent;
			}));

	// 5. create an index lookup table
	std::vector<size_t> old_to_new_index(inHierarchies.size());
	for (size_t i = 0; i < sortedIndexes.size(); ++i)
	{
		size_t oldIndex = sortedIndexes[i];
		old_to_new_index[oldIndex] = (int)i;
	}

	// 6. create an ordered hierarchies array and replace the old one
	DenseArray<HierarchyComponent> newHierarchies;
	newHierarchies.initialize(inHierarchies.size());
	newHierarchies.setSize(inHierarchies.size());
	for (size_t i = 0; i < old_to_new_index.size(); ++i)
	{
		size_t elementNewIndex = old_to_new_index[i];
		newHierarchies.at(elementNewIndex) = *inHierarchies.at(i);
	}

	inHierarchies.moveData(std::move(newHierarchies));
	inHierarchies.rebuildLookupTable();

	// 7. local transforms is a parallel array, apply the same order
	DenseArray<LocalTransformComponent> newLocalTransforms;
	newLocalTransforms.initialize(inHierarchies.size());
	newLocalTransforms.setSize(inHierarchies.size());
	for (size_t i = 0; i < old_to_new_index.size(); ++i)
	{
		size_t elementNewIndex = old_to_new_index[i];
		newLocalTransforms.at(elementNewIndex) = *inLocalTransforms.at(i);
	}

	inLocalTransforms.moveData(std::move(newLocalTransforms));
	inLocalTransforms.rebuildLookupTable();

	// 8. same for world transforms
	DenseArray<WorldTransformComponent> newWorldTransforms;
	newWorldTransforms.initialize(inHierarchies.size());
	newWorldTransforms.setSize(inHierarchies.size());
	for (size_t i = 0; i < old_to_new_index.size(); ++i)
	{
		size_t elementNewIndex = old_to_new_index[i];
		newWorldTransforms.at(elementNewIndex) = *inWorldTransforms.at(i);
	}

	inWorldTransforms.moveData(std::move(newWorldTransforms));
	inWorldTransforms.rebuildLookupTable();
}