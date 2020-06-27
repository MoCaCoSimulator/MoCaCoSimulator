#include "hit.h"

HitInfo& HitInfo::operator=(const HitInfo& other)
{
	position = other.position;
	distance = other.distance;
	model = other.model;
	meshID = other.meshID;
	hit = other.hit;
	triangleInfo = other.triangleInfo;

	return (*this);
}
