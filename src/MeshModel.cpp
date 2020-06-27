//
//  Model.cpp
//  ogl4
//
//  Created by Philipp Lensing on 21.09.16.
//  Copyright © 2016 Philipp Lensing. All rights reserved.
//

#include "MeshModel.h"
#include "phongshader.h"
#include "vector.h"
#include <list>
#include "AttachedModel.h"
#include "Customizable/JointnameParser.h"
#define FITSCALE 4.f

MeshModel::MeshModel() : indices(NULL), vertices(NULL), pMeshes(NULL), MeshCount(0), pMaterials(NULL), MaterialCount(0), inverseMeshTransform(Matrix::identity)
{
}

MeshModel::MeshModel(const char* ModelFile, bool FitSize) : pMeshes(NULL), MeshCount(0), pMaterials(NULL), MaterialCount(0), inverseMeshTransform(Matrix::identity)
{
    bool ret = load(ModelFile, FitSize);
    if(!ret)
        throw std::exception();
}

MeshModel::~MeshModel()
{
	if(this->MeshCount>0)
		delete[] this->pMeshes;
	if(this->MaterialCount>0)
		delete[] this->pMaterials;
    deleteNodes(&RootNode);
}

void MeshModel::deleteNodes(Node* pNode)
{
    if(!pNode)
        return;
    for(unsigned int i=0; i<pNode->ChildCount; ++i)
        deleteNodes(&(pNode->Children[i]));
    if(pNode->ChildCount>0)
        delete [] pNode->Children;
    if(pNode->MeshCount>0)
        delete [] pNode->Meshes;
}

void MeshModel::updateMeshTransforms()
{
	updateMeshTransforms(&RootNode, getGlobalTransform());
}

void MeshModel::updateMeshTransforms(Node* node, const Matrix& globalMat)
{
	node->GlobalTrans = globalMat * node->Trans;

	for (unsigned int i = 0; i < node->ChildCount; ++i)
		updateMeshTransforms(&node->Children[i], node->GlobalTrans);
}

void MeshModel::rayCollision(const Vector3& start, const Vector3& direction, const float& range, HitInfo& info)
{
	std::list<Node*> nodes;
	nodes.push_back(&RootNode);
	//float rangeSquared = range * range;

	while (!nodes.empty())
	{
		Node* pNode = nodes.front();
		/*
		if (pNode->Parent != NULL)
			pNode->GlobalTrans = pNode->Parent->GlobalTrans * pNode->Trans;
		else
			pNode->GlobalTrans = getGlobalTransform() * pNode->Trans;
			*/
		for (unsigned int i = 0; i < pNode->MeshCount; ++i)
		{
			int meshID = pNode->Meshes[i];
			Mesh& mesh = pMeshes[meshID];
			std::vector<unsigned int> indices = mesh.IB.indices();
			std::vector<Vector3> vertices = mesh.VB.vertices();

			for (int j = 0; j < indices.size(); j+=3)
			{
				unsigned int indexA = indices[j];
				unsigned int indexB = indices[j + 1];
				unsigned int indexC = indices[j + 2];

				Vector3 a = pNode->GlobalTrans * vertices[indexA];
				Vector3 b = pNode->GlobalTrans * vertices[indexB];
				Vector3 c = pNode->GlobalTrans * vertices[indexC];

				/*
				if ((start - a).lengthSquared() > rangeSquared)
					&& (start - b).lengthSquared() > rangeSquared)
					&& (start - c).lengthSquared() > rangeSquared)
					continue;
				*/

				float s;
				bool hit = start.triangleIntersection(direction, a, b, c, s);

				//continue if no hit, hit out of range or distance further away than previous
				if (!hit || s >= range || s >= info.distance)
					continue;

				info.position = start + direction * s;
				info.distance = s;
				info.model = this;
				info.meshID = meshID;
				info.hit = true;
				info.triangleInfo.indexA = indexA;
				info.triangleInfo.indexB = indexB;
				info.triangleInfo.indexC = indexC;
				info.triangleInfo.posA = a;
				info.triangleInfo.posB = b;
				info.triangleInfo.posC = c;
			}
		}

		for (unsigned int i = 0; i < pNode->ChildCount; ++i)
			nodes.push_back(&(pNode->Children[i]));
		nodes.pop_front();
	}
}

const MeshModel::Node& MeshModel::GetRoot() const
{
	return RootNode;
}

const MeshModel::Node* MeshModel::GetNode(std::string name, const Node* node) const
{
	if (!node)
		node = &RootNode;

	if (node->Name == name)
		return node;

	for (unsigned int i = 0; i < node->ChildCount; ++i)
	{
		const MeshModel::Node* found = GetNode(name, &node->Children[i]);
		if (found)
			return found;
	}

	return NULL;
}

