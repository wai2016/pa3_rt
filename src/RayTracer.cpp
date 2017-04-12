// The main ray tracer.

#include <Fl/fl_ask.h>

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"
#include "fileio/read.h"
#include "fileio/parse.h"

#include "ui/TraceUI.h"

extern TraceUI* traceUI;

// Trace a top-level ray through normalized window coordinates (x,y)
// through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.
vec3f RayTracer::trace( Scene *scene, double x, double y )
{
	bool depth = traceUI->getDOF();
	double thresh = traceUI->getAT();
	vec3f col;
    ray r( vec3f(0,0,0), vec3f(0,0,0) );
    scene->getCamera()->rayThrough( x,y,r );
	col = traceRay(scene, r, vec3f(thresh, thresh, thresh), 0).clamp();

	// depth of field 
	if (depth)
	{
		// shoot 4 more rays towards focus => stupid hard coding
		double focalLength = traceUI->getFocalLength(); 
		double radius = 0.1; // user input later?
		ray r1(vec3f(0, 0, 0), vec3f(0, 0, 0));
		ray r2(vec3f(0, 0, 0), vec3f(0, 0, 0));
		ray r3(vec3f(0, 0, 0), vec3f(0, 0, 0));
		ray r4(vec3f(0, 0, 0), vec3f(0, 0, 0));
		scene->getCamera()->rayThroughDOF(x, y, 1, 0, r1, focalLength, radius);
		scene->getCamera()->rayThroughDOF(x, y, -1, 0, r2, focalLength, radius);
		scene->getCamera()->rayThroughDOF(x, y, 0, 1, r3, focalLength, radius);
		scene->getCamera()->rayThroughDOF(x, y, 0, -1, r4, focalLength, radius);
		col = col + traceRay(scene, r1, vec3f(1.0, 1.0, 1.0), 0).clamp() + 
					traceRay(scene, r2, vec3f(1.0, 1.0, 1.0), 0).clamp() + 
					traceRay(scene, r3, vec3f(1.0, 1.0, 1.0), 0).clamp() + 
					traceRay(scene, r4, vec3f(1.0, 1.0, 1.0), 0).clamp();
		col = col / 5.0;
	}
	return col;
}

// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
vec3f RayTracer::traceRay( Scene *scene, const ray& r, 
	const vec3f& thresh, int depth )
{
	if (depth > traceUI->getDepth()) // base case
		return vec3f(0.0, 0.0, 0.0);

	isect i;

	if( scene->intersect( r, i ) ) {
		// YOUR CODE HERE

		// An intersection occured!  We've got work to do.  For now,
		// this code gets the material for the surface that was intersected,
		// and asks that material to provide a color for the ray.  

		// This is a great place to insert code for recursive ray tracing.
		// Instead of just returning the result of shade(), add some
		// more steps: add in the contributions from reflected and refracted
		// rays.

		const Material& m = i.getMaterial();
		vec3f I = m.shade(scene, r, i);

		if (I[0] < thresh[0] && I[1] < thresh[1] && I[2] < thresh[2] && depth > 0) // adaptive termination
			return vec3f(0.0, 0.0, 0.0);

		// reflection
		vec3f p = r.at(i.t); // position of the intersect point
		vec3f vd = (r.getPosition() - p).normalize(); // viewing direction from intersect point
		vec3f Rd = ((2 * i.N.dot(vd) * i.N) - vd).normalize(); 
		ray Rr(p, Rd);
		vec3f R = traceRay(scene, Rr, thresh, depth + 1);
		if (traceUI->getGlossyReflection())
		{
			// glossy reflection (4 more R: 0, 90, 180, 270)
			double rad = 1 * 3.14159265359 / 180; 
			double ratio = 0.5; // how glossy, input from user later
			vec3f R1(tan(rad), 0, 1);
			vec3f R2(0, tan(rad), 1);
			vec3f R3(-tan(rad), 0, 1);
			vec3f R4(0, -tan(rad), 1);
			vec3f stupidx = vec3f(0, 1, 0).cross(Rd).normalize();
			vec3f stupidy = Rd.cross(stupidx).normalize();
			mat4f T(vec4f(stupidx[0], stupidx[1], stupidx[2], p[0]),
					vec4f(stupidy[0], stupidy[1], stupidy[2], p[1]),
					vec4f(Rd[0], Rd[1], Rd[2], p[2]),
					vec4f(0, 0, 0, 1)); // transform to Rd
			R1 = R1 * T;
			R2 = R2 * T;
			R3 = R3 * T;
			R4 = R4 * T;
			ray Rr1(p, R1);
			ray Rr2(p, R2);
			ray Rr3(p, R3);
			ray Rr4(p, R4);
			R1 = traceRay(scene, Rr1, thresh, depth + 1);
			R2 = traceRay(scene, Rr2, thresh, depth + 1);
			R3 = traceRay(scene, Rr3, thresh, depth + 1);
			R4 = traceRay(scene, Rr4, thresh, depth + 1);
			R = R * ratio + (R1 + R2 + R3 + R4) * (1.0 - ratio) / 4.0;
		}
		I[0] = I[0] + m.kr[0] * R[0];
		I[1] = I[1] + m.kr[1] * R[1];
		I[2] = I[2] + m.kr[2] * R[2];

		// refraction and total internal reflection
		double n_i;
		double n_t;
		vec3f Tn;
		if (vd.dot(i.N) >= 0.0) // ray is entering object
		{
			n_i = 1.0002772; // refraction index of air
			n_t = m.index;
			Tn = i.N;
		}
		else 
		{
			n_i = m.index;
			n_t = 1.0002772; // refraction index of air
			Tn = -i.N;
		}
		double n = n_i / n_t;
		double cosi = Tn.dot(vd); // must be positive?
		double radicand = 1 - n * n * (1 - cosi * cosi);
		if (radicand >= 0.0) // not total internal reflection
		{
			double cost = sqrt(radicand);
			vec3f Td = ((n * cosi - cost) * Tn - n * vd).normalize();
			ray Tr(p, Td);
			vec3f T = traceRay(scene, Tr, thresh, depth + 1);
			I[0] = I[0] + m.kt[0] * T[0];
			I[1] = I[1] + m.kt[1] * T[1];
			I[2] = I[2] + m.kt[2] * T[2];
		}

		return I;
	
	} else {
		// No intersection.  This ray travels to infinity, so we color
		// it according to the background color, which in this (simple) case
		// is just black.

		return vec3f( 0.0, 0.0, 0.0 );
	}
}

