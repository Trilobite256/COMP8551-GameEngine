#pragma once

#define _USE_MATH_DEFINES

#include "entityx/entityx.h"
#include <vector>
#include <algorithm>
#include "../Components/Components.h"
// #include "PhysicsFunctions.h"
#include "../logger.h"
#include <string>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

using namespace entityx;

enum ColliderType {
    Box,
    Circle,
    Capsule
};

struct SASObject {
    SASObject(float val_, Entity& e_, bool isBegin_, ColliderType type_) : val(val_), e(e_), isBegin(isBegin_), type(type_) {}
    bool operator<(SASObject& right) {
        return val < right.val;
    }

    float val;
    Entity e;
    bool isBegin;
    ColliderType type;
};

struct EntityPair {
    EntityPair(Entity& a_, ColliderType aType_, Entity& b_, ColliderType bType_) : a(a_), b(b_), aType(aType_), bType(bType_) {}
    
    Entity a;
    ColliderType aType;
    Entity b;
    ColliderType bType;
};

class PhysicsSystem : public System<PhysicsSystem> {
    public:
        void update(EntityManager& es, EventManager& events, TimeDelta dt) override {
            //Step 1: Apply rigidbody movement (velocity)
            //TODO
            auto physicsEntities = es.entities_with_components<Rigidbody_2D>();

            for(Entity e : physicsEntities){
                //Update position:
                ComponentHandle<Rigidbody_2D> rb = e.component<Rigidbody_2D>();
                ComponentHandle<Transform> transform = e.component<Transform>();
                float t = (float)dt;
                transform->x = rb->velocityX * t + 0.5 * rb->accelerationX * powf(t, 2);
                transform->y = rb->velocityY * t + 0.5 * rb->accelerationY * powf(t, 2);
            }
            //Step 2: Detect collisions
            std::vector<EntityPair> pairs = broadphase(es); //returns pairs of possible collisions
            std::vector<EntityPair> collidingPairs = narrowphase(pairs); //should return pairs of entities that are colliding

            for (int i = 0; i < collidingPairs.size(); ++i) {
                Logger::getInstance() << collidingPairs.at(i).a.id().id() << " colliding with " << collidingPairs.at(i).b.id().id() << "\n";
            }
            //Logger::getInstance() << "physics!\n";
            //Step 3: apply physics to all entities and resolve all collisions from pairs
            for(int i = 0; i < collidingPairs.size(); ++i) {
                if(collidingPairs.at(i).a.has_component<Rigidbody_2D>() && collidingPairs.at(i).b.has_component<Rigidbody_2D>())
                    PerformCollisionCalculations(collidingPairs.at(i));
                else if(collidingPairs.at(i).a.has_component<Rigidbody_2D>())
                    PerformPhysicsWithOneRigidBody(collidingPairs.at(i).a, collidingPairs.at(i).b);
                else if(collidingPairs.at(i).b.has_component<Rigidbody_2D>())
                    PerformPhysicsWithOneRigidBody(collidingPairs.at(i).b, collidingPairs.at(i).a);
                else
                    ;//Notify for triggers on both entities
            }
            auto entities = es.entities_with_components<Rigidbody_2D>();

            for(Entity e : entities){
                //Update thrust by getting force value from input
                float thrust = 0.0f;

                //Update velocities and accelerations
                UpdateVelocityAndAcceleration(e, dt, thrust);
            }
        }
    private:
        std::vector<EntityPair> narrowphase(std::vector<EntityPair> possibleColl) {
            std::vector<EntityPair> collisions;
            for (EntityPair ep : possibleColl) {
                //get each entity's colliders and transforms
                bool isColliding = false;
                ComponentHandle<Transform> c1T = ep.a.component<Transform>();
                ComponentHandle<Transform> c2T = ep.b.component<Transform>();
                if (ep.aType == Box) {
                    ComponentHandle<BoxCollider> c1 = ep.a.component<BoxCollider>();
                    if (ep.bType == Box) {
                        ComponentHandle<BoxCollider> c2 = ep.b.component<BoxCollider>();
                        isColliding = CheckCollision(c1, c2, c1T, c2T);
                    } else if (ep.bType == Circle) {
                        ComponentHandle<CircleCollider> c2 = ep.b.component<CircleCollider>();
                        isColliding = CheckCollision(c1, c2, c1T, c2T);
                    } else if (ep.bType == Capsule) {
                        ComponentHandle<CapsuleCollider> c2 = ep.b.component<CapsuleCollider>();
                        isColliding = CheckCollision(c1, c2, c1T, c2T);
                    }
                } else if (ep.aType == Circle) {
                    ComponentHandle<CircleCollider> c1 = ep.a.component<CircleCollider>();
                    if (ep.bType == Box) {
                        ComponentHandle<BoxCollider> c2 = ep.b.component<BoxCollider>();
                        isColliding = CheckCollision(c1, c2, c1T, c2T);
                    } else if (ep.bType == Circle) {
                        ComponentHandle<CircleCollider> c2 = ep.b.component<CircleCollider>();
                        isColliding = CheckCollision(c1, c2, c1T, c2T);
                    } else if (ep.bType == Capsule) {
                        ComponentHandle<CapsuleCollider> c2 = ep.b.component<CapsuleCollider>();
                        isColliding = CheckCollision(c1, c2, c1T, c2T);
                    }
                } else if (ep.aType == Capsule) {
                    ComponentHandle<CapsuleCollider> c1 = ep.a.component<CapsuleCollider>();
                    if (ep.bType == Box) {
                        ComponentHandle<BoxCollider> c2 = ep.b.component<BoxCollider>();
                        isColliding = CheckCollision(c1, c2, c1T, c2T);
                    } else if (ep.bType == Circle) {
                        ComponentHandle<CircleCollider> c2 = ep.b.component<CircleCollider>();
                        isColliding = CheckCollision(c1, c2, c1T, c2T);
                    } else if (ep.bType == Capsule) {
                        ComponentHandle<CapsuleCollider> c2 = ep.b.component<CapsuleCollider>();
                        isColliding = CheckCollision(c1, c2, c1T, c2T);
                    }
                }
                // ComponentHandle<BoxCollider> c1 = ep.a.component<BoxCollider>();
                // ComponentHandle<BoxCollider> c2 = ep.b.component<BoxCollider>();

                // //check if the colliders are colliding based on their type/shape
                // bool isColliding = CheckCollision(c1, c2, c1T, c2T);
                if (isColliding) {
                    collisions.push_back(ep);
                }
            }

            return collisions;
        }
        #pragma region //Collision algorithms
        //Box - Box
        bool CheckCollision(ComponentHandle<BoxCollider>& c1, ComponentHandle<BoxCollider>& c2, ComponentHandle<Transform> c1T, ComponentHandle<Transform> c2T) {
            return DetectAABB(c1->x + c1T->x, c1->y + c1T->y, c1->bbWidth, c1->bbHeight, c2->x + c2T->x, c2->y + c2T->y, c2->bbWidth, c2->bbHeight);
        }

