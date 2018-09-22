/*
 * MxModel.h
 *
 *  Created on: Apr 2, 2017
 *      Author: andy
 */

#ifndef SRC_MXMODEL_H_
#define SRC_MXMODEL_H_

#include "mechanica_private.h"
#include "MxModule.h"
#include "MxMesh.h"


struct MxModel;

class LangevinPropagator;

/**
 * The type object for a MxSymbol.
 */
MxAPI_DATA(MxType) *MxModel_Type;

struct MxModelMethods {
    void (*execute) (MxModel* cpu);
};

/**
 * What is the model object, and what should it do?
 *
 * * Model will eventually be a compiled object that is generated by the
 *   compiler when it reads mechanica source code, and spits out a compiled
 *   model.
 *
 * * The model will be loaded by the simulator, the model is a stored in
 *   a shared library, and loaded at runtime.
 *
 * * Model should not create the simulator/propagator/integrator. Rather, the
 *   model should be called by these components to do things like calculate
 *   forces/rates of change, etc...
 *
 * * Model could provide hints as to what propagator to use though.
 *
 * * Initial models will be hard-coded, but we will hard-code to them to test
 *   out and better understand all the functionality required of it.
 *
 * * A model extends a compiled module, model can define functions, classes.
 *   Other modules can load a module just like a module.
 *
 * * An instance of a model is a fully compiled and loaded object, once a model
 *   is instantiated, it will not load any further meshes from disk. The model
 *   loader may optionally separate compiled module code and mesh geometry info,
 *   and compile them together when asked to create a model.
 *
 *
 * Model responsibilities:
 * * Create and initialize a MxMesh, maintain a reference to the mesh.
 *
 * * Calculate the total force acting on every element in the mesh.
 *
 * * Calculate the rate of change of every chemical species in the model,
 *   includes rate of change due to local reactions, and from the
 *   flux between regions contribution.
 *
 *
 *
 * Force Calculations
 *
 * One of the most expensive aspects of most physically based simulations such as
 * n-body (molecular dynamics, astrophysics), finite element, etc. is force calculations.
 *
 * In MD, force calculation, through efficient computationally and numeric implementation
 * is a challenge, is conceptually straightforward. (stuff about bonded and non-bonded
 * interactions) efficient numeric implementation via pipelining, cache coherence,
 * apply mathematical techniques such as operator splitting.
 *
 * The range and type of forces in active visco-elastic material is significantly
 * more rich and complex.
 *
 * Initial version will have three basic types of forces:
 * * Volume preservation -- acts perpendicular to surface
 * * Individual cell area preservation -- acts parallel to surface
 * * Shared contact area -- acts parallel to surface.
 *
 * The contact area forces represent surface and interfacial tension.
 *
 * The force calculations rely on numerous attributes, or 'bulk' quantities
 * of the objects that contain the triangles. It would of course be inefficient
 * to query and recalculate these quantities for each force calculation. Similarly
 * it is not quite architecturally pure for another component such as the propagator
 * to update these quantities, but the propagator is the component that actually
 * moves the vertices of each component. So, we have the propagator inform
 * each higher level component (cells) that it's vertices have moved, hence
 * it is time to re-calculate these bulk properties.
 */
struct MxModel : MxModule {

    /**
     * The model is responsible for creating a mesh. The mesh is shared
     * between different modules, but owned here.
     */
    MxMesh *mesh = nullptr;

    LangevinPropagator *propagator = nullptr;




    /**
     * If pos is null, use the current positions.
     */
    virtual HRESULT getForces(float time, uint32_t len, const Vector3 *pos, Vector3 *force) = 0;

    virtual HRESULT getAccelerations(float time, uint32_t len, const Vector3 *pos, Vector3 *acc) = 0;

    virtual HRESULT getMasses(float time, uint32_t len, float *masses) = 0;

    virtual HRESULT getPositions(float time, uint32_t len, Vector3 *pos) = 0;

    virtual HRESULT setPositions(float time, uint32_t len, const Vector3 *pos) = 0;


    /**
      * The state vector is a vector of elements that are defined by
      * differential equations (rate rules) or independent floating species
      * are defined by reactions.
      *
      * To get the ids of the state vector elements, use getStateVectorId.
      *
      * copies the internal model state vector into the provided
      * buffer.
      *
      * @param[out] stateVector: a buffer to copy the state vector into, if NULL,
      *         return the size required.
      *
      * @param[out] count: the number of items coppied into the provided buffer, if
      *         stateVector is NULL, returns the length of the state vector.
      */
     virtual HRESULT getStateVector(float *stateVector, uint32_t *count) = 0;

     /**
      * sets the internal model state to the provided packed state vector.
      *
      * @param[in] an array which holds the packed state vector, must be
      *         at least the size returned by getStateVector.
      *
      * @return the number of items copied from the state vector, negative
      *         on failure.
      */
     virtual HRESULT setStateVector(const float *stateVector) = 0;

     /**
      * the state vector y is the rate rule values and floating species
      * concentrations concatenated. y is of length numFloatingSpecies + numRateRules.
      *
      * The state vector is packed such that the first n raterule elements are the
      * values of the rate rules, and the last n floatingspecies are the floating
      * species values.
      *
      * @param[in] time current simulator time
      * @param[in] y state vector, must be either null, or have a size of that
      *         speciefied by getStateVector. If y is null, then the model is
      *         evaluated using its current state. If y is not null, then the
      *         y is considered the state vector.
      * @param[out] dydt calculated rate of change of the state vector, if null,
      *         it is ignored.
      */
     virtual HRESULT getStateVectorRate(float time, const float *y, float* dydt=0) = 0;

    virtual ~MxModel() {};


};




void MxModel_init(PyObject *m);

#endif /* SRC_MXMODEL_H_ */
