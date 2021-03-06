#include "geometry.h"

template<class C1, class C2> bool collisionDetection(C1* obj1, C2* obj2) { return false; }

template<> bool collisionDetection(Sphere* s, Plane* pl) {
	// If sphere is behind plane (as defined by normal) we have a collision
	float dist = (s->pos - pl->a).dot(pl->normal);
	return (dist <= s->rad);
}

template<> bool collisionDetection(Sphere* s, AABB* rect) {
	// Same as plane but also checks for rectangle boundaries
	float dist = (s->pos - rect->a).dot(rect->normal);

	// Consider collisions even if sphere has penetrated rectangle for some time.
	// This will eventually break down if speed is too large compared to loop processing speed.
	if (dist <= s->rad && dist >= -10 * s->rad) {
		Vec3f p = s->pos + dist * (-rect->normal);
		return(p.x >= rect->minX && p.x <= rect->maxX
			&& p.y >= rect->minY && p.y <= rect->maxY
			&& p.z >= rect->minZ && p.z <= rect->maxZ);
	}
	return false;
}

template<> bool collisionDetection(Sphere* s1, Sphere* s2) {
	return (s1->pos - s2->pos).normsq() <= (s1->rad + s2->rad) * (s1->rad + s2->rad);
}

Sphere::Sphere(Vec3f& position, float radius, float mass, float restitution, Vec3f& velocity, Vec3f& color, Vec3f& selectedColor)
	: pos(position), rad(radius), m(mass), r(restitution),
	origPos(position), rgb(color), selectRgb(selectedColor), 
	selected(false), velocity(velocity), origVelocity(velocity)
{
	// add gravity by default
	forces.push_back(Force(Vec3f(0, -1, 0), GRAVITY_ACCEL, 0.0f));
}

void Sphere::draw(){
	glPushMatrix();

	if (selected) glColor3fv(selectRgb);
	else glColor3fv(rgb);
	glTranslatef(pos.x, pos.y, pos.z);
	GLUquadricObj* qobj = gluNewQuadric();
	gluQuadricNormals(qobj, GLU_SMOOTH);
	gluSphere(qobj, rad, 16, 16);
	gluDeleteQuadric(qobj);
	
	glPopMatrix();
}

void Sphere::reset(){
	pos = origPos, velocity = origVelocity;
	// Remove all forces except gravity
	forces.erase(forces.begin() + 1, forces.end());
}

void Sphere::update(double dt) {
	for (int i = 0; i < forces.size(); ++i) {
		if (forces[i].f <= 0.01) {
			forces.erase(forces.begin() + i);
			// wind back index to account for deletion
			--i;
			continue;
		}
		float fcurr = m * forces[i].f * DAMPENING_FACTOR;
		velocity += (fcurr * forces[i].dpc);
		forces[i].decay();
	}
	pos += (dt * velocity);
}

void Sphere::collide(Scene& scene, int idx) {
	// Sphere collision
	int nspheres = scene.spheres.size();
	for (int i = idx + 1; i < nspheres; ++i) {
		Sphere* s = scene.spheres[i].get();
		if (s != nullptr && collisionDetection(this, s)) {
			Vec3f distVec = pos - s->pos;
			// Calculate projections of velocities onto force vector			
			Vec3f force = pos - s->pos;
			force.normalize();
			float x1_proj = force.dot(velocity);
			Vec3f v1x = x1_proj * force;
			Vec3f v1y = velocity - v1x;

			float x2_proj = (-force).dot(s->velocity);
			Vec3f v2x = x2_proj * (-force);
			Vec3f v2y = s->velocity - v2x;

			// Update velocities of both spheres according to Newtonian physics
			float cor = r * s->r;
			float m12 = m + s->m;
			Vec3f mu12 = m * v1x + s->m * v2x;
			velocity = v1y + (mu12 + s->m * cor * (v2x - v1x)) / m12;
			s->velocity = v2y + (mu12 + m * cor * (v1x - v2x)) / m12;
			
			// Prevent merging
			float diff = (rad + s->rad) * (rad + s->rad) - distVec.normsq();
			if (diff > 0) {
				distVec.normalize();

				// Move spheres in opposite directions
				pos += (diff / 2.0) * distVec;
				s->pos -= (diff / 2.0) * distVec;
			}
		}
	}

	// Plane collision
	int nplanes = scene.planes.size();
	for (int i = 0; i < nplanes; ++i) {
		Plane* p = scene.planes[i].get();
		if (p != nullptr && collisionDetection(this, p)) {
			// Reflect velocity around hit surface normal and apply restitution coefficient
			velocity -= (1 + r) * velocity.dot(p->normal) * p->normal;
			
			// Prevent merging
			float dist = (pos - p->a).dot(p->normal);
			if (dist < rad) {
				pos += (rad - dist) * p->normal;
			}
		}
	}

	// AABB collision
	int naabbs = scene.aabbs.size();
	for (int i = 0; i < naabbs; ++i) {
		AABB* aabb = scene.aabbs[i].get();
		if (aabb != nullptr && collisionDetection(this, aabb)) {
			// Reflect velocity around hit surface normal and mult by restitution factor
			velocity -= (1 + r) * velocity.dot(aabb->normal) * aabb->normal;

			// Prevent merging
			float dist = (pos - aabb->a).dot(aabb->normal);
			if (dist < rad) {
				pos += (rad - dist) * aabb->normal;
			}
		}
	}
}

Plane::Plane(Vec3f& a, Vec3f& b, Vec3f& c, Vec3f& d, Vec3f& color)
	: a(a), b(b), c(c), d(d), rgb(color) 
{
	normal = (b - a).cross(d - a);
	normal.normalize();
}

void Plane::draw(){
	glPushMatrix();

	glColor3fv(rgb);
	glBegin(GL_QUADS);
	glNormal3fv(normal);
	glVertex3fv(a);
	glVertex3fv(b);
	glVertex3fv(c);
	glVertex3fv(d);

	glEnd();

	glPopMatrix();
}

AABB::AABB(Vec3f& a, Vec3f& b, Vec3f& c, Vec3f& d, Vec3f& color) 
	: Plane(a, b, c, d, color) 
{
	minX = std::fmin(d.x, std::fmin(c.x, std::fmin(a.x, b.x)));
	minY = std::fmin(d.y, std::fmin(c.y, std::fmin(a.y, b.y)));
	minZ = std::fmin(d.z, std::fmin(c.z, std::fmin(a.z, b.z)));

	maxX = std::fmax(d.x, std::fmax(c.x, std::fmax(a.x, b.x)));
	maxY = std::fmax(d.y, std::fmax(c.y, std::fmax(a.y, b.y)));
	maxZ = std::fmax(d.z, std::fmax(c.z, std::fmax(a.z, b.z)));
}