        //Circle - Circle
        bool CheckCollision(ComponentHandle<CircleCollider>& c1, ComponentHandle<CircleCollider>& c2, ComponentHandle<Transform> c1T, ComponentHandle<Transform> c2T) {
            //TODO
            float c1PosX = c1->x + c1T->x;
            float c2PosX = c2->x + c2T->x;
            float c1PosY = c1->y + c1T->y;
            float c2PosY = c2->y + c2T->y;
            float distance = abs(sqrtf(powf(c2PosX - c1PosX, 2) +powf(c2PosY - c1PosY, 2)));
            if(distance - (c1->radius + c2->radius))
                return true;
            return false;
        }

        //Capsule - Capsule
        bool CheckCollision(ComponentHandle<CapsuleCollider>& c1, ComponentHandle<CapsuleCollider>& c2, ComponentHandle<Transform> c1T, ComponentHandle<Transform> c2T) {
            //TODO
            //Capsule c1:
            float c1Rotation = c1T->rz * M_PI / 180.0f;
            glm::vec2 c1Tip = glm::vec2(c1->x, c1->y + (c1->a)/2);
            c1Tip = glm::rotate(c1Tip, c1Rotation);
            c1Tip = glm::vec2(c1Tip.x + c1T->x, c1Tip.y + c1T->y);
            glm::vec2 c1Base = glm::vec2(c1->x, c1->y - (c1->a)/2);
            c1Base = glm::rotate(c1Base, c1Rotation);
            c1Base = glm::vec2(c1Base.x + c1T->x, c1Base.y + c1T->y);
            glm::vec2 c1Normal = glm::normalize(c1Tip - c1Base);
            glm::vec2 c1LineEndOffset = c1Normal * c1->radius;
            glm::vec2 c1_A = c1Base + c1LineEndOffset;
            glm::vec2 c1_B = c1Tip + c1LineEndOffset;

            //Capsule c2:
            float c2Rotation = c2T->rz * M_PI / 180.0f;
            glm::vec2 c2Tip = glm::vec2(c2->x, c2->y + (c2->a)/2);
            c2Tip = glm::rotate(c2Tip, c2Rotation);
            c2Tip = glm::vec2(c2Tip.x + c2T->x, c2Tip.y + c2T->y);
            glm::vec2 c2Base = glm::vec2(c2->x, c2->y - (c2->a)/2);
            c2Base = glm::rotate(c2Base, c2Rotation);
            c2Base = glm::vec2(c2Base.x + c2T->x, c2Base.y + c2T->y);
            glm::vec2 c2Normal = glm::normalize(c2Tip - c2Base);
            glm::vec2 c2LineEndOffset = c2Normal * c2->radius;
            glm::vec2 c2_A = c2Base + c2LineEndOffset;
            glm::vec2 c2_B = c2Tip + c2LineEndOffset;

            // vectors between line endpoints:
            glm::vec2 v0 = c2_A - c1_A;
            glm::vec2 v1 = c2_B - c1_A; 
            glm::vec2 v2 = c2_A - c1_B; 
            glm::vec2 v3 = c2_B - c1_B;

            // squared distances:
            float d0 = glm::dot(v0, v0); 
            float d1 = glm::dot(v1, v1); 
            float d2 = glm::dot(v2, v2); 
            float d3 = glm::dot(v3, v3);

            // select best potential endpoint on capsule A:
            glm::vec2 bestA;
            if (d2 < d0 || d2 < d1 || d3 < d0 || d3 < d1)
            {
                bestA = c1_B;
            }
            else
            {
                bestA = c1_A;
            }

            // select point on capsule B line segment nearest to best potential endpoint on A capsule:
            glm::vec2 bestB = ClosestPointOnLineSegment(c2_A, c2_B, bestA);
            // now do the same for capsule A segment:
            bestA = ClosestPointOnLineSegment(c1_A, c1_B, bestB);

            //Collision calc:
            glm::vec2 penetration_normal = bestA - bestB;
            float len = length(penetration_normal);
            penetration_normal /= len;  // normalize
            float penetration_depth = c1->radius + c2->radius - len;
            return penetration_depth > 0;
        }

