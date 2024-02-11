#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>
#include <unordered_map>
#include <string>

#include <memory>

/// <summary>
/// Represents a single bone within the hierarchy 
/// </summary>
struct B_Member
{
public:
	/*B_Member(aiBone* bone, std::shared_ptr<B_Member> parent, std::vector<std::shared_ptr<B_Member>> children);
	B_Member(aiBone* bone, std::vector<std::shared_ptr<B_Member>> children);
	B_Member(aiBone* bone, std::shared_ptr<B_Member> parent);*/
	B_Member(aiBone* bone, aiNode* bNode);

	void AddChild(std::shared_ptr<B_Member> bone);
	void RemoveChild(std::shared_ptr<B_Member> child);

	std::shared_ptr<B_Member> GetParent();
	std::vector<std::shared_ptr<B_Member>> GetChildren();

	void SetBone(aiBone* bone);
	void SetBNode(aiNode* bNode);

	void RecurPrint(int layer = 0);
private:

	std::shared_ptr<B_Member> parent;
	std::vector<std::shared_ptr<B_Member>> children;

	aiBone* bone;
	aiNode* bNode;
};

class SkeletalHierarchy
{
public:
	SkeletalHierarchy();
	SkeletalHierarchy(aiNode* root);
	SkeletalHierarchy(const aiScene* scene, aiBone** bones, int boneCount);

	/// <summary>
	/// Using the array of bones iterates through the scene to create a hierarchy 
	/// based on the bone names and comparing them with scene node names 
	/// </summary>
	/// <param name="bones"></param>
	void GenerateHierachy(const aiScene* scene, aiBone** bones, int boneCount);

	B_Member* GetRoot();

	// Indexer to get by string name 

private:
	aiNode* FindNode(aiBone* bone, aiNode** nodes, int nodeCount);

	// Holds pointers to all bones 
	std::unordered_map<std::string, std::shared_ptr<B_Member>> nameToBMember;

	std::shared_ptr<B_Member > rootMember;
};

