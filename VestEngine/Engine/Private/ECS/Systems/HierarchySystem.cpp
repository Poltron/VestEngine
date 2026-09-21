#include "ECS/Systems/HierarchySystem.h"

#include <algorithm>
#include <numeric>

#include "tracy/Tracy.hpp"

#include "Core/Scene.h"
#include "ECS/ComponentManager.h"
#include "ECS/Components/HierarchyComponent.h"
#include "ECS/Components/TransformComponent.h"

void HierarchySystem::update(Scene& inScene)
{
	ZoneScoped;

	ComponentManager<HierarchyComponent>& hierarchyComponents = inScene.getHierarchyComponents();

	// 0. Check if we can skip this system
	bool bNeedsUpdate = false;
	for (size_t i = 0; i < hierarchyComponents.size(); ++i)
	{
		bNeedsUpdate |= hierarchyComponents.at(i)->bDirty;
	}

	if (!bNeedsUpdate)
	{
		return;
	}

	// 1. Collect roots and group children
	std::vector<size_t> roots;
	// index = hierarchycomponent index
	std::vector<std::vector<Entity>> children(hierarchyComponents.size());
	for (size_t i = 0; i < hierarchyComponents.size(); ++i)
	{
		HierarchyComponent* hierarchy = hierarchyComponents.at(i);
		ensure(hierarchy);

		if (!EntityFuncs::isEntityValid(hierarchy->parent))
		{
			roots.push_back(i);
			continue;
		}

		size_t elementIndex = hierarchyComponents.getIndex(hierarchy->parent);
		children[elementIndex].push_back(hierarchy->entity);
	}

	// 2. initialize depth for roots
	unsigned int currentDepth = 0;
	std::vector<unsigned int> depths(hierarchyComponents.size(), 0);
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

		const HierarchyComponent* parentHierarchyComponent = hierarchyComponents.at(parentIndex);
		if (!parentHierarchyComponent)
		{
			continue;
		}

		Entity childEntity = parentHierarchyComponent->firstChild;
		while (EntityFuncs::isEntityValid(childEntity))
		{
			size_t childIndex = hierarchyComponents.getIndex(childEntity);
			depths[childIndex] = depths[parentIndex] + 1;
			hierarchyIndexesToCheck.push(childIndex);

			const HierarchyComponent* childHierarchyComponent = hierarchyComponents.at(childIndex);
			childEntity = childHierarchyComponent->nextSibling;
		}
	}


	// 4. sort indexes, order by depth then by parent
	std::vector<unsigned int> sortedIndexes(hierarchyComponents.size());
	std::iota(sortedIndexes.begin(), sortedIndexes.end(), 0);
	std::stable_sort(sortedIndexes.begin(), sortedIndexes.end()
		, ([&depths, &hierarchyComponents](unsigned int a, unsigned int b)
			{
				if (depths[a] < depths[b])
				{
					return true;
				}

				return hierarchyComponents.at(a)->parent < hierarchyComponents.at(b)->parent;
			}));

	// 5. create an index lookup table
	std::vector<size_t> old_to_new_index(hierarchyComponents.size());
	for (size_t i = 0; i < sortedIndexes.size(); ++i)
	{
		size_t oldIndex = sortedIndexes[i];
		old_to_new_index[oldIndex] = (int)i;
	}

	// 6. create an ordered hierarchies array and replace the old one
	DenseArray<HierarchyComponent> newHierarchies;
	newHierarchies.initialize(hierarchyComponents.size());
	newHierarchies.setSize(hierarchyComponents.size());
	for (size_t i = 0; i < old_to_new_index.size(); ++i)
	{
		size_t elementNewIndex = old_to_new_index[i];
		newHierarchies.at(elementNewIndex) = *hierarchyComponents.at(i);
	}

	hierarchyComponents.moveData(std::move(newHierarchies));
	hierarchyComponents.rebuildLookupTable();

	ComponentManager<LocalTransformComponent>& localTransforms = inScene.getLocalTransformComponents();

	// 7. local transforms is a parallel array, apply the same order
	DenseArray<LocalTransformComponent> newLocalTransforms;
	newLocalTransforms.initialize(hierarchyComponents.size());
	newLocalTransforms.setSize(hierarchyComponents.size());
	for (size_t i = 0; i < old_to_new_index.size(); ++i)
	{
		size_t elementNewIndex = old_to_new_index[i];
		newLocalTransforms.at(elementNewIndex) = *localTransforms.at(i);
	}

	localTransforms.moveData(std::move(newLocalTransforms));
	localTransforms.rebuildLookupTable();

	ComponentManager<WorldTransformComponent>& worldTransforms = inScene.getWorldTransformComponents();

	// 8. same for world transforms
	DenseArray<WorldTransformComponent> newWorldTransforms;
	newWorldTransforms.initialize(hierarchyComponents.size());
	newWorldTransforms.setSize(hierarchyComponents.size());
	for (size_t i = 0; i < old_to_new_index.size(); ++i)
	{
		size_t elementNewIndex = old_to_new_index[i];
		newWorldTransforms.at(elementNewIndex) = *worldTransforms.at(i);
	}

	worldTransforms.moveData(std::move(newWorldTransforms));
	worldTransforms.rebuildLookupTable();
}