RayTracer::RayTracer()
{
	buffer = NULL;
	buffer_width = buffer_height = 256;
	scene = NULL;

	m_bSceneLoaded = false;
}


RayTracer::~RayTracer()
{
	delete [] buffer;
	delete scene;
}

void RayTracer::getBuffer( unsigned char *&buf, int &w, int &h )
{
	buf = buffer;
	w = buffer_width;
	h = buffer_height;
}

double RayTracer::aspectRatio()
{
	return scene ? scene->getCamera()->getAspectRatio() : 1;
}

bool RayTracer::sceneLoaded()
{
	return m_bSceneLoaded;
}

bool RayTracer::loadScene( char* fn )
{
	try
	{
		scene = readScene( fn );
	}
	catch( ParseError pe )
	{
		fl_alert( "ParseError: %s\n", pe );
		return false;
	}

	if( !scene )
		return false;
	
	buffer_width = 256;
	buffer_height = (int)(buffer_width / scene->getCamera()->getAspectRatio() + 0.5);

	bufferSize = buffer_width * buffer_height * 3;
	buffer = new unsigned char[ bufferSize ];
	
	// separate objects into bounded and unbounded
	scene->initScene();
	
	// Add any specialized scene loading code here
	
	m_bSceneLoaded = true;

	return true;
}

void RayTracer::traceSetup( int w, int h )
{
	if( buffer_width != w || buffer_height != h )
	{
		buffer_width = w;
		buffer_height = h;

		bufferSize = buffer_width * buffer_height * 3;
		delete [] buffer;
		buffer = new unsigned char[ bufferSize ];
	}
	memset( buffer, 0, w*h*3 );
}

void RayTracer::traceLines( int start, int stop )
{
	vec3f col;
	if( !scene )
		return;

	if( stop > buffer_height )
		stop = buffer_height;

	for( int j = start; j < stop; ++j )
		for( int i = 0; i < buffer_width; ++i )
			tracePixel(i,j);
}

void RayTracer::tracePixel(int i, int j)
{
	vec3f col;

	if (!scene)
		return;

	double xrand = 0.0;
	double yrand = 0.0;

	if (traceUI->getJittering()) // jittering
	{
		// range bewteen -0.5 and 0.5 over width and heigth
		xrand = ((double)(rand() % 101) / 100.0 - 0.5) / double(buffer_width);
		yrand = ((double)(rand() % 101) / 100.0 - 0.5) / double(buffer_height);
	}

	double x = double(i) / double(buffer_width) + xrand;
	double y = double(j) / double(buffer_height) + yrand;

	if (traceUI->getAntialiasing() && traceUI->getAntialiasingRange() > 1)
	{
		// antialiasing
		vec3f* sample;
		int range = traceUI->getAntialiasingRange();
		int num = range * range;

		sample = new vec3f[num];

		double xOffset = (1.0 / (range - 1)) / double(buffer_width);
		double yOffset = (1.0 / (range - 1)) / double(buffer_height);

		double xMTC = 0.5 / double(buffer_width); // -0.5 for moving the origin from corner to center
		double yMTC = 0.5 / double(buffer_height); // -0.5 for moving the origin from corner to center

		for (int a = 0; a < range; a++)
		{
			for (int b = 0; b < range; b++)
			{
				sample[a * range + b] = trace(scene, x + xOffset * a - xMTC + xrand, y + yOffset * b - yMTC + yrand); 
			}
		}

		for (int k = 0; k < num; k++)
		{
			col = col + sample[k];
		}

		col[0] = col[0] / num;
		col[1] = col[1] / num;
		col[2] = col[2] / num;

		delete[] sample;
	}
	else
		col = trace( scene,x,y );
	
	unsigned char *pixel = buffer + ( i + j * buffer_width ) * 3;

	pixel[0] = (int)( 255.0 * col[0]);
	pixel[1] = (int)( 255.0 * col[1]);
	pixel[2] = (int)( 255.0 * col[2]);

}