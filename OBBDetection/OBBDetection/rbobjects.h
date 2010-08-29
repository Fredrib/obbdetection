/*************************************************************
**					Rigid Body Objects						**
**			-> Store object states, Summer 2009				**
**************************************************************
File:		rbobjects.h
Author:		Jonathan Tompson
e-mail:		jjt2119@columbia.edu or jonathantompson@gmail.com

D. Baraff and A. Witkin's "Bypically Based Modeling: Principles and Practice
was very useful.  I used their code as a starting point.

want to do this eventually!
Some codes use hierarchical trees of proxy geometry, e.g., build a capsule (cylinder 
with spherical endcaps) for each limb of an articulated character. There are algorithms 
for building these proxy geometry hierarchies in an automated way. For example, do a 
search on OBBTree (oriented bounding box tree), and you should find papers from the 
University of North Carolina @ Chapel Hill on a technique to automatically build a 
hierarchical tree that uses rectanguloid-shaped boxes for the proxy geometry. */

#ifndef rbobject_h
#define rbobject_h

#include	"main.h"
#include	"d3dformats.h"
#include	"obbtree.h"

// ******************************
// Rigid body object parent class
// ******************************
class int3; class double3;
class rbobject
{
public:
	friend class rboMesh;
	friend class obbox;
//	friend class rboIndex;
							rbobject(void);		// default Constructor

	inline void				SetScale(float newscale) { scale = newscale; dirtyScaleMatrix = true; }
	inline void				SetPosition(D3DXVECTOR3 newpos) { x = newpos; dirtyTransMatrix = true; }
	inline void				SetRotation(D3DXQUATERNION newrot) { q = newrot; dirtyRotMatrix = true; }
	inline void				SetLinearMomentum(D3DXVECTOR3 newP) { P = newP; }
	inline void				SetAngularMomentum(D3DXVECTOR3 newL) { L = newL; }
	inline void				SetAngularVelocity(D3DXVECTOR3 newOmega) { omega = newOmega; }
	void					UpdateBoundingBox(void); // Transform BB into world space
	void					UpdateMatricies(void);
	void					GetItensorBox(D3DXVECTOR3 * maxBound, D3DXMATRIX *Ib, D3DXMATRIX *Ib_inv);
	void					GetItensor(double3 * verts, int3 * ind, DWORD numFaces, D3DXMATRIX *Ib, D3DXMATRIX *Ib_inv);

	static void				StateToArray(rbobject * rb, float *y);
	static void				ArrayToState(rbobject * rb, float *y);
	static void				ObjectsToState(float *yout);	// Store the state for all the rigid bodies into y[]
	static void				StateToObjects(float *yin);	// Load the state for all the rigid bodies from y[]

	// virtual members
	inline virtual void		Create(LPCTSTR Filename, IDirect3DDevice9 *pDevice) {}
	inline virtual void		Create(CUSTOMVERTEX vertices[], short numVert, short indices[], short numindices) {}
	inline virtual void		Render(void) {}
	inline virtual void		RenderSM(void) {}
	inline virtual void		Normalize(float scaleTo = 1.0f, bool center = true) {}
	inline virtual			~rbobject(void){}	// default Destructor
	inline virtual void		GetVertexIndexData(void){}

// private: --> THIS SHOULD BE PRIVATE (BETTER OOP), BUT DON'T WANT OVERHEAD
//				OF SELECTOR AND MODIFIER CALLS ON THE STACK
	// Constant Values
	float					mass;				// Object Mass
    // Note: OBBOX lengths for enitre OBB Tree are updated everytime the rboject scale factor is 
	//       updated.  This will take a long time --> Scale is only to be updated once at startup, 
	//       it was never meant to be dynamic.
	float					scale;				// Scaling factor
	D3DXVECTOR3				m_objectBounds[8];	// Bounding Box data in MODEL SPACE
	D3DXMATRIX				scaleMat,			// Scaling translation
							Ibody,				// Body Space Inertia Tensor --> Calculated on runtime
							Ibody_inv;			// Inverse Body Space Inertia Tensor

	// State Variables
	D3DXVECTOR3				x,					// Position
							x_t1,
							P,					// Linear Momentum
							P_t1,
							L,					// Angular Momentum
							L_t1;
	D3DXQUATERNION			q,					// Rotational state
							q_t1;
	// (use D3DXQuaternionRotationMatrix and D3DXMatrixRotationQuaternion to transform: quaternion <--> matrix)

	// Derived Quantities (from state vectors)
	D3DXMATRIX				Iinv,				// Inverse World Space Inertia Tensor
							R,					// Rotation matrix (derived from quaternions)
							R_t,				// Transpose Rotation matrix
							Trans,				// Translation matrix (updated when rendered)
							matWorld;			// MODEL SPACE --> WORLD SPACE TRANSFORM
	D3DXVECTOR3				v,					// Velocity
							omega,				// Angular Velocity
							m_worldBounds[8],	// Bounding Box data in WORLD SPACE (not axis aligned)
							m_minAABB,			// Minimum Bounding Box data in axis-alighed WORLD SPACE
							m_maxAABB;			// Maximum Boudning Box data in axis-alighed WORLD SPACE
	bool					dirtyScaleMatrix,
							dirtyRotMatrix,
							dirtyTransMatrix,
							immovable;
	
	// Computed quantities
	D3DXVECTOR3				force,				// Total force on the object
							torque;				// Total torque on the object

	// Values used to build OBB tree
	DWORD					numFaces;
	DWORD					numVerticies;
	D3DXVECTOR3 *			vertexBuffer;		// IN OBJECT COORDINATES! For OBB calculation
	int *					indexBuffer;		// IN OBJECT COORDINATES!
	obbtree	*				OBBTree;
	
};

// ****************************
// Rigid body object Mesh class
// ****************************
class rboMesh : public rbobject
{
public:
				rboMesh(void);
				~rboMesh(void);

	// The next four based on http://www.dhpoware.com/demos/
	void		Create(LPCTSTR Filename, IDirect3DDevice9 *pDevice);
	void		Render(void);
	void		RenderSM(void);
	void		GetVertexIndexData(void);

	// These are no longer in use
//	void		Bounds(D3DXVECTOR3 *pCenter, float *pRadius);
//	void		Normalize(float scaleTo = 1.0f, bool center = true);
//	void		Scale(float Scale, D3DXVECTOR3 *pOffset = 0);

private:
    ID3DXMesh *						pMesh;
    IDirect3DVertexDeclaration9 *	pDeclaration;
    std::vector<IDirect3DTexture9*> textures;		// Vector array of textures
    std::vector<Mtrl>				materials;		// Vector array of materials
	IDirect3DVertexDeclaration9*	Decl;			// Vertex declaration

	// OLD FUNCTIONS - TO BE UPDATED
	void							ComputeMassProperties(void);

};

// *************************************************
// Rigid body object Indexed primatives class
// NO LONGER IN USE --> SEE OLD CODE IF NEEDED AGAIN
// *************************************************
// class rboIndex : public rbobject { }

#endif