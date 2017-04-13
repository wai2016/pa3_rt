#include <cmath>
#include <assert.h>

#include "Box.h"

bool Box::intersectLocal( const ray& r, isect& i ) const
{
	// YOUR CODE HERE:
    // Add box intersection code here.

	vec3f p = r.getPosition();
	vec3f d = r.getDirection();

	vec3f plane[3] = { {0, 0, 1}, {0, 1, 0}, {1, 0, 0} }; // xy-plane normal, xz-plane normal, yz-plane normal

	double D[6] = { 0.5, 0.5, 0.5, -0.5, -0.5, -0.5 };

	double vd;
	double v;

	double t[6] = { INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY };

	bool intercept = false;

	for (int j = 0; j < 6; j++)
	{
		vd = plane[j % 3].dot(d);
		v = -(plane[j % 3].dot(p) + D[j]);

		if (abs(vd) <= RAY_EPSILON) // divide by 0
			continue;

		double tempt = v / vd;

		if (tempt <= RAY_EPSILON) 
			continue;

		vec3f P = r.at(tempt);

		if ((P[0] < -0.5 || P[0] > 0.5) && j % 3 != 2) 
			continue;

		if ((P[1] < -0.5 || P[1] > 0.5) && j % 3 != 1) 
			continue;

		if ((P[2] < -0.5 || P[2] > 0.5) && j % 3 != 0) 
			continue;

		intercept = true;
		t[j] = tempt; // set t[j] if it really pass all tests
	}

	double min = t[0];
	int minplane = 0;

	for (int j = 1; j < 6; j++) // find the smallest one
	{
		if (t[j] < min)
		{
			min = t[j];
			minplane = j;
		}
	}

	if (!intercept)
		return false;

	i.obj = this;
	i.t = min;
	if (minplane % 3 == 0)
	{
		if (D[minplane] > 0)
			i.N = vec3f(0.0, 0.0, -1.0);
		else
			i.N = vec3f(0.0, 0.0, 1.0);
		
	}
	else if (minplane % 3 == 1)
	{
		if (D[minplane] > 0)
			i.N = vec3f(0.0, -1.0, 0.0);
		else
			i.N = vec3f(0.0, 1.0, 0.0);
	}
	else if (minplane % 3 == 2)
	{
		if (D[minplane] > 0)
			i.N = vec3f(-1.0, 0.0, 0.0);
		else 
			i.N = vec3f(1.0, 0.0, 0.0);
	}

	return true;
}

void Box::do2Dmap3D(const ray& r, const isect& i, int& x, int& y) const {
	vec3f ip = transform->globalToLocalCoords(r.at(i.t));
	if (0.5 - abs(ip[0]) < RAY_EPSILON) {
		x = (ip[1] + 0.5) * material->texture_width;
		y = (ip[2] + 0.5) * material->texture_height;
	}
	else if (0.5 - abs(ip[1]) < RAY_EPSILON) {
		x = (ip[0] + 0.5) * material->texture_width;
		y = (ip[2] + 0.5) * material->texture_height;
	}
	else if (0.5 - abs(ip[2]) < RAY_EPSILON) {
		x = (ip[0] + 0.5) * material->texture_width;
		y = (ip[1] + 0.5) * material->texture_height;
	}
}
