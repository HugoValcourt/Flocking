#include "Boid.h"
#include "DieRoller.h"

extern Blit3D *blit3D;
extern DieRoller *rd;

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//AUTHOR: Hugo Valcourt
//DATE:   March 1st 2018
//PURPOSE:Create an instance of a boid
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Boid::Boid(float speed, float maxTurnSpeed, float cohesionRadius, float alignmentRadius, float separationRadius, std::vector<Boid *> *boidList, float angle, glm::vec2 position, Sprite *sprite)
{
	//Assign attributes
	this->sprite = sprite;
	this->speed = speed;
	this->maxTurnSpeed = maxTurnSpeed;
	this->cohesionRadius = cohesionRadius;
	this->alignmentRadius = alignmentRadius;
	this->separationRadius = separationRadius;
	this->boidList = boidList;
	this->angle = angle;
	this->position = position;
	cohesion = glm::vec2(0.f, 0.f);
	alignment = glm::vec2(0.f, 0.f);
	separation = glm::vec2(0.f, 0.f);
	dir = glm::normalize(glm::vec2(1.f, 1.f));
	nextHeading = glm::normalize(glm::vec2(1.f, 1.f));
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//AUTHOR: Hugo Valcourt
//DATE:   March 1st 2018
//PURPOSE:Update the boids heading
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
void Boid::Update(float seconds)
{
	if (position.x < 0) position.x += blit3D->screenWidth;		//Wrap around position
	if (position.x > blit3D->screenWidth) position.x -= blit3D->screenWidth;
	if (position.y < 0) position.y += blit3D->screenHeight;
	if (position.y > blit3D->screenHeight) position.y -= blit3D->screenHeight;

	CalculateHeading(seconds);

	//update the position based on the facing
	position += velocity * seconds;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//AUTHOR: Hugo Valcourt
//DATE:   March 1st 2018
//PURPOSE:Calculate the boids heading based on behaviours
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
void Boid::CalculateHeading(float seconds)
{
	//Neighbors will change (probably)
	neighborList.clear();

	//Make neighbor list
	for (int i = 0; i < boidList->size(); i++)
		if (boidList->at(i) != this)
		{
			float distance = Distance(boidList->at(i), *this); //Check for neighbor in cohesion radius, which is the biggest. (Works with wrap around.)
			if (distance <= cohesionRadius)
				neighborList.push_back(boidList->at(i));
		}

	//calculate facing vector
	float radians = angle * (M_PI / 180.f);
	dir.x = std::cos(radians);
	dir.y = std::sin(radians);
	//Go towards facing
	velocity += dir * speed;
	velocity = glm::normalize(velocity) * speed;

	//Assign behaviours
	cohesion = Cohesion();
	alignment = Alignment();
	separation = Separation();

	if (!std::isnan(cohesion.x) && !std::isnan(cohesion.y)) cohesion = glm::normalize(cohesion) * 0.05f;
	//else cohesion = glm::vec2(0.f, 0.f);
	if (!std::isnan(alignment.x) && !std::isnan(alignment.y)) alignment = glm::normalize(alignment) * 1.f;
	//else alignment = glm::vec2(0.f, 0.f);
	if (!std::isnan(separation.x) && !std::isnan(separation.y)) separation = glm::normalize(separation) * 1.0f;
	else separation = glm::vec2(0.f, 0.f);

	if (!std::isnan(cohesion.x) && !std::isnan(cohesion.y) &&
		!std::isnan(alignment.x) && !std::isnan(alignment.y) &&
		!std::isnan(separation.x) && !std::isnan(separation.y))
	{
		nextHeading = separation + alignment + cohesion;//If nothing is nan, set the nextHeading with the combined behaviours
	}
	else
	{
		nextHeading = nextHeading;						//If something is nan, just keep going straight 
	}

	nextHeading = glm::normalize(nextHeading);			//Normalize the nextheading
	TurnToHeading(nextHeading, seconds);				//Then start turning towards it

}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//AUTHOR: Hugo Valcourt
//DATE:   March 1st 2018
//PURPOSE:Draw the boid
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
void Boid::Draw()
{
	sprite->angle = angle;

	sprite->Blit(position.x, position.y, 0.25f, 0.25f);//Draw it on the screen

	//redraw if too close to an edge
	if (position.x < separationRadius)sprite->Blit(position.x + blit3D->screenWidth, position.y, 0.25f, 0.25f);//left
	if (position.x > blit3D->screenWidth - (separationRadius))sprite->Blit(position.x - blit3D->screenWidth, position.y, 0.25f, 0.25f);//right
	if (position.y < separationRadius)sprite->Blit(position.x, position.y + blit3D->screenHeight, 0.25f, 0.25f);//down
	if (position.y > blit3D->screenHeight - (separationRadius))sprite->Blit(position.x, position.y - blit3D->screenHeight, 0.25f, 0.25f);//up
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//AUTHOR: Hugo Valcourt
//DATE:   March 1st 2018
//PURPOSE:Calculate cohesion behaviour
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
glm::vec2 Boid::Cohesion()
{
	//Average position of other boids' position in radius (This exclude THIS boid's position)
	
	int count = 0;

	glm::vec2 averagePos(0.f, 0.f);
	if (neighborList.size() > 0)//Can't divide by 0
	{
		//For each boid in the big radius of THIS boid's
		for (int i = 0; i < neighborList.size(); i++)
		{
			averagePos += neighborList.at(i)->position;		//If a boid is found in the radius, add it to the average
			count++;
		}
		//Sum the average and this is our next heading vector
		if (count > 0)
		{
			averagePos /= count;
			averagePos -= position;
			return glm::normalize(averagePos);
		}
	}
	return dir;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//AUTHOR: Hugo Valcourt
//DATE:   March 1st 2018
//PURPOSE:Calculate alignment behaviour
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
glm::vec2 Boid::Alignment()
{
	//Average of every other boids' heading, then normalize
	int count = 0;
	glm::vec2 averageHeading(0.f, 0.f);
	//if (neighborList.size() > 0)//Can't divide by 0
	{
		for (int i = 0; i < neighborList.size(); i++)
		{
			float distance = Distance(neighborList.at(i), *this);
			if (distance <= alignmentRadius)
			{
				averageHeading += neighborList.at(i)->velocity;		//If a boid is found in the radius, add it to the average
				count++;
			}
		}
		averageHeading /= count;
	}
	return averageHeading;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//AUTHOR: Hugo Valcourt
//DATE:   March 1st 2018
//PURPOSE:Calculate separation behaviour
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
glm::vec2 Boid::Separation()
{
	//To calculate Separation, we need to decide on a small local radius, then make vectors AWAY from every other boid within that radius.
	//These vectors can be wieghted so that they are larger for closer boids, and smaller for farther boids; one easy way to do this is:

	//1.	make a vector by subtracting the local boid's postion from our boid's position, call it V
	//2.	determining the length of that vector, call it L
	//3.	noramalizing the vector V
	//4.	then scaling it : V = V * (1 - (L / r), where r is the local radius of the steering behaviour.

	int count = 0;

	for (int i = 0; i < boidList->size(); i++)//Calculate how many boids are actually in the separation radius
		if (boidList->at(i) != this)
		{
			float distance = Distance(boidList->at(i), *this);
			if (distance <= separationRadius)
				count++;
		}

	glm::vec2 V(0.f, 0.f);//Where do we need to go to seperate our self?
	if (count == 0)//If there is no one in the radius, return where the boid was going
		return dir;

	for (int i = 0; i < neighborList.size(); i++)
		if (boidList->at(i) != this)
		{
			float distance = Distance(neighborList.at(i), *this);

			if (distance <= separationRadius)//If a boid is found in the local radius
			{
				glm::vec2 diff = position - neighborList.at(i)->position;//180 opposite way of boid in local radius
				if (diff.x != 0.f && diff.y != 0.f)//If diff is not 0, we cannot divide it 
				{
					diff = glm::normalize(diff);//Normalize the vector
					V += diff;//Add it to the difference to the local vector 
				}
			}
		}
	V /= count;//Divide the vector by how many boids there is in the local radius
	V = glm::normalize(V);//Normalize the vector
	return V * (1 - (glm::length(V) / separationRadius));
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//AUTHOR: Hugo Valcourt
//DATE:   March 1st 2018
//PURPOSE:Calculate distance between THIS boid and another one.
//		  Distance works with wrap around of the screen size
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
float Boid::Distance(Boid *boid1, Boid boid2)
{
	float distanceX = std::abs(boid1->position.x - boid2.position.x);//Get the distance on the x
	if (distanceX > blit3D->screenWidth / 2) distanceX = (distanceX - blit3D->screenWidth);//Check if the distance is close the the opposite edge on x, if so, the width of the screen is deducted on x Distance

	float distanceY = std::abs(boid1->position.y - boid2.position.y);//Get the distance on the y
	if (distanceY > blit3D->screenHeight / 2) distanceY = (distanceY - blit3D->screenHeight);//Check if the distance is close the the opposite edge on y, if so, the width of the screen is deducted on y Distance

	float distance = std::sqrt((distanceX * distanceX) + (distanceY * distanceY));//The actual distance
	return distance;//Return the distance
}

void Boid::TurnToHeading(glm::vec2 desiredHeading, float seconds)
{
	//angle=atan2(-b*x+a*y,a*x+b*y)
	float difference = std::atan2f(-dir.y*desiredHeading.x + dir.x*desiredHeading.y, dir.x*desiredHeading.x
		+ dir.y*desiredHeading.y);
	difference = glm::degrees(difference);
	float amountAllowed = maxTurnSpeed * seconds;

	if (abs(difference) <= amountAllowed)
	{
		//we are turning less than the amount allowed, so we can just
		//set our direction to the desired one
		angle += difference;
		dir = desiredHeading;
	}
	else
	{
		//we are trying to turn too far for the time passed,
		//so only change by the amount allowed in that time
		if (difference < 0) angle -= amountAllowed;
		else angle += amountAllowed;
		dir.x = cos(glm::radians(angle));
		dir.y = sin(glm::radians(angle));
	}
}