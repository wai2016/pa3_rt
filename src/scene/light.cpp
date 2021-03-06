#include <cmath>

#include "light.h"

double DirectionalLight::distanceAttenuation( const vec3f& P ) const
{
	// distance to light is infinite, so f(di) goes to 0.  Return 1.
	return 1.0;
}


vec3f DirectionalLight::shadowAttenuation( const vec3f& P ) const
{
    // YOUR CODE HERE:
    // You should implement shadow-handling code here.

	vec3f sa = vec3f(1, 1, 1); // assume not block the light
	vec3f d = getDirection(P);
	ray r(P, d);
	isect i;
	ray nr = r;

	while (scene->intersect(nr, i)) // no need care about the light position
	{
		const Material& m = i.getMaterial();
		if (m.kt.iszero())
			return vec3f(0, 0, 0); // block the light
		sa = prod(m.kt, color);
		nr = ray(nr.at(i.t), d); // produce new ray if the object is transparent
		if (i.t < RAY_EPSILON) // check if error occurs
			nr = ray(nr.at(i.t), d);
	}
	return sa;
}

vec3f DirectionalLight::getColor( const vec3f& P ) const
{
	// Color doesn't depend on P 
	return color;
}

vec3f DirectionalLight::getDirection( const vec3f& P ) const
{
	return -orientation;
}

double PointLight::distanceAttenuation( const vec3f& P ) const
{
	// YOUR CODE HERE

	// You'll need to modify this method to attenuate the intensity 
	// of the light based on the distance between the source and the 
	// point P.  

	double d = sqrt((position[0] - P[0]) * (position[0] - P[0]) + 
					(position[1] - P[1]) * (position[1] - P[1]) + 
					(position[2] - P[2]) * (position[2] - P[2])); // sqrt(x^2 + y^2 + z^2)
	double fd = 1.0 / (cac + lac * d + qac * d * d);
	fd = minimum(1.0, fd);
	return fd;
}

vec3f PointLight::getColor( const vec3f& P ) const
{
	// Color doesn't depend on P 
	return color;
}

vec3f PointLight::getDirection( const vec3f& P ) const
{
	return (position - P).normalize();
}


vec3f PointLight::shadowAttenuation(const vec3f& P) const
{
    // YOUR CODE HERE:
    // You should implement shadow-handling code here.

	vec3f sa = vec3f(1, 1, 1); // assume not block the light
	vec3f d = getDirection(P);
	ray r(P, d);
	isect i;
	ray nr = r;

	while (scene->intersect(nr, i)) // just check this is not enough for point light because it has position
	{
		// use distance to check whether the light source is in the front or not
		double d1 = sqrt((position[0] - P[0]) * (position[0] - P[0]) +
						 (position[1] - P[1]) * (position[1] - P[1]) +
						 (position[2] - P[2]) * (position[2] - P[2])); // sqrt(x^2 + y^2 + z^2)
		double d2 = sqrt((nr.at(i.t)[0] - P[0]) * (nr.at(i.t)[0] - P[0]) +
						 (nr.at(i.t)[1] - P[1]) * (nr.at(i.t)[1] - P[1]) +
						 (nr.at(i.t)[2] - P[2]) * (nr.at(i.t)[2] - P[2])); // sqrt(x^2 + y^2 + z^2)
		if (d1 < d2) // check whether the light source is in the front or not
			break; // light source in the front
		else
		{
			const Material& m = i.getMaterial();
			if (m.kt.iszero())
				return vec3f(0, 0, 0); // block the light
			sa = prod(m.kt, color);
			nr = ray(nr.at(i.t), d); // produce new ray if the object is transparent
			if (i.t < RAY_EPSILON) // check if error occurs
				nr = ray(nr.at(i.t), d);
		}
		
	}
    return sa;
}


// -------------------------------------------------------------ambient light------------------------------------------------------
double AmbientLight::distanceAttenuation(const vec3f& P) const
{
	// ???
	return 1.0;
}

