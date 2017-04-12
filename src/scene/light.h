#ifndef __LIGHT_H__
#define __LIGHT_H__

#include "scene.h"

class Light
	: public SceneElement
{
public:
	virtual vec3f shadowAttenuation(const vec3f& P) const = 0;
	virtual double distanceAttenuation( const vec3f& P ) const = 0;
	virtual vec3f getColor( const vec3f& P ) const = 0;
	virtual vec3f getDirection( const vec3f& P ) const = 0;

protected:
	Light( Scene *scene, const vec3f& col )
		: SceneElement( scene ), color( col ) {}
	vec3f 		color;
};

class DirectionalLight
	: public Light
{
public:
	DirectionalLight( Scene *scene, const vec3f& orien, const vec3f& color )
		: Light( scene, color ), orientation( orien ) {}
	virtual vec3f shadowAttenuation(const vec3f& P) const;
	virtual double distanceAttenuation( const vec3f& P ) const;
	virtual vec3f getColor( const vec3f& P ) const;
	virtual vec3f getDirection( const vec3f& P ) const;

protected:
	vec3f 		orientation;
};

class PointLight
	: public Light
{
public:
	PointLight( Scene *scene, const vec3f& pos, const vec3f& color, const double& cac, const double& lac, const double& qac)
		: Light( scene, color ), position( pos ), cac(cac), lac(lac), qac(qac) {}
	virtual vec3f shadowAttenuation(const vec3f& P) const;
	virtual double distanceAttenuation( const vec3f& P ) const;
	virtual vec3f getColor( const vec3f& P ) const;
	virtual vec3f getDirection( const vec3f& P ) const;

protected:
	vec3f position;
	double cac, lac, qac; // constant_attenuation_coeff, linear_attenuation_coeff, quadratic_attenuation_coeff
};


class AmbientLight
	: public Light
{
public:
	AmbientLight(Scene *scene, const vec3f& color)
		: Light(scene, color) {}
	virtual vec3f shadowAttenuation(const vec3f& P) const;
	virtual double distanceAttenuation(const vec3f& P) const;
	virtual vec3f getColor(const vec3f& P) const;
	virtual vec3f getDirection(const vec3f& P) const;
};

class SpotLight
	: public Light // inherit pointlight will be better?
{
public:
	SpotLight(Scene *scene, const vec3f& pos, const vec3f& d, const double& a, const vec3f& color, const double& cac, const double& lac, const double& qac)
		: Light(scene, color), position(pos), direction(d), angle(a), cac(cac), lac(lac), qac(qac) {}
	virtual vec3f shadowAttenuation(const vec3f& P) const;
	virtual double distanceAttenuation(const vec3f& P) const;
	virtual vec3f getColor(const vec3f& P) const;
	virtual vec3f getDirection(const vec3f& P) const;

protected:
	vec3f position, direction;
	double angle; // coneAngle
	double cac, lac, qac; // constant_attenuation_coeff, linear_attenuation_coeff, quadratic_attenuation_coeff
};

class AreaLight
	: public Light
{
public:
	AreaLight(Scene *scene, const vec3f& pos, const vec3f& color, const double& cac, const double& lac, const double& qac, const vec3f& up)
		: Light(scene, color), position(pos), cac(cac), lac(lac), qac(qac), up(up), pl1(scene, pos - up, color, cac, lac, qac), pl2(scene, pos, color, cac, lac, qac), pl3(scene, pos + up, color, cac, lac, qac) {}
	virtual vec3f shadowAttenuation(const vec3f& P) const;
	virtual double distanceAttenuation(const vec3f& P) const;
	virtual vec3f getColor(const vec3f& P) const;
	virtual vec3f getDirection(const vec3f& P) const;

protected:
	vec3f position, up;
	double cac, lac, qac; // constant_attenuation_coeff, linear_attenuation_coeff, quadratic_attenuation_coeff
	PointLight pl1, pl2, pl3;
};
#endif // __LIGHT_H__
