#include "BaseModel.h"
#include "Paths.h"
#include <algorithm>

BaseModel::BaseModel() : 
	pShader(NULL), 
	parent(NULL),
	deleteShader(false), 
	transparency(TRANSPARENCY_NONE), 
	name("unnamed"), 
	selected(false), 
	selectable(false), 
	disabled(false),
	onTop(false)
{
    transform.setIdentity();
}

BaseModel::~BaseModel()
{
	printf("deleting %s\n", name.c_str());
    if(deleteShader && pShader != NULL)
        delete pShader;

	if (parent)
		parent->children.remove(this);
}

void BaseModel::shader( BaseShader* shader, bool deleteOnDestruction )
{
    pShader = shader;
    deleteShader = deleteOnDestruction;
}

void BaseModel::draw(const BaseCamera& Cam)
{
    if(!pShader) {
        std::cout << "BaseModel::draw() no shader found" << std::endl;
        return;
    }
    
    pShader->modelTransform(getGlobalTransform());
    pShader->activate(Cam);
}

const Matrix& BaseModel::getTransform() const
{
	return transform;
}

const Matrix BaseModel::localToWorld() const
{
	if (parent != nullptr)
		return parent->getGlobalTransform();
	return Matrix::identity;
}

const Matrix BaseModel::getGlobalTransform() const
{
	return localToWorld() * getTransform();
}

void BaseModel::rayCollision(const Vector3& start, const Vector3& direction, const float& distance, HitInfo& info)
{
}

void BaseModel::setParent(BaseModel* parent)
{
	if (!parent)
		return;

	this->parent = parent;
	parent->children.push_back(this);
}
