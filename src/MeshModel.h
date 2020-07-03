#pragma once

#include <stdio.h>
#include "basemodel.h"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "vertexbuffer.h"
#include "indexbuffer.h"
#include "texture.h"
#include "aabb.h"
#include "lineboxmodel.h"
#include <string>

class AttachedModel;

class MeshModel : public BaseModel
{
public:
    struct Material
    {
        Material() : DiffTex(NULL), NormalTex(NULL), DiffColor(1, 1, 1), SpecColor(0.3f, 0.3f, 0.3f), AmbColor(0, 0, 0), SpecExp(10) {}
        Color DiffColor;
        Color SpecColor;
        Color AmbColor;
        float SpecExp;
        const Texture* DiffTex;
        const Texture* NormalTex;
    };
    struct Node
    {
        Node() : Parent(NULL), Children(NULL), ChildCount(0), MeshCount(0), Meshes(NULL) {}
        Matrix Trans;
        Matrix GlobalTrans;
        int* Meshes;
        unsigned int MeshCount;
        Node* Parent;
        Node* Children;
        unsigned int ChildCount;
        std::string Name;
    };
	struct Mesh
	{
		Mesh() : MaterialIdx(-1), node(NULL) {}
		VertexBuffer VB;
		IndexBuffer IB;
		int MaterialIdx;
		Node* node;
	};
    struct JointInfo
    {
        JointInfo() : jointID(0), offset(Matrix::identity), transform(Matrix::identity), localTransform(Matrix::identity), globalTransform(Matrix::identity), node(NULL) {}
        int jointID;
        Matrix offset;
        Matrix transform;
		Matrix localTransform;
		Matrix globalTransform;
        const Node* node;
    };

    MeshModel();
    MeshModel(const char* ModelFile, bool FitSize=false);
    virtual ~MeshModel();

    virtual bool load(const char* ModelFile, bool FitSize=false);
    virtual void draw(const BaseCamera& Cam);
    virtual void setTransform(const Matrix& m);
    void updateMeshTransforms();
    void updateMeshTransforms(Node* node, const Matrix& globalMat);
    const AABB& boundingBox() const { return BoundingBox; }
	void setForward(Vector3 forward);
	std::string getPath();
	virtual void rayCollision(const Vector3& start, const Vector3& direction, const float& range, HitInfo& info);
    const Node& GetRoot() const;
	const Node* GetNode(std::string name, const Node* node = NULL) const;
	const Mesh& GetMesh(int meshID) const;
	std::string GetFilename() const { return Filename; }
	Matrix GetInverseMeshTransform() const { return inverseMeshTransform; }
	std::string GetFilepath() const { return Path; }
	const JointInfo* GetJointInfo(std::string name) const;
protected: // protected methods
	void loadFaces(const aiMesh* mMesh, Mesh* pMesh);
	void loadBones(const aiMesh* mMesh, Mesh* pMesh, const int vertexID);
	void loadPosition(const aiMesh* mMesh, Mesh* pMesh, const int vertexID, const Vector3 scale);
	void loadTangentAndBitangent(const aiMesh* mMesh, Mesh* pMesh, const int vertexID);
	void loadNormal(const aiMesh* mMesh, Mesh* pMesh, const int vertexID);
	void loadTextureCoords(const aiMesh* mMesh, Mesh* pMesh, const int vertexID, const int channelID);
	void loadMesh(const aiScene* pScene, const int meshID, const Vector3 scale);
    void loadMeshes(const aiScene* pScene, bool FitSize);
    void loadMaterials(const aiScene* pScene);
    void calcBoundingBox( const aiScene* pScene, AABB& Box);

    void loadNodes(const aiScene* pScene);
    void copyNodesRecursive(const aiNode* paiNode, Node* pNode);
    Matrix convertAiMatrix4x4(const aiMatrix4x4& m);
    void applyMaterial( unsigned int index);
    void deleteNodes(Node* pNode);

protected: // protected member variables
    Mesh* pMeshes;
    unsigned int MeshCount;
    Material* pMaterials;
    unsigned int MaterialCount;
    AABB BoundingBox;

	const unsigned int* indices;
	const Vector3* vertices;
    
    std::string Filepath; // stores pathname and filename
    std::string Path; // stores path without filename
	std::string Filename; //stores filename
    Node RootNode;
    std::map<std::string, JointInfo> jointMapping;
	Matrix inverseMeshTransform;
};