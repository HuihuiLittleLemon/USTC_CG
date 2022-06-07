#pragma once

#include <Basic/HeapObj.h>
#include <UHEMesh/HEMesh.h>
#include <UGM/UGM>

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/SparseCholesky>

using namespace Eigen;

namespace Ubpa {
	class TriMesh;
	class Paramaterize;

	class MinSurf : public HeapObj {
	public:
		MinSurf(Ptr<TriMesh> triMesh);
	public:
		static const Ptr<MinSurf> New(Ptr<TriMesh> triMesh) {
			return Ubpa::New<MinSurf>(triMesh);
		}
	public:
		// clear cache data
		void Clear();

		// init cache data (eg. half-edge structure) for Run()
		bool Init(Ptr<TriMesh> triMesh);

		// call it after Init()
		bool Run(bool is_uniform_weight);

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
	private:

		// kernel part of the algorithm
		ComputationInfo  sparse_matrix_preprocess();
		void solve_sparse_matrix();
		void solve_sparse_matrix(std::unordered_map<size_t, vecf3> boundary_vertices_map_on_circle);
		void Minimize(bool is_uniform_weight);
		void Minimize(bool is_uniform_weight, std::unordered_map<size_t, vecf3> boundary_vertices_map_on_circle);
		float vecf_dot(vecf3 first, vecf3 second);
		vecf3 vecf_cross(vecf3 first, vecf3 second);
		float calc_wij(V* centre, V* adj);
	private:
		friend class Paramaterize;

		SimplicialLLT<SparseMatrix<float>> solver_sparse_;
		SparseMatrix<float> A_saprse_;
		Ptr<TriMesh> triMesh;
		const Ptr<HEMesh<V>> heMesh; // vertice order is same with triMesh
		bool is_uniform_weight_;
		int inner_vertices_num_;
		std::unordered_map<size_t, V*> boundary_vertices_;
		std::unordered_map<size_t, int> inner_vertices_index_map_matrix_colum_;
	};
}
