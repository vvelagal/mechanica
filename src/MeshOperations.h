/*
 * MeshOperations.h
 *
 *  Created on: Nov 20, 2017
 *      Author: andy
 */

#ifndef SRC_MESHOPERATIONS_H_
#define SRC_MESHOPERATIONS_H_

#include "MxMeshCore.h"
#include <random>

/**
 * An operation modifies a mesh, typically topologically.
 *
 * Operations get constructed with their arguments, then they
 * get re-ordered in the queue based on their energy cost. The values
 * of their arguments change as other operations modify the mesh.
 * Operations must be removed from the queue if their arguments
 * get removed from the queue.
 *
 * Basic responsibilities:
 *
 * * factory operation, given a triangle or edge, create a new operation.
 */
struct MeshOperation {

    MeshOperation(MeshPtr _mesh);

    /**
     * Apply this operation
     */
    virtual HRESULT apply() = 0;

    /**
     * lower, more negative energy operations are queued at a higher priority.
     */
    virtual float energy() const = 0;

    /**
     * does this operation depend on this triangle?
     */
    virtual bool depends(CPolygonPtr) const = 0;

    /**
     * does this operation depend on this vertex?
     */
    virtual bool depends(CVertexPtr) const = 0;


    virtual bool equals(CVertexPtr) const = 0;

    /**
     * mark the edge, for debug purposes
     */
    virtual void mark() const = 0;


    bool  operator<(const MeshOperation& other) const {
        return energy() < other.energy();
    }

    virtual ~MeshOperation() {};



    enum Codes : HRESULT {
        /**
         * The operation failed, but it should be retried the next time,
         * and not removed from the queue.
         */
        RETRY = MAKE_HRESULT(1, FACULTY_MESHOPERATION, 1)
    };


protected:
    MeshPtr mesh;
    struct MeshOperations *ops;
};

struct MeshOperationFactory {

};



/**
 * A priority queue of mesh operations.
 */
struct MeshOperations {



    MeshOperations(MeshPtr mesh, float shortEdgeCutoff, float longEdgeCutoff);

    /**
     * Inform the MeshOperations that a mesh object (likely positions) was changed,
     * Check if the mesh object triggers any operations and enqueue any triggered
     * operations. A mesh object can trigger an operation if an edge is too short
     * or long.
     */
    HRESULT positionsChanged(TriangleContainer::const_iterator begin,
            TriangleContainer::const_iterator end);

    /**
     * Inform the MeshOperations that a mesh object (likely positions) was changed,
     * Check if the mesh object triggers any operations and enqueue any triggered
     * operations. A mesh object can trigger an operation if an edge is too short
     * or long.
     */
    HRESULT valenceChanged(const VertexPtr vert);

    /**
     * a mesh object was deleted, remove any enqueued operations
     * that refer to this obj.
     */
    HRESULT removeDependentOperations(const PolygonPtr tri);

    /**
     * a mesh object was deleted, remove any enqueued operations
     * that refer to this obj.
     */
    HRESULT removeDependentOperations(const VertexPtr vert);

    bool empty() const { return c.empty(); }

    std::size_t size() const { return c.size(); }

    /**
     * Apply all of the queued operations. The queue is empty on return.
     */
    HRESULT apply();

    float getLongCutoff() const { return longCutoff;};

    float getShortCutoff() const { return shortCutoff; };

    void setShortCutoff(float);

    void setLongCutoff(float);

    void setDebugMode(bool);

    HRESULT debugStep();




#ifndef NDEBUG

    bool shouldStop = false;
#endif

    bool debugMode{false};

    int debugCnt = 0;

    MeshOperation *pendingDebugOp{nullptr};


    ~MeshOperations();

private:

    MeshPtr mesh;

    // TODO: this would be a lot more efficient if we stack allocated
    // the ops. Can't do yet, that because of different size for each op,
    // need to fix in future versions.
    typedef std::vector<MeshOperation*> Container;

    Container c;

    void push(MeshOperation* x);


    MeshOperation* pop();

    float shortCutoff;
    float longCutoff;

    Container::iterator findDependentOperation(Container::iterator start, const PolygonPtr);

    Container::iterator findDependentOperation(Container::iterator start, const VertexPtr);

    MeshOperation *findMatchingOperation(CVertexPtr vertex) ;

    MeshOperation *currentOp = nullptr;

};

void setMeshOpDebugMode(uint c);

#endif /* SRC_MESHOPERATIONS_H_ */
