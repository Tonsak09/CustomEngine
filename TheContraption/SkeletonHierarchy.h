#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>
#include <unordered_map>
#include <string>

#include <memory>

class SkeletalHierarchy
{
public:
	SkeletalHierarchy();
	SkeletalHierarchy(aiNode* root);
	SkeletalHierarchy(aiScene* scene, aiBone** bones);

	/// <summary>
	/// Using the array of bones iterates through the scene to create a hierarchy 
	/// based on the bone names and comparing them with scene node names 
	/// </summary>
	/// <param name="bones"></param>
	void GenerateHierachy(aiScene* scene, aiBone** bones);

	B_Member* GetRoot();

	// Indexer to get by string name 

private:
	// Holds pointers to all bones 
	std::unordered_map<std::string, std::shared_ptr<B_Member>> nameToBMember;
};

/// <summary>
/// Represents a single bone within the hierarchy 
/// </summary>
struct B_Member
{
public:
	B_Member(aiBone* bone, std::shared_ptr<B_Member> parent, std::vector<std::shared_ptr<B_Member>> children);

	void AddChild(std::shared_ptr<B_Member> bone);
	void RemoveChild(std::shared_ptr<B_Member> child);

	std::shared_ptr<B_Member> GetParent();
	std::vector<std::shared_ptr<B_Member>> GetChildren();

private:
	std::shared_ptr<B_Member> parent;
	std::vector<std::shared_ptr<B_Member>> children;
};