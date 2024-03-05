#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>
#include <unordered_map>
#include <string>

#include "Vertex.h"
#include "Mesh.h"

#include <memory>
#include "Transform.h"

using namespace DirectX;

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

	aiBone* GetBone();
	aiNode* GetNode();

	void SetBone(aiBone* bone);
	void SetBNode(aiNode* bNode);
	void SetPosition(XMFLOAT3 position);
	void SetRotationEuler(XMFLOAT3 rotation);


	void RecurPrint(int layer = 0);

	void ResetTransformToLoad();

private:

	std::shared_ptr<B_Member> parent;
	std::vector<std::shared_ptr<B_Member>> children;

	aiBone* bone;
	aiNode* bNode;

	std::shared_ptr<Transform> transform;
	std::shared_ptr<DirectX::XMMATRIX> loadTransform;
};

class SkeletalHierarchy
{
public:
	//SkeletalHierarchy();
	//SkeletalHierarchy(aiNode* root);
	SkeletalHierarchy(const aiScene* scene, aiBone** bones, int boneCount);
	void ConstructMesh(
		Microsoft::WRL::ComPtr<ID3D11Device> device, 
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);
	void UpdateMesh(
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context);

	/// <summary>
	/// Using the array of bones iterates through the scene to create a hierarchy 
	/// based on the bone names and comparing them with scene node names 
	/// </summary>
	/// <param name="bones"></param>
	void GenerateHierachy(const aiScene* scene, aiBone** bones, int boneCount);
	void UpdateMember(std::string memberName, XMFLOAT3 position, XMFLOAT4 rotation);

	std::shared_ptr<B_Member> GetRoot(std::shared_ptr<B_Member> startPoint);
	std::shared_ptr<Mesh> GetMesh();

	// TODO: Indexer to get by string name 
private:

	void RecurConstructIndicies(
		std::shared_ptr<B_Member> current,
		std::shared_ptr<std::vector<Vertex>> verts,
		std::shared_ptr<std::vector<unsigned int>> inds );

	aiNode* FindNode(aiBone* bone, aiNode** nodes, int nodeCount);

	Vertex B_MemberToVertex(std::shared_ptr<B_Member> parent, std::shared_ptr<B_Member> member);

	// Holds pointers to all bones 
	std::unordered_map<std::string, std::shared_ptr<B_Member>> nameToBMember;
	std::shared_ptr<B_Member > rootMember;

	std::shared_ptr<std::vector<Vertex>> verticies;
	std::shared_ptr<std::vector<unsigned int>> indicies;
	std::shared_ptr<Mesh> mesh;

	std::unordered_map<std::string, int> memberNameToVertexIndex;
};

