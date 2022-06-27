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
	class ARAP : public HeapObj {
	public:
		ARAP(Ptr<TriMesh> triMesh);
	public:
		static const Ptr<ARAP> New(Ptr<TriMesh> triMesh) {
			return Ubpa::New<ARAP>(triMesh);
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
		void local_phase();
		void global_phase();
		void local_global_algorithm();
		ComputationInfo  sparse_matrix_preprocess();
	private:
		Ptr<TriMesh> triMesh;
		const Ptr<HEMesh<V>> heMesh; // vertice order is same with triMesh
		std::unordered_map<size_t, std::vector<pointf2>> ref_triangles_;
		std::unordered_map<size_t, std::vector<float>> cot_theta_;
		SimplicialLLT<SparseMatrix<float>> solver_sparse_;
		SparseMatrix<float> A_saprse_;
		std::vector<std::vector<float>> St_;
		int fixed_index_[1] = { 100 };//{ 3161,2898 };
		std::unordered_map<size_t, size_t> vertices_map_row_num_;
	};
}
#pragma once