        //Box - Circle
        bool CheckCollision(ComponentHandle<BoxCollider>& c1, ComponentHandle<CircleCollider>& c2, ComponentHandle<Transform> c1T, ComponentHandle<Transform> c2T) {
            //TODO
            //Box stuff
            float boxPosX = c1->x + c1T->x;
            float boxPosY = c1->y + c1T->y;

            float boxRotation = c1T->rz * M_PI / 180.0f;

            //Circle
            float circleX = c2->x + c2T->x;
            float circleY = c2->y + c2T->y;

            //Rotate circle
            circleX = cosf(boxRotation) * (circleX - boxPosX) - sinf(boxRotation) * 
                (circleY - boxPosY) + boxPosX;
            circleY = sinf(boxRotation) * (circleX - boxPosX) + cosf(boxRotation) *
                (circleY - boxPosY) + boxPosY;


            //Collision stuff
            //Check first if center of the circle is inside the box:
            if(circleX > boxPosX - c1->width / 2 && circleX < boxPosX + c1->width / 2 &&
                circleY < boxPosY + c1->height / 2 && circleY > boxPosY - c1->height / 2)
                return true;
            
            //Now check if the circle is outside of the box
            float testX;
            float testY;
            if(circleX < boxPosX - c1->width / 2)
                testX = boxPosX - c1->width / 2;   //Testing left edge of box
            else if(circleX > boxPosX + c1->width / 2)
                testX = boxPosX + c1->width / 2;  //Testing right edge of box
            else
                testX = boxPosX;  //In this case the circle is between left and right edges
            
            if(circleY > boxPosY + c1->height / 2)
                testY = boxPosY + c1->height / 2;   //Testing top edge of box
            else if(circleY < boxPosY - c1->height / 2)
                testY = boxPosY - c1->height / 2;   //Testing bottom edge of box
            else
                testY = boxPosY;  //Circle is between the top and bottom edges;
            
            float distX = circleX - testX;
            float distY = circleY - testY;
            float distance = sqrtf(powf(distX, 2) + powf(distY, 2));

            if(distance < c2->radius)
                return true;
            return false;
        }

        //Circle - Box (Calls Box - Circle)
        bool CheckCollision(ComponentHandle<CircleCollider>& c1, ComponentHandle<BoxCollider>& c2, ComponentHandle<Transform> c1T, ComponentHandle<Transform> c2T) {
            return CheckCollision(c2, c1, c2T, c1T);
        }

