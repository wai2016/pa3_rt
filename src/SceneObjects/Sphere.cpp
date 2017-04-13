#include <cmath>

#include "Sphere.h"

#ifdef WIN32
#define M_PI	3.1415926535F
#endif

bool Sphere::intersectLocal( const ray& r, isect& i ) const
{
	vec3f v = -r.getPosition();
	double b = v.dot(r.getDirection());
	double discriminant = b*b - v.dot(v) + 1;

	if( discriminant < 0.0 ) {
		return false;
	}

	discriminant = sqrt( discriminant );
	double t2 = b + discriminant;

	if( t2 <= RAY_EPSILON ) {
		return false;
	}

	i.obj = this;

	double t1 = b - discriminant;

	if( t1 > RAY_EPSILON ) {
		i.t = t1;
		i.N = r.at( t1 ).normalize();
	} else {
		i.t = t2;
		i.N = r.at( t2 ).normalize();
	}

	return true;
}

void Sphere::do2Dmap3D(const ray& r, const isect& i, int& x, int& y) const {
	vec3f ip = transform->globalToLocalCoords(r.at(i.t));
	double a = acos(ip[1] / ip.length());
	double b;
	if (abs(ip[0]) > RAY_EPSILON) {
		b = atan(ip[2] / ip[0]);
	}
	else {
		b = M_PI / 2;
	}
	x = (2 - a / M_PI) * material->texture_width;
	if (b < 0) {
		y = b / (2 * M_PI) * material->texture_height + material->texture_height / 2;
	}
	else {
		y = b / (2 * M_PI) * material->texture_height;
	}
}


