#include "CcdPhysicsController.h"

#include "Dynamics/RigidBody.h"
#include "PHY_IMotionState.h"
#include "BroadphaseCollision/BroadphaseProxy.h"
#include "CollisionShapes/ConvexShape.h"

class BP_Proxy;

bool gEnableSleeping = true;//false;//true;
#include "Dynamics/MassProps.h"

SimdVector3 startVel(0,0,0);//-10000);
CcdPhysicsController::CcdPhysicsController (const CcdConstructionInfo& ci)
{
	m_collisionDelay = 0;

	m_sleepingCounter = 0;

	m_MotionState = ci.m_MotionState;


	SimdTransform trans;
	float tmp[3];
	m_MotionState->getWorldPosition(tmp[0],tmp[1],tmp[2]);
	trans.setOrigin(SimdVector3(tmp[0],tmp[1],tmp[2]));

	SimdQuaternion orn;
	m_MotionState->getWorldOrientation(orn[0],orn[1],orn[2],orn[3]);
	trans.setRotation(orn);

	MassProps mp(ci.m_mass, ci.m_localInertiaTensor);

	m_body = new RigidBody(mp,0,0);
	
	m_broadphaseHandle = ci.m_broadphaseHandle;

	m_collisionShape = ci.m_collisionShape;

	//
	// init the rigidbody properly
	//
	
	m_body->setMassProps(ci.m_mass, ci.m_localInertiaTensor);
	m_body->setGravity( ci.m_gravity);

	m_friction = ci.m_friction;
	m_restitution = ci.m_restitution;

	m_body->setDamping(ci.m_linearDamping, ci.m_angularDamping);

	
	m_body->setCenterOfMassTransform( trans );

	#ifdef WIN32
	if (m_body->getInvMass())
		m_body->setLinearVelocity(startVel);
	#endif

}

CcdPhysicsController::~CcdPhysicsController()
{
	//will be reference counted, due to sharing
	//delete m_collisionShape;
	delete m_MotionState;
	delete m_body;
}

		/**
			SynchronizeMotionStates ynchronizes dynas, kinematic and deformable entities (and do 'late binding')
		*/
bool		CcdPhysicsController::SynchronizeMotionStates(float time)
{
	const SimdVector3& worldPos = m_body->getCenterOfMassPosition();
	m_MotionState->setWorldPosition(worldPos[0],worldPos[1],worldPos[2]);
	
	const SimdQuaternion& worldquat = m_body->getOrientation();
	m_MotionState->setWorldOrientation(worldquat[0],worldquat[1],worldquat[2],worldquat[3]);

	m_MotionState->calculateWorldTransformations();

	float scale[3];
	m_MotionState->getWorldScaling(scale[0],scale[1],scale[2]);
	
	SimdVector3 scaling(scale[0],scale[1],scale[2]);
	m_collisionShape->setLocalScaling(scaling);


	return true;
}

		/**
			WriteMotionStateToDynamics synchronizes dynas, kinematic and deformable entities (and do 'late binding')
		*/
		
void		CcdPhysicsController::WriteMotionStateToDynamics(bool nondynaonly)
{

}
void		CcdPhysicsController::WriteDynamicsToMotionState()
{
}
		// controller replication
void		CcdPhysicsController::PostProcessReplica(class PHY_IMotionState* motionstate,class PHY_IPhysicsController* parentctrl)
{
}

		// kinematic methods
void		CcdPhysicsController::RelativeTranslate(float dlocX,float dlocY,float dlocZ,bool local)
{
	SimdTransform xform = m_body->getCenterOfMassTransform();
	xform.setOrigin(xform.getOrigin() + SimdVector3(dlocX,dlocY,dlocZ));
	this->m_body->setCenterOfMassTransform(xform);

}

void		CcdPhysicsController::RelativeRotate(const float drot[9],bool local)
{
}
void		CcdPhysicsController::getOrientation(float &quatImag0,float &quatImag1,float &quatImag2,float &quatReal)
{
}
void		CcdPhysicsController::setOrientation(float quatImag0,float quatImag1,float quatImag2,float quatReal)
{
}
void		CcdPhysicsController::setPosition(float posX,float posY,float posZ)
{
}
void		CcdPhysicsController::resolveCombinedVelocities(float linvelX,float linvelY,float linvelZ,float angVelX,float angVelY,float angVelZ)
{
}

void 		CcdPhysicsController::getPosition(PHY__Vector3&	pos) const
{
	assert(0);
}

void		CcdPhysicsController::setScaling(float scaleX,float scaleY,float scaleZ)
{
}
		
		// physics methods
void		CcdPhysicsController::ApplyTorque(float torqueX,float torqueY,float torqueZ,bool local)
{
}
void		CcdPhysicsController::ApplyForce(float forceX,float forceY,float forceZ,bool local)
{
}
void		CcdPhysicsController::SetAngularVelocity(float ang_velX,float ang_velY,float ang_velZ,bool local)
{
	SimdVector3 angvel(ang_velX,ang_velY,ang_velZ);

	m_body->setAngularVelocity(angvel);

}
void		CcdPhysicsController::SetLinearVelocity(float lin_velX,float lin_velY,float lin_velZ,bool local)
{

	SimdVector3 linVel(lin_velX,lin_velY,lin_velZ);
	m_body->setLinearVelocity(linVel);
}
void		CcdPhysicsController::applyImpulse(float attachX,float attachY,float attachZ, float impulseX,float impulseY,float impulseZ)
{
}
void		CcdPhysicsController::SetActive(bool active)
{
}
		// reading out information from physics
void		CcdPhysicsController::GetLinearVelocity(float& linvX,float& linvY,float& linvZ)
{
}
void		CcdPhysicsController::GetVelocity(const float posX,const float posY,const float posZ,float& linvX,float& linvY,float& linvZ)
{
}
void		CcdPhysicsController::getReactionForce(float& forceX,float& forceY,float& forceZ)
{
}

		// dyna's that are rigidbody are free in orientation, dyna's with non-rigidbody are restricted 
void		CcdPhysicsController::setRigidBody(bool rigid)
{
}

		// clientinfo for raycasts for example
void*		CcdPhysicsController::getNewClientInfo()
{
	return 0;
}
void		CcdPhysicsController::setNewClientInfo(void* clientinfo)
{

}

#ifdef WIN32
float gSleepingTreshold = 0.8f;
float gAngularSleepingTreshold = 1.f;

#else

float gSleepingTreshold = 0.8f;
float gAngularSleepingTreshold = 1.0f;
#endif


bool CcdPhysicsController::wantsSleeping()
{

	if (!gEnableSleeping)
		return false;

	if ( (m_body->GetActivationState() == 3) || (m_body->GetActivationState() == 2))
		return true;

	if ((m_body->getLinearVelocity().length2() < gSleepingTreshold*gSleepingTreshold) &&
		(m_body->getAngularVelocity().length2() < gAngularSleepingTreshold*gAngularSleepingTreshold))
	{
		m_sleepingCounter++;
	} else
	{
		m_sleepingCounter=0;
	}

	if (m_sleepingCounter> 150)
	{
		return true;
	}
	return false;
}

