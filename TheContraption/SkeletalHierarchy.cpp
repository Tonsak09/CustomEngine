#include "SkeletonHierarchy.h"

#pragma region SkeletalHierarchy 

SkeletalHierarchy::SkeletalHierarchy(const aiScene* scene, aiBone** bones, int boneCount)
{
	GenerateHierachy(scene, bones, boneCount);
}

void SkeletalHierarchy::GenerateHierachy(const aiScene* scene, aiBone** bones, int boneCount)
{
	auto rootNode = scene->mRootNode;
	std::shared_ptr<B_Member> startPoint;

	for (int i = 0; i < boneCount; i++)
	{
		auto bone = bones[i];
		aiNode* bNode = FindNode(bone, rootNode->mChildren, rootNode->mNumChildren);
		std::string name = bNode->mName.C_Str();

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

			if (startPoint == nullptr)
			{
				startPoint = nameToBMember[name];
			}
		}
		else
		{
			// Set variables 
			nameToBMember[name]->SetBone(bone);
			nameToBMember[name]->SetBNode(bNode);
		}


		// Check if parent is not on bone list *********
		if (bNode->mParent == nullptr)
			return;

		// Does parent exist already in our hierarchy 
		std::string parentName = bNode->mParent->mName.C_Str();
		if (nameToBMember.find(parentName) == nameToBMember.end())
		{
			// Create parent member 
			nameToBMember[parentName] = std::make_shared<B_Member>(nullptr, nullptr);
		}
		
		nameToBMember[parentName]->AddChild(nameToBMember[name]);
	}

	// Can only search for 
	rootMember = GetRoot(startPoint);
	rootMember->RecurPrint(0);
}


/// <summary>
/// Iterates through some starting point in the tree in order to 
/// </summary>
/// <returns></returns>
std::shared_ptr<B_Member> SkeletalHierarchy::GetRoot(std::shared_ptr<B_Member> startPoint)
{
	std::shared_ptr<B_Member> current = startPoint;

	while (current->GetParent() != nullptr)
	{
		current = current->GetParent();
	}

	return current;
}

/// <summary>
/// Gets a mesh to be drawn with D3D11_PRIMITIVE_TOPOLOGY_LINELIST
/// </summary>
/// <returns></returns>
std::shared_ptr<Mesh> SkeletalHierarchy::GetMesh()
{
	return mesh;
}

/// <summary>
/// Iterate through this skeleton in order
/// to create a mesh that can be used to
/// render it with a line list. 
/// </summary>
void  SkeletalHierarchy::ConstructMesh(
	Microsoft::WRL::ComPtr<ID3D11Device> device,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context)
{
	// Create a vector of each bone 
	std::shared_ptr<std::vector<Vertex>> verticies =		std::make_shared<std::vector<Vertex>>();
	std::shared_ptr<std::vector<unsigned int>> indicies =	std::make_shared<std::vector<unsigned int>>();;


	// When adding a new bone to this vector
	// add the following two indicies to the 
	// indicies vector. Parent index and 
	// current index.

	// Recur so each child draws themself to
	// their child 
	
	
	verticies->push_back(B_MemberToVertex(nullptr, rootMember));
	RecurConstructIndicies(rootMember, verticies, indicies);

	mesh = std::make_shared<Mesh>(device, context,
		&(*verticies)[0],
		&(*indicies)[0],
		verticies->size(), indicies->size());
}

/// <summary>
/// Adds to the vectors the details of the current's
/// B_Member's children. Then applies this function 
/// to each child 
/// </summary>
/// <param name="current"></param>
/// <param name="verts"></param>
/// <param name="inds"></param>
void SkeletalHierarchy::RecurConstructIndicies(
	std::shared_ptr<B_Member> current,
	std::shared_ptr<std::vector<Vertex>> verts,
	std::shared_ptr<std::vector<unsigned int>> inds)
{
	int index = verts->size() - 1;

	for (auto& child : current->GetChildren())
	{
		verts->push_back(B_MemberToVertex(current, child));

		// Line pair between parent and child 
		inds->push_back(index);
		inds->push_back(verts->size() - 1);

		if (child->GetChildren().size() > 0)
		{
			RecurConstructIndicies(child, verts, inds);
		}
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

/// <summary>
/// Creates a vertex from a B_Member object. Used for constructing the 
/// skeleton mesh for debug rendering 
/// </summary>
/// <param name="member"></param>
/// <returns></returns>
Vertex SkeletalHierarchy::B_MemberToVertex(std::shared_ptr<B_Member> parent, std::shared_ptr<B_Member> member)
{
	Vertex vert = {};

	aiVector3D sca;
	aiVector3D rot;
	aiVector3D pos;
	aiMatrix4x4 mat;
	
	//if (parent != nullptr)
	//{
	//	mat = parent->GetBone()->mOffsetMatrix * member->GetBone()->mOffsetMatrix;
	//}
	//else
	//{
	//	// Root does not have a parent 
	//	mat = member->GetBone()->mOffsetMatrix;
	//}
	mat = member->GetBone()->mOffsetMatrix;
	mat.Decompose(sca, rot, pos);

	vert.Position = DirectX::XMFLOAT3(pos.x, pos.y, pos.z);
	return vert;
}

#pragma endregion 



#pragma region B_Member

B_Member::B_Member(aiBone* bone, aiNode* bNode) :
	bone(bone), bNode(bNode)
{
	children = std::vector<std::shared_ptr<B_Member>> ();
};

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

std::shared_ptr<B_Member> B_Member::GetParent()
{
	return parent;
}

std::vector<std::shared_ptr<B_Member>> B_Member::GetChildren()
{
	return children;
}

aiBone* B_Member::GetBone()
{
	return bone;
}

aiNode* B_Member::GetNode()
{
	return bNode;
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
		printf("   ");
	}

	printf(bone->mName.C_Str());
	printf("\n");

	for (auto child : children)
	{
		child->RecurPrint(layer + 1);
	}
}

#pragma endregion