        //Box - Capsule
        bool CheckCollision(ComponentHandle<BoxCollider>& c1, ComponentHandle<CapsuleCollider>& c2, ComponentHandle<Transform> c1T, ComponentHandle<Transform> c2T) {
            //TODO
            //Box c1:
            glm::vec2 topLeft = glm::vec2(0 - c1->width / 2, 0 + c1->height / 2);
            glm::vec2 topRight = glm::vec2(0 + c1->width / 2, 0 + c1->height / 2);
            glm::vec2 bottomLeft = glm::vec2(0 - c1->width / 2, 0 - c1->height / 2);
            glm::vec2 bottomRight = glm::vec2(0 + c1->width / 2, 0 - c1->height / 2);
            //Rotate box corners and move to actual position:
            float theta = c1T->rz * M_PI / 180.0f;
            topLeft = glm::rotate(topLeft, theta);
            topLeft = glm::vec2(topLeft.x + c1T->x, topLeft.y + c1T->y);
            topRight = glm::rotate(topRight, theta);
            topRight = glm::vec2(topRight.x + c1T->x, topRight.y + c1T->y);
            bottomLeft = glm::rotate(bottomLeft, theta);
            bottomLeft = glm::vec2(bottomLeft.x + c1T->x, bottomLeft.y + c1T->y);
            bottomRight = glm::rotate(bottomRight, theta);
            bottomRight = glm::vec2(bottomRight.x + c1T->x, bottomRight.y + c1T->y);

            //Capsule c2:
            float c2Rotation = c2T->rz * M_PI / 180.0f;
            glm::vec2 c2Tip = glm::rotate(glm::vec2(c2->x + c2T->x, c2->y + c2T->y + (c2->a)/2), c2Rotation);
            glm::vec2 c2Base = glm::rotate(glm::vec2(c2->x + c2T->x, c2->y + c2T->y - (c2->a)/2), c2Rotation);
            glm::vec2 c2Normal = glm::normalize(c2Tip - c2Base);
            glm::vec2 c2LineEndOffset = c2Normal * c2->radius;
            glm::vec2 c2_A = c2Base + c2LineEndOffset;
            glm::vec2 c2_B = c2Tip + c2LineEndOffset;

            //Test both ends of the capsule to all edges of the rectangle
            //Test the tip against the rect points:
            glm::vec2 tip2TL = topLeft - c2Tip;
            glm::vec2 tip2TR = topRight - c2Tip;
            glm::vec2 tip2BL = bottomLeft - c2Tip;
            glm::vec2 tip2BR = bottomRight - c2Tip;

            float d1T = glm::dot(tip2TL, tip2TL);
            float d2T = glm::dot(tip2TR, tip2TR);
            float d3T = glm::dot(tip2BL, tip2BL);
            float d4T = glm::dot(tip2BR, tip2BR);

            float testMin = d1T;
            if(testMin > d2T)
                testMin = d2T;
            if(testMin > d3T)
                testMin = d3T;
            if(testMin > d4T)
                testMin = d4T;

            glm::vec2 bestB = c2Tip;

            //Test the base against the rect points:
            glm::vec2 base2TL = topLeft - c2Base;
            glm::vec2 base2TR = topRight - c2Base;
            glm::vec2 base2BL = bottomLeft - c2Base;
            glm::vec2 base2BR = bottomRight - c2Base;

            float d1B = glm::dot(base2TL, base2TL);
            float d2B = glm::dot(base2TR, base2TR);
            float d3B = glm::dot(base2BL, base2BL);
            float d4B = glm::dot(base2BR, base2BR);

            if(testMin > d1B || testMin > d2B || testMin > d3B || testMin > d4B)
                bestB = c2Base;

            //Find the closest point on the rect to the selected capsule end point
            glm::vec2 closeTop = ClosestPointOnLineSegment(topLeft, topRight, bestB);
            glm::vec2 closeRight = ClosestPointOnLineSegment(topRight, bottomRight, bestB);
            glm::vec2 closeBottom = ClosestPointOnLineSegment(bottomLeft, bottomRight, bestB);
            glm::vec2 closeLeft = ClosestPointOnLineSegment(topLeft, bottomLeft, bestB);

            float minDistance = glm::length(closeTop - bestB);
            float bToRight = glm::length(closeRight - bestB);
            float bToBottom = glm::length(closeBottom - bestB);
            float bToLeft = glm::length(closeLeft - bestB);

            glm::vec2 bestA = closeTop;
            if(minDistance > bToRight)
                bestA = closeRight;
            if(minDistance > bToBottom)
                bestA = closeBottom;
            if(minDistance > bToLeft)
                bestA = closeLeft;

            //Find the closest point on the capsule to the closest edge of the rect
            bestB = ClosestPointOnLineSegment(c2Tip, c2Base, bestA);

            //First rotate the test point with respect to the rect's rotation for easy calc:
            float boxPosX = c1->x + c1T->x;
            float boxPosY = c1->y + c1T->y;
            glm::vec2 bestBRotated = glm::vec2(cosf(theta) * (bestB.x - boxPosX) - sinf(theta) * 
                (bestB.y - boxPosY) + boxPosX, sinf(theta) * (bestB.x - boxPosX) + cosf(theta) *
                (bestB.y - boxPosY) + boxPosY);
            

            //Collision calc:
            //First test if the point on the capsule is inside the rect:
            if(bestBRotated.x > c1->x - c1->width / 2 && bestBRotated.x < c1->x + c1->width / 2 &&
                bestBRotated.y < c1->y + c1->height / 2 && bestBRotated.y > c1->y - c1->height / 2)
                return true;
            
            //Now do test for outside the rect with original points on capsule and rect:
            glm::vec2 penetration_normal = bestA - bestB;
            float len = length(penetration_normal);
            penetration_normal /= len;  // normalize
            float penetration_depth = c2->radius - len;
            return penetration_depth > 0;
        }

        //Capsule - Box (Calls Box - Capsule)
        bool CheckCollision(ComponentHandle<CapsuleCollider>& c1, ComponentHandle<BoxCollider>& c2, ComponentHandle<Transform> c1T, ComponentHandle<Transform> c2T) {
            return CheckCollision(c2, c1, c2T, c1T);
        }

        //Circle - Capsule
        bool CheckCollision(ComponentHandle<CircleCollider>& c1, ComponentHandle<CapsuleCollider>& c2, ComponentHandle<Transform> c1T, ComponentHandle<Transform> c2T) {
            //Capsule c2:
            float c2Rotation = c2T->rz * M_PI / 180.0f;
            glm::vec2 c2Tip = glm::rotate(glm::vec2(c2->x + c2T->x, c2->y + c2T->y + (c2->a)/2), c2Rotation);
            glm::vec2 c2Base = glm::rotate(glm::vec2(c2->x + c2T->x, c2->y + c2T->y - (c2->a)/2), c2Rotation);
            glm::vec2 c2Normal = glm::normalize(c2Tip - c2Base);
            glm::vec2 c2LineEndOffset = c2Normal * c2->radius;
            glm::vec2 c2_A = c2Base + c2LineEndOffset;
            glm::vec2 c2_B = c2Tip + c2LineEndOffset;

            //Circle c1:
            glm::vec2 c1_Point = glm::vec2(c1->x + c1T->x, c1->y + c1T->y);

            //Selecting point on the capsule closest to the circle:
            glm::vec2 c2_Best = ClosestPointOnLineSegment(c2_A, c2_B, c1_Point);

            //Collision calculation:
            glm::vec2 penetration_normal = c1_Point - c2_Best;
            float len = length(penetration_normal);
            penetration_normal /= len;  // normalize
            float penetration_depth = c1->radius + c2->radius - len;
            return penetration_depth > 0;
        }

        //Capsule - Circle (Calls Circle - Capsule)
        bool CheckCollision(ComponentHandle<CapsuleCollider>& c1, ComponentHandle<CircleCollider>& c2, ComponentHandle<Transform> c1T, ComponentHandle<Transform> c2T) {
            return CheckCollision(c2, c1, c2T, c1T);
        }

