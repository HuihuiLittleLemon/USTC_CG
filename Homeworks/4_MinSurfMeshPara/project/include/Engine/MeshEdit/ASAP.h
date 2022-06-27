#pragma once
#include <Basic/HeapObj.h>
#include <UGM/UGM>
#include <UHEMesh/HEMesh.h>

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/SparseCholesky>

using namespace Eigen;

namespace Ubpa {
	class TriMesh;
	class MinSurf;

	// mesh boundary == 1
	class ASAP : public HeapObj {
	public:
		ASAP(Ptr<TriMesh> triMesh);
	public:
		static const Ptr<ASAP> New(Ptr<TriMesh> triMesh) {
			return Ubpa::New<ASAP>(triMesh);
		}
	private:
		class V;
		class E;
		class P;
		class V : public TVertex<V, E, P> {
		public:
			vecf3 pos;
		};
		class E : public TEdge<V, E, P> { };
		class P :public TPolygon<V, E, P> { };

	public:
		void Clear();
		bool Init(Ptr<TriMesh> triMesh);
		bool Run(bool is_uniform_weight);
		void generator_ref_triangle();
		void calc_all_cot_theta();
		ComputationInfo  sparse_matrix_preprocess();
	private:
		friend class ARAP;
		Ptr<TriMesh> triMesh;
		const Ptr<HEMesh<V>> heMesh; // vertice order is same with triMesh
		std::unordered_map<size_t, std::vector<pointf2>> ref_triangles_;
		std::unordered_map<size_t,std::vector<float>> cot_theta_;
		SimplicialLLT<SparseMatrix<float>> solver_sparse_;
		SparseMatrix<float> A_saprse_;
	};
}
