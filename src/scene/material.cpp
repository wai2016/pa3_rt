#include "ray.h"
#include "material.h"
#include "light.h"
#include <iostream>

#include "../ui/TraceUI.h"

extern TraceUI* traceUI;

// Apply the phong model to this point on the surface of the object, returning
// the color of that point.
vec3f Material::shade( Scene *scene, const ray& r, const isect& i ) const
{
	// YOUR CODE HERE

	// For now, this method just returns the diffuse color of the object.
	// This gives a single matte color for every distinct surface in the
	// scene, and that's it.  Simple, but enough to get you started.
	// (It's also inconsistent with the phong model...)

	// Your mission is to fill in this method with the rest of the phong
	// shading model, including the contributions of all the light sources.
    // You will need to call both distanceAttenuation() and shadowAttenuation()
    // somewhere in your code in order to compute shadows and light falloff.

	const Material& m = i.getMaterial();
	vec3f I = m.ke;

	// ambient light from user input
	double userAmbient = traceUI->getAmbient();
	I[0] = I[0] + m.ka[0] * userAmbient * (1 - m.kt[0]);
	I[1] = I[1] + m.ka[1] * userAmbient * (1 - m.kt[1]);
	I[2] = I[2] + m.ka[2] * userAmbient * (1 - m.kt[2]);

	Scene::cliter iter1;
	for (iter1 = scene->beginLights(); iter1 != scene->endLights(); ++iter1) {
		// At this point, *iter1 is the current list item, just
		// as if iter1 were a pointer.
		
		// ambient light from .ray file
		if (typeid(*(*iter1)).name() == typeid(AmbientLight).name()) // but there should be one ambient light?
		{
			vec3f temp;
			temp[0] = m.ka[0] * (*iter1)->getColor(temp)[0] * (1 - m.kt[0]); 
			temp[1] = m.ka[1] * (*iter1)->getColor(temp)[1] * (1 - m.kt[1]);
			temp[2] = m.ka[2] * (*iter1)->getColor(temp)[2] * (1 - m.kt[2]);
			I = I + temp;
		}
		else
		{
			vec3f temp;

			// distance attenuation
			temp = (*iter1)->distanceAttenuation(r.at(i.t)) * (*iter1)->getColor(temp);

			// shadow attenuation
			temp[0] = temp[0] * (*iter1)->shadowAttenuation(r.at(i.t))[0];
			temp[1] = temp[1] * (*iter1)->shadowAttenuation(r.at(i.t))[1];
			temp[2] = temp[2] * (*iter1)->shadowAttenuation(r.at(i.t))[2];

			// diffuse
			vec3f n = i.N.normalize();
			vec3f l = (*iter1)->getDirection(r.at(i.t)).normalize(); // in fact, l should be already normalized!
			double nl = n.dot(l);
			nl = maximum(0, nl);

			// specular
			vec3f v = (r.getPosition() - r.at(i.t)).normalize();
			vec3f R = (2.0 * (l.dot(n)) * n - l).normalize();
			double vr = v.dot(R);
			double s = m.shininess * 128;
			vr = maximum(0, vr);
			vr = pow(vr, s);

			// diffuse and specular
			vec3f dns;
			if (i.obj->allow2Dmap3D() && texture != NULL) {
				int x = 0;
				int y = 0;
				i.obj->do2Dmap3D(r, i, x, y);
				x = x % texture_width;
				y = y % texture_height;
				double red = texture[(y * texture_width + x) * 3] / 255.0;
				double green = texture[(y * texture_width + x) * 3 + 1] / 255.0;
				double blue = texture[(y * texture_width + x) * 3 + 2] / 255.0;
				dns[0] = nl * red * (1 - m.kt[0]) + vr * m.ks[0];
				dns[1] = nl * green * (1 - m.kt[1]) + vr * m.ks[1];
				dns[2] = nl * blue * (1 - m.kt[2]) + vr * m.ks[2];
			}
			else {
				dns[0] = nl * m.kd[0] * (1 - m.kt[0]) + vr * m.ks[0];
				dns[1] = nl * m.kd[1] * (1 - m.kt[1]) + vr * m.ks[1];
				dns[2] = nl * m.kd[2] * (1 - m.kt[2]) + vr * m.ks[2];
			}

			temp[0] = dns[0] * temp[0];
			temp[1] = dns[1] * temp[1];
			temp[2] = dns[2] * temp[2];

			// final I
			I = I + temp;
		}
	}

	return I;
}