        bool DetectAABB(float x1, float y1, float width1, float height1,
            float x2, float y2, float width2, float height2){
            if (x1 <= x2 + width2 && x1 + width1 >= x2 && y1 <= y2 + height2 &&
            y1 + height1 >= y2){
                return true;
            }
            return false;
        }

        glm::vec2 ClosestPointOnLineSegment(glm::vec2 A, glm::vec2 B, glm::vec2 Point)
        {
            glm::vec2 AB = B - A;
            float t = glm::dot(Point - A, AB) / glm::dot(AB, AB);
            return A + min(max(t, 0.0f), 1.0f) * AB; // saturate(t) can be written as: min((max(t, 0), 1)
        }

        void PerformCollisionCalculations(EntityPair collision) {

            ComponentHandle<Transform> aTrans = collision.a.component<Transform>();
            ComponentHandle<Transform> bTrans = collision.b.component<Transform>();
            ComponentHandle<Rigidbody_2D> aRB = collision.a.component<Rigidbody_2D>();
            ComponentHandle<Rigidbody_2D> bRB = collision.b.component<Rigidbody_2D>();

            float aMass = aRB->mass;
            float bMass = bRB->mass;

            float xDist = (bTrans->x + bRB->cmX) - (aTrans->x + aRB->cmX);
            float yDist = (bTrans->y + bRB->cmY) - (aTrans->y + aRB->cmY);

            float phi = atan2f(yDist, xDist);

            float vAI = sqrtf(powf(aRB->velocityX, 2.0f) + powf(aRB->velocityY, 2.0f));
            float iThetaA = atan2f(aRB->velocityY, aRB->velocityX);

            float vBI = sqrtf(powf(bRB->velocityX, 2.0f) + powf(bRB->velocityY, 2.0f));
            float iThetaB = atan2f(bRB->velocityY, bRB->velocityX);

            float vAFX = ((vAI * cosf(iThetaA - phi) * (aMass - bMass) + 2 * bMass * vBI * cosf(iThetaB - phi)) / (aMass + bMass)) * 
            cosf(phi) + vAI * sinf(iThetaA - phi) * cosf(phi + M_PI_2);

            float vAFY = ((vAI * cosf(iThetaA - phi) * (aMass - bMass) + 2 * bMass * vBI * cosf(iThetaB - phi)) / (aMass + bMass)) * 
            sinf(phi) + vAI * sinf(iThetaA - phi) * sinf(phi + M_PI_2);

            float vBFX = ((vBI * cosf(iThetaB - phi) * (bMass - aMass) + 2 * aMass * vAI * cosf(iThetaA - phi)) / (aMass + bMass)) * 
            cosf(phi) + vBI * sinf(iThetaB - phi) * cosf(phi + M_PI_2);

            float vBFY = ((vBI * cosf(iThetaB - phi) * (bMass - aMass) + 2 * aMass * vAI * cosf(iThetaA - phi)) / (aMass + bMass)) * 
            sinf(phi) + vBI * sinf(iThetaB - phi) * sinf(phi + M_PI_2);

            aRB->velocityX = vAFX;
            aRB->velocityY = vAFY;
            bRB->velocityX = vBFX;
            bRB->velocityY = vBFY;
        }