vec3f AmbientLight::getColor(const vec3f& P) const
{
	// Color doesn't depend on P 
	return color;
}

vec3f AmbientLight::getDirection(const vec3f& P) const
{
	// ???
	return vec3f(1,1,1);
}


vec3f AmbientLight::shadowAttenuation(const vec3f& P) const
{
	// ???
	return vec3f(1, 1, 1);
}

// -------------------------------------------------------------------spotlight--------------------------------------------------------------------
double SpotLight::distanceAttenuation(const vec3f& P) const
{
	double d = sqrt((position[0] - P[0]) * (position[0] - P[0]) +
		(position[1] - P[1]) * (position[1] - P[1]) +
		(position[2] - P[2]) * (position[2] - P[2])); // sqrt(x^2 + y^2 + z^2)
	double fd = 1.0 / (cac + lac * d + qac * d * d);
	fd = minimum(1.0, fd);
	return fd;
}

vec3f SpotLight::getColor(const vec3f& P) const
{
	// Color doesn't depend on P 
	return color;
}

vec3f SpotLight::getDirection(const vec3f& P) const
{
	return (position - P).normalize();
}


vec3f SpotLight::shadowAttenuation(const vec3f& P) const
{
	vec3f sa;
	vec3f x = (P - position).normalize(); // = -getDirection(P); // direction from light position to P
	double y = x.dot(direction.normalize()); // dot product of direction from light position to P and direction of light

	if (y < 0.0) // behind spotlight and assume a spotlight never light up things behind
		return vec3f(0, 0, 0);

	double deg = acos(y) * 180 / 3.14159265359; // pi = 3.14159265359
	if (angle >= deg) // in range
	{
		vec3f d = getDirection(P);
		ray r(P, d);
		isect i;

		if (scene->intersect(r, i)) // just check this is not enough for point light because it has position
		{
			// use distance to check whether the light source is in the front or not
			double d1 = sqrt((position[0] - P[0]) * (position[0] - P[0]) +
				(position[1] - P[1]) * (position[1] - P[1]) +
				(position[2] - P[2]) * (position[2] - P[2])); // sqrt(x^2 + y^2 + z^2)
			double d2 = sqrt((r.at(i.t)[0] - P[0]) * (r.at(i.t)[0] - P[0]) +
				(r.at(i.t)[1] - P[1]) * (r.at(i.t)[1] - P[1]) +
				(r.at(i.t)[2] - P[2]) * (r.at(i.t)[2] - P[2])); // sqrt(x^2 + y^2 + z^2)
			if (d1 < d2) // check whether the light source is in the front or not
				sa = vec3f(1, 1, 1); // light source in the front
			else
			{
				//const Material& m = i.getMaterial();
				//vec3f transparent; // don't know correct or not
				//transparent[0] = m.kt[0] * color[0];
				//transparent[1] = m.kt[1] * color[1];
				//transparent[2] = m.kt[2] * color[2];
				//sa = vec3f(0, 0, 0) + transparent; // don't know correct or not
				sa = vec3f(0, 0, 0);
			}
		}
		else
			sa = vec3f(1, 1, 1);
	}
	else
		sa = vec3f(0, 0, 0);

	return sa;
}

double AreaLight::distanceAttenuation(const vec3f& P) const
{
	// YOUR CODE HERE

	// You'll need to modify this method to attenuate the intensity 
	// of the light based on the distance between the source and the 
	// point P.  

	return (pl1.distanceAttenuation(P) + pl2.distanceAttenuation(P) + pl3.distanceAttenuation(P)) / 3;
}

vec3f AreaLight::getColor(const vec3f& P) const
{
	// Color doesn't depend on P 
	return color;
}

vec3f AreaLight::getDirection(const vec3f& P) const
{
	return (position - P).normalize();
}


vec3f AreaLight::shadowAttenuation(const vec3f& P) const
{
	// YOUR CODE HERE:
	// You should implement shadow-handling code here.

	return (pl1.shadowAttenuation(P) + pl2.shadowAttenuation(P) + pl3.shadowAttenuation(P)) / 3;
}