const MeshModel::Mesh& MeshModel::GetMesh(int meshID) const
{
	if (meshID > this->MeshCount)
		assert(0);

	return this->pMeshes[meshID];
}

bool MeshModel::load(const char* ModelFile, bool FitSize)
{
	qDebug() << "load" << ModelFile;
    const aiScene* pScene = aiImportFile( ModelFile, aiProcessPreset_TargetRealtime_Fast | aiProcess_TransformUVCoords);

    if(pScene==NULL || pScene->mNumMeshes<=0)
        return false;

    Filepath = ModelFile;
    Path = Filepath;
    size_t pos = Filepath.rfind('/');
    if(pos == std::string::npos)
        pos = Filepath.rfind('\\');
    if(pos != std::string::npos)
        Path.resize(pos+1);
	Filename = Filepath.substr(pos + 1, Filepath.length() - 1);

    loadMeshes(pScene, FitSize);
    loadMaterials(pScene);
    loadNodes(pScene);

	inverseMeshTransform = Matrix(RootNode.Trans).invert();

	aiReleaseImport(pScene);
	updateMeshTransforms();

    return true;
}

void MeshModel::loadNormal(const aiMesh* mMesh, Mesh* pMesh, const int vertexID)
{
	aiVector3D vec = mMesh->mNormals[vertexID];
	pMesh->VB.addNormal(vec.x, vec.y, vec.z);
}

void MeshModel::loadTextureCoords(const aiMesh* mMesh, Mesh* pMesh, const int vertexID, const int channelID)
{
	aiVector3D vec = mMesh->mTextureCoords[channelID][vertexID];

	switch (channelID) {
	case 0:
		pMesh->VB.addTexcoord0(vec.x, -vec.y);
		break;
	case 1:
		pMesh->VB.addTexcoord1(vec.x, -vec.y);
		break;
	default:
		//TODO
		break;
	}
}

void MeshModel::loadTangentAndBitangent(const aiMesh* mMesh, Mesh* pMesh, const int vertexID)
{
	aiVector3D vec;

	vec = mMesh->mTangents[vertexID];
	pMesh->VB.addTexcoord2(vec.x, vec.y, vec.z);
	vec = mMesh->mBitangents[vertexID];
	pMesh->VB.addTexcoord3(vec.x, vec.y, vec.z);
}

void MeshModel::loadPosition(const aiMesh* mMesh, Mesh* pMesh, const int vertexID, Vector3 scale)
{
	aiVector3D vec = mMesh->mVertices[vertexID];
	vec.x *= scale.x;
	vec.y *= scale.y;
	vec.z *= scale.z;
	pMesh->VB.addVertex(vec.x, vec.y, vec.z);
}

void MeshModel::loadBones(const aiMesh* aiMesh, Mesh* pMesh, const int vertexID)
{
	VertexBuffer::JointWeights weights;
	aiBone* bone;

	for (int i = 0; i < aiMesh->mNumBones; i++)
	{
		bone = aiMesh->mBones[i];
		std::string boneName = JointnameParser::ExtractJointName(bone->mName);
		JointInfo info;

		if (jointMapping.find(boneName) == jointMapping.end())
		{
			info.jointID = jointMapping.size();
			info.offset = convertAiMatrix4x4(bone->mOffsetMatrix);
			//info.identityOffset = Matrix(info.offset).invert();
			jointMapping[boneName] = info;
		}
		else
			info = jointMapping[boneName];

		for (int j = 0; j < bone->mNumWeights; j++)
		{
			aiVertexWeight weight = bone->mWeights[j];

			if (weight.mVertexId - 1 == vertexID)
				weights.AddWeight(info.jointID, weight.mWeight);
		}
	}

	pMesh->VB.addJointWeights(weights);
}

const MeshModel::JointInfo* MeshModel::GetJointInfo(std::string name) const
{
	if (jointMapping.find(name) == jointMapping.end())
		return NULL;
	return &jointMapping.at(name);
}

void MeshModel::loadFaces(const aiMesh* mMesh, Mesh* pMesh)
{
	for (unsigned int j = 0; j < mMesh->mNumFaces; ++j)
	{
		aiFace& face = mMesh->mFaces[j];
		for (unsigned int k = 0; k < face.mNumIndices; ++k)
			pMesh->IB.addIndex(face.mIndices[k]);
	}
}

