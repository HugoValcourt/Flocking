#pragma once
#include "Blit3D.h"

class Boid
{
public:
	Boid(float speed, float maxTurnSpeed, float cohesionRadius, float alignmentRadius, float separationRadius, std::vector<Boid *> *boidList, float angle, glm::vec2 position, Sprite *sprite);
	void Update(float seconds);
	void Draw();

private:
	Sprite *sprite;

	float speed = 5.f;
	float maxTurnSpeed = 500.f;
	float angle;
	float cohesionRadius;
	float alignmentRadius;
	float separationRadius;

	std::vector<Boid *> *boidList;
	std::vector<Boid *> neighborList;

	glm::vec2 cohesion;
	glm::vec2 alignment;
	glm::vec2 separation;

	glm::vec2 nextHeading, velocity, dir, position, wrapPosition;
	glm::vec2 Cohesion();
	glm::vec2 Alignment();
	glm::vec2 Separation();

	void TurnToHeading(glm::vec2 desiredHeading, float seconds);
	void CalculateHeading(float seconds);
	float Distance(Boid *boid1, Boid boid2);
};