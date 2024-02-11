#include "SkeletonHierarchy.h"

#pragma region SkeletalHierarchy 

SkeletalHierarchy::SkeletalHierarchy(const aiScene* scene, aiBone** bones, int boneCount)
{
	GenerateHierachy(scene, bones, boneCount);
}

void SkeletalHierarchy::GenerateHierachy(const aiScene* scene, aiBone** bones, int boneCount)
{
	auto rootNode = scene->mRootNode;
	//printf("%i", rootNode->mNumChildren);

	for (int i = 0; i < boneCount; i++)
	{
		auto bone = bones[i];
		aiNode* bNode = FindNode(bone, rootNode->mChildren, rootNode->mNumChildren);
		std::string name = bNode->mName.C_Str();

		// Print out node parent and children 
		printf("\t   Name: ");
		printf(name.c_str());
		printf("\t   Parent: ");
		printf(bNode->mParent->mName.C_Str());
		printf("\n");



		// Does this node already exist
		if (nameToBMember.find(bNode->mParent->mName.C_Str()) == nameToBMember.end())
		{
			// Create member
			nameToBMember[bone->mName.C_Str()] = std::make_shared<B_Member>(bone, bNode);
		}
		else
		{
			// Set variables 
		}



		// Is this bone a root, segement, or leaf
		if (bNode->mParent == nullptr)
			return;

		// Does parent exist 
		std::string parentName = bNode->mParent->mName.C_Str();
		if (nameToBMember.find(parentName) == nameToBMember.end())
		{
			// Create parent member 
			nameToBMember[parentName] = std::make_shared<B_Member>(nullptr, nullptr);
		}
		
		nameToBMember[parentName]->AddChild(nameToBMember[name]);

		/*printf("\t   Name: ");
		printf(bone->mName.C_Str());
		printf("\t   Child Count: ");
		printf("%i", bone->mNode->mNumChildren);

		printf("\n");
		printf("\t   NodeStatus: ");
		printf("\n");*/
	}
}

/// <summary>
/// Recursively travel through node children to find the
/// </summary>
/// <returns></returns>
aiNode* SkeletalHierarchy::FindNode(aiBone* bone, aiNode** nodes, int nodeCount)
{
	for (int i = 0; i < nodeCount; i++)
	{
		auto node = nodes[i];

		// Compare names 
		if (bone->mName == node->mName)
		{
			return node;
		}

		// Check Children 
		aiNode* childResult = nullptr;
		if (node->mNumChildren > 0)
			childResult = FindNode(bone, node->mChildren, node->mNumChildren);

		if (childResult != nullptr)
			return childResult;
	}

	return nullptr;
}

#pragma endregion 



#pragma region B_Member

B_Member::B_Member(aiBone* bone, aiNode* bNode) :
	bone(bone), bNode(bNode) {};

void B_Member::AddChild(std::shared_ptr<B_Member> bone)
{

}

void B_Member::RemoveChild(std::shared_ptr<B_Member> child)
{

}

void B_Member::SetBone(aiBone* bone)
{

}

void B_Member::SetBNode(aiNode* bNode)
{

}


#pragma endregion