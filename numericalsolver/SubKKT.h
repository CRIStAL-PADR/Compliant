#ifndef COMPLIANT_SUBKKT_H
#define COMPLIANT_SUBKKT_H

#include "../initCompliant.h"
#include "../assembly/AssembledSystem.h"
#include "../utils/eigen_types.h"

namespace sofa {
namespace component {
namespace linearsolver {

class Response;


/**

   A factorization of a sub-system in an AssembledSystem.

   For now, it provides:
        - a way to factor the sub-kkt corresponding
            to non-zero lines/columns in the projection matrix P.
        - a way to assemble the complete KKT (dynamics+bilateral constraints)
            in a large matrix (with removed zero lines/columns in
            the projection matrix P)

   @author Maxime Tournier

 */

class SOFA_Compliant_API SubKKT : public utils::eigen_types {
public:

    // TODO: determine correct access rights

    // primal/dual selection matrices
    rmat P, Q;

    // filtered subsystem
    rmat A;

private:
    // work vectors during solve
    mutable vec vtmp1, vtmp2;

    mutable cmat mtmpc1, mtmpc2;
    mutable rmat mtmpr;

public:

    SubKKT();

    // named constructors

    // standard projected (1, 1) schur subsystem (Q is
    // empty). size_full = sys.m, size_sub = #(non-empty P elements)
    static void projected_primal(SubKKT& res, const AssembledSystem& sys);

    // project constraint substem by exluding non-bilateral constraints
    // returns true iff there are no bilateral constraints
    static bool projected_dual(SubKKT& res, const AssembledSystem& sys);

    // full kkt with projected primal variables
    // eps is for a Tikhonov regularization on null Compliance diagonal entries
    // only_lower to build only the low triangular matrix for symmetric problems
    static void projected_kkt(SubKKT& res,
                              const AssembledSystem& sys,
                              real eps = 0,
                              bool only_lower = false);


    // full kkt with projected primal variables
    // excludes non bilateral constaints
    // TODO upgrade it to any constraint mask and/or move it in a sub-class
    static void projected_kkt_bilateral(SubKKT& res,
                              const AssembledSystem& sys,
                              real eps = 0,
                              bool only_lower = false);

    
    // TODO more ctors with non-zero Q


    // P.rows() + Q.rows()
    unsigned size_full() const;

    // P.cols() + Q.cols()
    unsigned size_sub() const;

    // factor the sub-kkt using
    template<class Solver>
    void factor(Solver& response) const;

    // WARNING the API might change a bit here 

    // solve for rhs vec/mat. rhs must be of size size_full(), result
    // will be resized as needed (full size).
    void solve(const Response& response, cmat& result, const cmat& rhs) const;
    template<class Solver>
    void solve(const Solver& response, vec& result, const vec& rhs ) const;


    void prod(vec& result, const vec& rhs) const;

    // this one transposes rhs before solving (avoids temporary)
    void solve_opt(const Response& response, cmat& result, const rmat& rhs ) const;


    // adaptor to response API for solving
    class Adaptor {
        Response& resp;
        const SubKKT& sub;
    public:

        Adaptor(Response& resp, const SubKKT& sub): resp(resp), sub(sub) { }

        void solve(vec& res, const vec& rhs) const { sub.solve(resp, res, rhs); }
        void solve(cmat& res, const cmat& rhs) const { sub.solve(resp, res, rhs); }
        
    };

    Adaptor adapt(Response& resp) const {
        return Adaptor(resp, *this);
    }
    
};



}
}
}



#endif
