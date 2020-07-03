
#ifndef BaseModel_hpp
#define BaseModel_hpp

#include <stdio.h>
#include <GL/glew.h>
#include <list>
#include "camera.h"
#include "matrix.h"
#include "baseshader.h"
#include "Aabb.h"
#include "hit.h"

class HitInfo;

class BaseModel
{
public:
    BaseModel();
    virtual ~BaseModel();
    virtual void draw(const BaseCamera& Cam);
	const Matrix& getTransform() const;
	virtual const Matrix localToWorld() const;
	virtual const Matrix getGlobalTransform() const;
	virtual void setTransform(const Matrix& m) { transform = m; }
    virtual void shader( BaseShader* shader, bool deleteOnDestruction=false );
    virtual BaseShader* shader() const { return pShader; }
	virtual const AABB& boundingBox() const { return AABB::unitBox(); }
	unsigned int getTransparency() const { return transparency; }
	void setTransparency(unsigned int t) { transparency = t; }
	bool deleteShaderOnDestruction() const { return deleteShader; }
	std::string getName() const { return name; }
	void setName(std::string name) { this->name = name; }
	virtual void activate() {};
	virtual void deactivate() {};
	virtual void rayCollision(const Vector3& start, const Vector3& direction, const float& distance, HitInfo& info);
	const BaseModel* getParent() const { return parent; }
	const std::list<BaseModel*> getChildren() const { return children; }
	void setParent(BaseModel* parent);

	const bool IsSelectable() const { return selectable; }
	void SetSelectable(bool state) { selectable = state; }
	const bool IsSelected() const { return selected; }
	void Select(bool state = true) { selected = state; }
	void Unselect() { selected = false; }
	const bool Disabled() const { return disabled; }
	void Disabled(bool state) { disabled = state; }
	const bool AlwaysOnTop() const { return onTop; }
	void AlwaysOnTop(bool state) { onTop = state; }
protected:
	BaseModel* parent;
	std::list<BaseModel*> children;
    Matrix transform;
    BaseShader* pShader;
    bool deleteShader;
	//bool ShadowReceiver;
	unsigned int transparency;
	std::string name;
	bool selectable;
	bool selected;
	bool disabled;
	bool onTop;
};


#endif /* BaseModel_hpp */
