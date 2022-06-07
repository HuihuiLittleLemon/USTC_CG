#pragma once

#include <Basic/HeapObj.h>
#include <UGM/UGM>
#include <UHEMesh/HEMesh.h>

namespace Ubpa {
	class TriMesh;
	class MinSurf;

	// mesh boundary == 1
	class Paramaterize : public HeapObj {
	public:
		Paramaterize(Ptr<TriMesh> triMesh);
	public:
		static const Ptr<Paramaterize> New(Ptr<TriMesh> triMesh) {
			return Ubpa::New<Paramaterize>(triMesh);
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
		float distance(vecf3 begin, vecf3 end);
		bool map_trimesh_boundary_vertices_to_unit_circle();
		bool Run(bool is_uniform_weight);

	private:
		Ptr<TriMesh> triMesh;
		const Ptr<HEMesh<V>> heMesh; // vertice order is same with triMesh
		std::unordered_map<size_t, vecf3> boundary_vertices_map_on_circle_pos_;
	};
}