        void PerformPhysicsWithOneRigidBody(Entity hasRigidbody, Entity noRigidbody){
            ComponentHandle<Transform> hRBTrans = hasRigidbody.component<Transform>();
            ComponentHandle<Transform> nRBTrans = noRigidbody.component<Transform>();
            ComponentHandle<Rigidbody_2D> hRB = hasRigidbody.component<Rigidbody_2D>();
            ComponentHandle<CircleCollider> hRBCC;
            ComponentHandle<BoxCollider> hRBBC;
            ComponentHandle<CapsuleCollider> hRBCapC;
            glm::vec2 rbVelocity = glm::vec2(hRB->velocityX, hRB->velocityY);

            //Figure out what kind of collider the rigidbody has:
            if(hasRigidbody.has_component<CircleCollider>()){{
                hRBCC = hasRigidbody.component<CircleCollider>();
                if(hRBCC->isTrigger){
                    ;//Notify
                }
            }
            if(hasRigidbody.has_component<BoxCollider>()){
                hRBBC = hasRigidbody.component<BoxCollider>();
                if(hRBBC->isTrigger){
                    ;//Notify
                }
            if(hasRigidbody.has_component<CapsuleCollider>()){
                hRBCapC = hasRigidbody.component<CapsuleCollider>();
                if(hRBCapC->isTrigger){
                    ;//Notify
                }
            }
            if(noRigidbody.has_component<CircleCollider>()){
                ComponentHandle<CircleCollider> nRBCollider = noRigidbody.component<CircleCollider>();
                //Check for trigger:
                if(nRBCollider->isTrigger){
                    ;//Notify
                    return;
                }

                //Find the vector of the angle between the center of the two bodies
                glm::vec2 directionFromNoRBToRB = glm::vec2(hRBTrans->x + hRB->cmX - nRBTrans->x + nRBCollider->x, 
                    hRBTrans->y + hRB->cmY - nRBTrans->y + nRBCollider->y);

                //Calculate the reflection of the velocity vector:
                glm::vec2 newVelocity = rbVelocity - (2 * glm::dot(rbVelocity, directionFromNoRBToRB)) / 
                    glm::dot(directionFromNoRBToRB, directionFromNoRBToRB) * directionFromNoRBToRB;

                hRB->velocityX = newVelocity.x;
                hRB->velocityY = newVelocity.y;

            } else {
                //Collider bounding box points setup:
                glm::vec2 topLeft;
                glm::vec2 topRight;
                glm::vec2 bottomLeft;
                glm::vec2 bottomRight;
                if(noRigidbody.has_component<BoxCollider>()){
                    ComponentHandle<BoxCollider> nRBCollider = noRigidbody.component<BoxCollider>();
                    //Check for trigger:
                    if(nRBCollider->isTrigger){
                        ;//Notify
                        return;
                    }
                    float theta = nRBTrans->rz * M_PI / 180.0f;
                    topLeft = glm::rotate(glm::vec2(0 - nRBCollider->bbWidth / 2, nRBCollider->bbHeight / 2), theta);
                    topLeft = glm::vec2(topLeft.x + nRBTrans->x + nRBCollider->x, topLeft.y + nRBTrans->y + nRBCollider->y);
                    topRight = glm::rotate(glm::vec2(nRBCollider->bbWidth / 2, nRBCollider->bbHeight / 2), theta);
                    topRight = glm::vec2(topRight.x + nRBTrans->x + nRBCollider->x, topRight.y + nRBTrans->y + nRBCollider->y);
                    bottomLeft = glm::rotate(glm::vec2(0 - nRBCollider->bbWidth / 2, 0 - nRBCollider->bbHeight / 2), theta);
                    bottomLeft = glm::vec2(bottomLeft.x + nRBTrans->x + nRBCollider->x, bottomLeft.y + nRBTrans->y + nRBCollider->y);
                    bottomRight = glm::rotate(glm::vec2(nRBCollider->bbWidth / 2, 0 - nRBCollider->bbHeight / 2), theta);
                    bottomRight = glm::vec2(bottomRight.x + nRBTrans->x + nRBCollider->x, bottomRight.y + nRBTrans->y + nRBCollider->y);
                }
                else if(noRigidbody.has_component<CapsuleCollider>()){
                    ComponentHandle<CapsuleCollider> nRBCollider = noRigidbody.component<CapsuleCollider>();
                    //Check for trigger:
                    if(nRBCollider->isTrigger){
                        ;//Notify
                        return;
                    }
                    float theta = nRBTrans->rz * M_PI / 180.0f;
                    topLeft = glm::rotate(glm::vec2(0 - nRBCollider->bbWidth / 2, nRBCollider->bbHeight / 2), theta);
                    topLeft = glm::vec2(topLeft.x + nRBTrans->x + nRBCollider->x, topLeft.y + nRBTrans->y + nRBCollider->y);
                    topRight = glm::rotate(glm::vec2(nRBCollider->bbWidth / 2, nRBCollider->bbHeight / 2), theta);
                    topRight = glm::vec2(topRight.x + nRBTrans->x + nRBCollider->x, topRight.y + nRBTrans->y + nRBCollider->y);
                    bottomLeft = glm::rotate(glm::vec2(0 - nRBCollider->bbWidth / 2, 0 - nRBCollider->bbHeight / 2), theta);
                    bottomLeft = glm::vec2(bottomLeft.x + nRBTrans->x + nRBCollider->x, bottomLeft.y + nRBTrans->y + nRBCollider->y);
                    bottomRight = glm::rotate(glm::vec2(nRBCollider->bbWidth / 2, 0 - nRBCollider->bbHeight / 2), theta);
                    bottomRight = glm::vec2(bottomRight.x + nRBTrans->x + nRBCollider->x, bottomRight.y + nRBTrans->y + nRBCollider->y);
                }
                //Center of mass of the rigidbody
                glm::vec2 rbCoM = glm::vec2(hRBTrans->x + hRB->cmX, hRB->cmY - nRBTrans->y);

                //Vectors of the edges of the bounding box:            
                //Test the tip against the rect points:
                glm::vec2 tip2TL = topLeft - rbCoM;
                glm::vec2 tip2TR = topRight - rbCoM;
                glm::vec2 tip2BL = bottomLeft - rbCoM;
                glm::vec2 tip2BR = bottomRight - rbCoM;

                //Closest points on the rect to the CoM
                glm::vec2 closestPoint;
                glm::vec2 nextClosest;

                float minDistance = glm::length(tip2TL);
                closestPoint = topLeft;
                float nextMin = glm::length(tip2TR);
                nextClosest = topRight;

                if(glm::length(tip2TL) < glm::length(tip2TR)){
                    float minDistance = glm::length(tip2TL);
                    closestPoint = topLeft;
                    float nextMin = glm::length(tip2TR);
                    nextClosest = topRight;
                } else {
                    float minDistance = glm::length(tip2TR);
                    closestPoint = topRight;
                    float nextMin = glm::length(tip2TL);
                    nextClosest = topLeft;
                }

                if(glm::length(tip2BL) < minDistance){
                    nextMin = minDistance;
                    nextClosest = closestPoint;
                    minDistance = glm::length(tip2BL);
                    closestPoint = bottomLeft;
                } else if(glm::length(tip2BL) < nextMin){
                    nextMin = glm::length(tip2BL);
                    nextClosest = bottomLeft;
                }
                if(glm::length(tip2BR) < minDistance){
                    nextMin = minDistance;
                    nextClosest = closestPoint;
                    minDistance = glm::length(tip2BR);
                    closestPoint = bottomRight;
                } else if(glm::length(tip2BR) < nextMin){
                    nextMin = glm::length(tip2BR);
                    nextClosest = bottomRight;
                }

                glm::vec2 bestPointOnRect = ClosestPointOnLineSegment(closestPoint, nextClosest, rbCoM);
                glm::vec2 reflectNorm = rbCoM - bestPointOnRect;

                //Calculate the reflection of the velocity vector:
                glm::vec2 newVelocity = rbVelocity - (2 * glm::dot(rbVelocity, reflectNorm)) / 
                    glm::dot(reflectNorm, reflectNorm) * reflectNorm;

                hRB->velocityX = newVelocity.x;
                hRB->velocityY = newVelocity.y;
            }
        }

        void UpdateVelocityAndAcceleration(Entity e, TimeDelta dt, float thrust = 0.0f){
            ComponentHandle<Rigidbody_2D> rb = e.component<Rigidbody_2D>();
            float tau = rb->mass / rb->linDrag;
            float t = (float)dt;
            //update velocity
            rb->velocityX = (1.0f/rb->linDrag)*(thrust - expf(-1 * rb->linDrag * t / rb->mass) * (thrust - rb->linDrag * rb->velocityX));
            rb->velocityY = (1.0f/rb->linDrag)*(thrust - expf(-1 * rb->linDrag * t / rb->mass) * (thrust - rb->linDrag * rb->velocityY));
            //update acceleration
            rb->accelerationX = (thrust - rb->linDrag * rb->velocityX) / rb->mass;
            rb->accelerationY = (thrust - rb->linDrag * rb->velocityY) / rb->mass;

            return;
        }

        //Updates the move on the rigidbody with detection of collisions along the way
        void RigidbodyMoveTo(EntityManager& es, Entity e, float x, float y){
            int checkInterval = 10;
            ComponentHandle<Transform>transform = e.component<Transform>();
            float xStep = (x - transform->x) / checkInterval;
            float yStep = (y - transform->y) / checkInterval;

            for(int i = 0; i <= checkInterval; i++){
                //Step 1:  Perform step
                transform->x += xStep;
                transform->y += yStep;
                //Step 2: Detect collisions
                std::vector<EntityPair> pairs = broadphase(es); //returns pairs of possible collisions
                std::vector<EntityPair> collideWithMe;  //Container for all the collisions involving the entity
                std::vector<EntityPair> narrowPhaseCollisions;
                for(EntityPair p : pairs){
                    if(p.a == e || p.b == e)
                        collideWithMe.push_back(p);
                }

                if(collideWithMe.size() > 0)
                    narrowPhaseCollisions = narrowphase(collideWithMe);

                if(narrowPhaseCollisions.size() > 0){
                    //Reverse step:
                    transform->x -= xStep;
                    transform->y -= yStep;
                    return;     //Leave function after backstep instead of continuing
                }
            }
        }

        #pragma endregion //collision algorithms

        std::vector<EntityPair> broadphase(EntityManager& es) {
            //might need performance boost - right now it's sorting every frame,
            //could keep it sorted between frames but would need a way to track new/moving colliders

            //sort and sweep
            //get all colliders
            auto boxEntities = es.entities_with_components<BoxCollider>();
            auto circleEntities = es.entities_with_components<CircleCollider>();
            auto capsuleEntities = es.entities_with_components<CapsuleCollider>();

            // colliders require b and e attributes (along x) - type float
            //get each collider's b & e
            //and put all b & e in a list
            std::vector<SASObject> sas;
            for (Entity e : boxEntities) {
                ComponentHandle<BoxCollider> handle = e.component<BoxCollider>();
                ComponentHandle<Transform> handleT = e.component<Transform>();
                sas.emplace_back(SASObject(handle->b + handleT->x, e, true, Box));
                sas.emplace_back(SASObject(handle->e + handleT->x, e, false, Box));
            }
            for (Entity e : circleEntities) {
                ComponentHandle<CircleCollider> handle = e.component<CircleCollider>();
                ComponentHandle<Transform> handleT = e.component<Transform>();
                sas.emplace_back(SASObject(handle->b + handleT->x, e, true, Circle));
                sas.emplace_back(SASObject(handle->e + handleT->x, e, false, Circle));
            }
            for (Entity e : capsuleEntities) {
                ComponentHandle<CapsuleCollider> handle = e.component<CapsuleCollider>();
                ComponentHandle<Transform> handleT = e.component<Transform>();
                sas.emplace_back(SASObject(handle->b + handleT->x, e, true, Capsule));
                sas.emplace_back(SASObject(handle->e + handleT->x, e, false, Capsule));
            }
            //pretty sure that each entity can only have one component of each type, i.e. no duplicates.
            //If this is the case it doesnt make sense for an entity to be able to have two different types of components either
            //solution: Generic Collider component that is either modified, or has children types
            //for now just assume only has box collider

            //sort the list
            std::sort(sas.begin(), sas.end());
            //sweep
            std::vector<SASObject> active;
            std::vector<EntityPair> possibleCollides;
            for (auto it = sas.begin(); it != sas.end(); ++it) {
                if ((*it).isBegin) {
                    //add to active
                    active.push_back(*it);
                } else {
                    //remove match from active
                    //when removing an SASObject, perform AABB check between this obj and all active ones
                    std::vector<SASObject>::iterator oToRemove; // to remember where to remove
                    for (auto it2 = active.begin(); it2 != active.end(); ++it2) {
                        if ((*it).e == (*it2).e) {
                            //they have the same entity, begin and end are *it and *it2
                            //remove begin
                            oToRemove = it2; //might be a reference issue here
                        } else {
                            Entity e1 = (*it).e;
                            Entity e2 = (*it2).e;
                            ComponentHandle<Transform> c1T = e1.component<Transform>();
                            ComponentHandle<Transform> c2T = e2.component<Transform>();
                            //Get each components' types and test AABB
                            if ((*it).type == Box) {
                                ComponentHandle<BoxCollider> c1 = e1.component<BoxCollider>();
                                if ((*it2).type == Box) {
                                    ComponentHandle<BoxCollider> c2 = e2.component<BoxCollider>();
                                    if (DetectAABB(c1->x + c1T->x, c1->y + c1T->y, c1->bbWidth, c1->bbHeight, c2->x + c2T->x, c2->y + c2T->y, c2->bbWidth, c2->bbHeight)) {
                                        possibleCollides.emplace_back(EntityPair((*it).e, Box, (*it2).e, Box));
                                    }
                                } else if ((*it2).type == Circle) {
                                    ComponentHandle<CircleCollider> c2 = e2.component<CircleCollider>();
                                    if (DetectAABB(c1->x + c1T->x, c1->y + c1T->y, c1->bbWidth, c1->bbHeight, c2->x + c2T->x, c2->y + c2T->y, c2->bbWidth, c2->bbHeight)) {
                                        possibleCollides.emplace_back(EntityPair((*it).e, Box, (*it2).e, Circle));
                                    }
                                } else if ((*it2).type == Capsule) {
                                    ComponentHandle<CapsuleCollider> c2 = e2.component<CapsuleCollider>();
                                    if (DetectAABB(c1->x + c1T->x, c1->y + c1T->y, c1->bbWidth, c1->bbHeight, c2->x + c2T->x, c2->y + c2T->y, c2->bbWidth, c2->bbHeight)) {
                                        possibleCollides.emplace_back(EntityPair((*it).e, Box, (*it2).e, Capsule));
                                    }
                                }
                            } else if ((*it).type == Circle) {
                                ComponentHandle<CircleCollider> c1 = e1.component<CircleCollider>();
                                if ((*it2).type == Box) {
                                    ComponentHandle<BoxCollider> c2 = e2.component<BoxCollider>();
                                    if (DetectAABB(c1->x + c1T->x, c1->y + c1T->y, c1->bbWidth, c1->bbHeight, c2->x + c2T->x, c2->y + c2T->y, c2->bbWidth, c2->bbHeight)) {
                                        possibleCollides.emplace_back(EntityPair((*it).e, Circle, (*it2).e, Box));
                                    }
                                } else if ((*it2).type == Circle) {
                                    ComponentHandle<CircleCollider> c2 = e2.component<CircleCollider>();
                                    if (DetectAABB(c1->x + c1T->x, c1->y + c1T->y, c1->bbWidth, c1->bbHeight, c2->x + c2T->x, c2->y + c2T->y, c2->bbWidth, c2->bbHeight)) {
                                        possibleCollides.emplace_back(EntityPair((*it).e, Circle, (*it2).e, Circle));
                                    }
                                } else if ((*it2).type == Capsule) {
                                    ComponentHandle<CapsuleCollider> c2 = e2.component<CapsuleCollider>();
                                    if (DetectAABB(c1->x + c1T->x, c1->y + c1T->y, c1->bbWidth, c1->bbHeight, c2->x + c2T->x, c2->y + c2T->y, c2->bbWidth, c2->bbHeight)) {
                                        possibleCollides.emplace_back(EntityPair((*it).e, Circle, (*it2).e, Capsule));
                                    }
                                }
                            } else if ((*it).type == Capsule) {
                                ComponentHandle<CapsuleCollider> c1 = e1.component<CapsuleCollider>();
                                if ((*it2).type == Box) {
                                    ComponentHandle<BoxCollider> c2 = e2.component<BoxCollider>();
                                    if (DetectAABB(c1->x + c1T->x, c1->y + c1T->y, c1->bbWidth, c1->bbHeight, c2->x + c2T->x, c2->y + c2T->y, c2->bbWidth, c2->bbHeight)) {
                                        possibleCollides.emplace_back(EntityPair((*it).e, Capsule, (*it2).e, Box));
                                    }
                                } else if ((*it2).type == Circle) {
                                    ComponentHandle<CircleCollider> c2 = e2.component<CircleCollider>();
                                    if (DetectAABB(c1->x + c1T->x, c1->y + c1T->y, c1->bbWidth, c1->bbHeight, c2->x + c2T->x, c2->y + c2T->y, c2->bbWidth, c2->bbHeight)) {
                                        possibleCollides.emplace_back(EntityPair((*it).e, Capsule, (*it2).e, Circle));
                                    }
                                } else if ((*it2).type == Capsule) {
                                    ComponentHandle<CapsuleCollider> c2 = e2.component<CapsuleCollider>();
                                    if (DetectAABB(c1->x + c1T->x, c1->y + c1T->y, c1->bbWidth, c1->bbHeight, c2->x + c2T->x, c2->y + c2T->y, c2->bbWidth, c2->bbHeight)) {
                                        possibleCollides.emplace_back(EntityPair((*it).e, Capsule, (*it2).e, Capsule));
                                    }
                                }
                            }
                        }
                    }
                    active.erase(oToRemove);
                }
            }
            return possibleCollides;
        }
};