void MeshModel::loadMesh(const aiScene* pScene, const int meshID, const Vector3 scale)
{
	aiMesh* mMesh = pScene->mMeshes[meshID];
	Mesh& pMesh = this->pMeshes[meshID];

	pMesh.VB.begin();
	for (unsigned int vertexID = 0; vertexID < mMesh->mNumVertices; ++vertexID)
	{
		pMesh.MaterialIdx = mMesh->mMaterialIndex;
		if (mMesh->HasNormals())
			loadNormal(mMesh, &pMesh, vertexID);

		for (unsigned int uvChannel = 0; uvChannel < mMesh->GetNumUVChannels(); ++uvChannel)
			if (mMesh->HasTextureCoords(uvChannel))
				loadTextureCoords(mMesh, &pMesh, vertexID, uvChannel);

		if (mMesh->HasTangentsAndBitangents())
			loadTangentAndBitangent(mMesh, &pMesh, vertexID);

		if (mMesh->HasPositions())
			loadPosition(mMesh, &pMesh, vertexID, scale);

		if (mMesh->HasBones())
			loadBones(mMesh, &pMesh, vertexID);
	}
	pMesh.VB.end();

	pMesh.IB.begin();
	if (mMesh->HasFaces())
		loadFaces(mMesh, &pMesh);
	pMesh.IB.end();
}

void MeshModel::loadMeshes(const aiScene* pScene, bool FitSize)
{
	this->calcBoundingBox(pScene, this->BoundingBox);
	this->MeshCount = pScene->mNumMeshes;
	this->pMeshes = new Mesh[this->MeshCount];
	Vector3 scale( 1, 1, 1 );

	if (FitSize)
	{
		Vector3 size = this->BoundingBox.size();
		scale = Vector3(FITSCALE / size.x, FITSCALE / size.y, FITSCALE / size.z);
	}

	for (unsigned int meshID = 0; meshID < pScene->mNumMeshes; ++meshID) {
		loadMesh(pScene, meshID, scale);
	}

	if (FitSize) {
		Vector3 min, max, vec;
		std::vector<Vector3> vecs;
		for (unsigned int i = 0; i < MeshCount; ++i)
		{
			vecs = pMeshes[i].VB.vertices();
			for (unsigned int j = 0; j < vecs.size(); ++j)
			{
				vec = vecs.at(j);
				if (vec.x < min.x || (j == 0 && i == 0)) min.x = vec.x;
				if (vec.y < min.y || (j == 0 && i == 0)) min.y = vec.y;
				if (vec.z < min.z || (j == 0 && i == 0)) min.z = vec.z;
				if (vec.x > max.x || (j == 0 && i == 0)) max.x = vec.x;
				if (vec.y > max.y || (j == 0 && i == 0)) max.y = vec.y;
				if (vec.z > max.z || (j == 0 && i == 0)) max.z = vec.z;
			}
		}
		this->BoundingBox = AABB(min, max);
	}
}

void MeshModel::loadMaterials(const aiScene* pScene)
{
	this->MaterialCount = pScene->mNumMaterials;
	if (!pScene->HasMaterials())
		return;

	this->pMaterials = new Material[pScene->mNumMaterials];
	for (unsigned int i = 0; i < pScene->mNumMaterials; ++i)
	{
		aiMaterial *mMaterial = pScene->mMaterials[i];
		Material &pMaterial = this->pMaterials[i];
		aiColor3D col;

		//unused (on purpose)
		mMaterial->Get(AI_MATKEY_COLOR_AMBIENT, col);
		pMaterial.AmbColor = Color(col.r, col.g, col.b);

		mMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, col);
		pMaterial.DiffColor = Color(col.r, col.g, col.b);
		mMaterial->Get(AI_MATKEY_COLOR_SPECULAR, col);
		pMaterial.SpecColor = Color(col.r, col.g, col.b);

		mMaterial->Get(AI_MATKEY_SHININESS, pMaterial.SpecExp);

		for (unsigned int k = 0; k < mMaterial->GetTextureCount(aiTextureType_DIFFUSE); ++k)
		{
			aiString TexS;
			mMaterial->GetTexture(aiTextureType_DIFFUSE, k, &TexS);
			
			std::string fileName = this->Path + std::string(TexS.C_Str());
			pMaterial.DiffTex = Texture::LoadShared(fileName.c_str());

			break;
		}
		
		for (unsigned int i = 0; i < mMaterial->GetTextureCount(aiTextureType_NORMALS); i++)
		{
			aiString TexS;
			mMaterial->GetTexture(aiTextureType_NORMALS, i, &TexS);

			std::string fileName = this->Path + std::string(TexS.C_Str());
			pMaterial.NormalTex = Texture::LoadShared(fileName.c_str());

			break;
		}
	}
}

void MeshModel::calcBoundingBox(const aiScene* pScene, AABB& Box)
{
	Vector3 min, max;
	aiVector3D vec;
	aiMesh *mMesh;

	for (unsigned int i = 0; i < pScene->mNumMeshes; ++i) {
		mMesh = pScene->mMeshes[i];
		for (unsigned int j = 0; j < mMesh->mNumVertices; ++j)
		{
			if (!mMesh->HasPositions()) continue;
			vec = mMesh->mVertices[j];
			if (vec.x < min.x || (j == 0 && i == 0)) min.x = vec.x;
			if (vec.y < min.y || (j == 0 && i == 0)) min.y = vec.y;
			if (vec.z < min.z || (j == 0 && i == 0)) min.z = vec.z;
			if (vec.x > max.x || (j == 0 && i == 0)) max.x = vec.x;
			if (vec.y > max.y || (j == 0 && i == 0)) max.y = vec.y;
			if (vec.z > max.z || (j == 0 && i == 0)) max.z = vec.z;
		}
	}

	Box = AABB(min, max);
}

