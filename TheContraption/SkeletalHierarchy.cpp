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
		/*printf("\t   Name: ");
		printf(name.c_str());
		printf("\t   Parent: ");
		printf(bNode->mParent->mName.C_Str());
		printf("\n");*/

		/*
			Members may already exist since when we create
			a member it must be attached as child to another
			member. The only exception is the root which we
			take care of intially. 
			
			Created parents that are set before they are searched 
			for are not set with a bone or bnode member so they 
			must be set in future when they are found. 
		*/

		// Does this member already exist
		if (nameToBMember.find(name) == nameToBMember.end())
		{
			// Create member
			nameToBMember[name] = std::make_shared<B_Member>(bone, bNode);
		}
		else
		{
			// Set variables 
			nameToBMember[name]->SetBone(bone);
			nameToBMember[name]->SetBNode(bNode);
		}



		// Do not continue if this is the root node 
		if (bNode->mParent == nullptr)
		{
			// Current is root 
			rootMember = nameToBMember[name];
			return;
		}

		// Does parent exist already in our hierarchy 
		std::string parentName = bNode->mParent->mName.C_Str();
		if (nameToBMember.find(parentName) == nameToBMember.end())
		{
			// Create parent member 
			nameToBMember[parentName] = std::make_shared<B_Member>(nullptr, nullptr);
		}
		
		nameToBMember[parentName]->AddChild(nameToBMember[name]);
	}

	rootMember->RecurPrint();
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

void B_Member::AddChild(std::shared_ptr<B_Member> member)
{
	children.push_back(member);
}

void B_Member::RemoveChild(std::shared_ptr<B_Member> child)
{
	std::vector<std::shared_ptr<B_Member>>::iterator it;
	it = std::find(children.begin(), children.end(), child);

	if (it != children.end())
	{
		children.erase(it);
	}
}

void B_Member::SetBone(aiBone* bone)
{
	this->bone = bone;
}

void B_Member::SetBNode(aiNode* bNode)
{
	this->bNode = bNode;
}

void B_Member::RecurPrint(int layer)
{
	for (int i = 0; i < layer; i++)
	{
		printf("\t");
	}

	printf(bNode->mName.C_Str());

	for (auto child : children)
	{
		child->RecurPrint(layer + 1);
	}
}

#pragma endregion