void MeshModel::loadNodes(const aiScene* pScene)
{
    deleteNodes(&RootNode);
    copyNodesRecursive(pScene->mRootNode, &RootNode);
}

void MeshModel::copyNodesRecursive(const aiNode* paiNode, Node* pNode)
{
	pNode->Name = JointnameParser::ExtractJointName(paiNode->mName);
    pNode->Trans = convertAiMatrix4x4(paiNode->mTransformation);
	
	if (jointMapping.find(pNode->Name) != jointMapping.end())
		jointMapping[pNode->Name].node = pNode;

    if(paiNode->mNumMeshes > 0)
    {
        pNode->MeshCount = paiNode->mNumMeshes;
        pNode->Meshes = new int[pNode->MeshCount];
		for (unsigned int i = 0; i < pNode->MeshCount; ++i)
		{
			int meshID = (int)paiNode->mMeshes[i];
			pNode->Meshes[i] = meshID;
			pMeshes[meshID].node = pNode;
		}
    }

    if(paiNode->mNumChildren <=0)
        return;

    pNode->ChildCount = paiNode->mNumChildren;
    pNode->Children = new Node[pNode->ChildCount];
    for(unsigned int i=0; i<paiNode->mNumChildren; ++i)
    {
        copyNodesRecursive(paiNode->mChildren[i], &(pNode->Children[i]));
        pNode->Children[i].Parent = pNode;
    }
}

void MeshModel::applyMaterial( unsigned int index)
{
    if(index>=MaterialCount)
        return;

    PhongShader* pPhong = dynamic_cast<PhongShader*>(shader());

    if(!pPhong)
        return;

    Material* pMat = &pMaterials[index];
    pPhong->ambientColor(pMat->AmbColor);
    pPhong->diffuseColor(pMat->DiffColor);
    pPhong->specularExp(pMat->SpecExp);
    pPhong->specularColor(pMat->SpecColor);
    pPhong->diffuseTexture(pMat->DiffTex);
	pPhong->normalTexture(pMat->NormalTex);
}

void MeshModel::draw(const BaseCamera& Cam)
{
	updateMeshTransforms();

    if(!pShader) {
        std::cout << "Model::draw() no shader found" << std::endl;
        return;
    }

    std::list<Node*> DrawNodes;
    DrawNodes.push_back(&RootNode);

    while(!DrawNodes.empty())
    {
        Node* pNode = DrawNodes.front();
		/*
        if(pNode->Parent != NULL)
            pNode->GlobalTrans = pNode->Parent->GlobalTrans * pNode->Trans;
        else
            pNode->GlobalTrans = getGlobalTransform() * pNode->Trans;
		*/
        pShader->modelTransform(pNode->GlobalTrans);

        for(unsigned int i = 0; i<pNode->MeshCount; ++i )
        {
            Mesh& mesh = pMeshes[pNode->Meshes[i]];
            mesh.VB.activate();
            mesh.IB.activate();
            applyMaterial(mesh.MaterialIdx);
			pShader->activate(Cam);
			this->activate();
            glDrawElements(GL_TRIANGLES, mesh.IB.indexCount(), mesh.IB.indexFormat(), 0);
			this->deactivate();
			pShader->deactivate();
            mesh.IB.deactivate();
            mesh.VB.deactivate();
        }
        for(unsigned int i = 0; i<pNode->ChildCount; ++i )
            DrawNodes.push_back(&(pNode->Children[i]));

        DrawNodes.pop_front();
    }
}

void MeshModel::setTransform(const Matrix& m)
{
	BaseModel::setTransform(m);
	updateMeshTransforms();
}

void MeshModel::setForward(Vector3 forward)
{
	//Set passed vector
	transform.forward(forward.normalize());
	//Calc right vector from crossing global up vector
	transform.right(Vector3::up.cross(forward).normalize());
	//Calc up vector from crossing right with forward Vector
	transform.up(forward.cross(transform.right()).normalize());
}

std::string MeshModel::getPath()
{
	return Filepath;
}

Matrix MeshModel::convertAiMatrix4x4(const aiMatrix4x4& m)
{
    return Matrix(m.a1, m.a2, m.a3, m.a4,
                  m.b1, m.b2, m.b3, m.b4,
                  m.c1, m.c2, m.c3, m.c4,
                  m.d1, m.d2, m.d3, m.d4);
}
