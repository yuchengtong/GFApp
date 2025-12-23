//用于生成netgen网格
#pragma once
#include <iostream>
#include <climits>

#include "TopTools_IndexedMapOfShape.hxx"
#include "TopoDS.hxx"
#include "TopoDS_Face.hxx"
#include "TopoDS_Shape.hxx"
#include "GProp_GProps.hxx"
#include "BRepGProp.hxx"
#include "BRepPrimAPI_MakeCone.hxx"
#include "BRepPrimAPI_MakeTorus.hxx"
#include "BRepPrimAPI_MakeWedge.hxx"
using namespace std;

namespace nglib {
#include <nglib.h>
}

int main()
{
	using namespace nglib;

	cout << "Netgen NgLib - OpenCascade Test Case" << endl;



	// Define pointer to OCC Geometry
	Ng_OCC_Geometry *occ_geom;

	Ng_Mesh *occ_mesh;

	Ng_Meshing_Parameters mp;

	TopTools_IndexedMapOfShape FMap;

	Ng_OCC_TopTools_IndexedMapOfShape *occ_fmap = (Ng_OCC_TopTools_IndexedMapOfShape*)&FMap;

	// Result of Netgen Operations
	Ng_Result ng_res;

	// Initialise the Netgen Core library
	Ng_Init();
	//谢新明
	TopoDS_Shape C1 = BRepPrimAPI_MakeCone(50., 25., 200.);;
	occ_geom = Ng_OCC_Load_Shape(C1);
	// Read in the OCC File
	//const char* filename = ("d:\\stp\\cone.step");
	//occ_geom = Ng_OCC_Load_STEP(filename);
	if (!occ_geom)
	{
		//cout << "Error reading in STEP File: " << filename << endl;
		system("pause");
		return 1;
	}
	//cout << "Successfully loaded STEP File: " << filename << endl;

	occ_mesh = Ng_NewMesh();

	ng_res = Ng_OCC_GetFMap(occ_geom, occ_fmap);

	cout << "ng_res = " << ng_res << endl;

	if (!FMap.Extent())
	{
		cout << "Error retrieving Face map...." << endl;
		system("pause");
		return 1;
	}

	cout << "Successfully extracted the Face Map....:" << FMap.Extent() << endl;

	for (int i = 1; i <= FMap.Extent(); i++)
	{
		TopoDS_Face OCCface;
		OCCface = TopoDS::Face(FMap.FindKey(i));

		GProp_GProps faceProps;
		BRepGProp::SurfaceProperties(OCCface, faceProps);

		cout << "Index: " << i
			<< " :: Area: " << faceProps.Mass()
			<< " :: Hash: " << OCCface.HashCode(1e+6)
			<< endl;
	}

	mp.uselocalh = 1;
	mp.elementsperedge = 2.0;
	mp.elementspercurve = 2.0;
	mp.maxh = 10.0;
	mp.grading = 0.2;
	mp.closeedgeenable = 0;
	mp.closeedgefact = 1.0;
	mp.optsurfmeshenable = 1;


	cout << "Setting Local Mesh size....." << endl;
	cout << "OCC Mesh Pointer before call = " << occ_mesh << endl;
	Ng_OCC_SetLocalMeshSize(occ_geom, occ_mesh, &mp);
	cout << "Local Mesh size successfully set....." << endl;
	cout << "OCC Mesh Pointer after call = " << occ_mesh << endl;

	cout << "Creating Edge Mesh....." << endl;
	ng_res = Ng_OCC_GenerateEdgeMesh(occ_geom, occ_mesh, &mp);
	if (ng_res != NG_OK)
	{
		Ng_DeleteMesh(occ_mesh);
		cout << "Error creating Edge Mesh.... Aborting!!" << endl;
		system("pause");
		return 1;
	}
	else
	{
		cout << "Edge Mesh successfully created....." << endl;
		cout << "Number of points = " << Ng_GetNP(occ_mesh) << endl;
	}

	cout << "Creating Surface Mesh....." << endl;

	ng_res = Ng_OCC_GenerateSurfaceMesh(occ_geom, occ_mesh, &mp);
	if (ng_res != NG_OK)
	{
		Ng_DeleteMesh(occ_mesh);
		cout << "Error creating Surface Mesh..... Aborting!!" << endl;
		system("pause");
		return 1;
	}
	else
	{
		cout << "Surface Mesh successfully created....." << endl;
		cout << "Number of points = " << Ng_GetNP(occ_mesh) << endl;
		cout << "Number of surface elements = " << Ng_GetNSE(occ_mesh) << endl;
	}
	//细化网格
	Ng_OCC_Uniform_Refinement(occ_geom, occ_mesh);
	//获取面网格数据  谢新明
	int np = 0;
	double point[3] = { 0.0 };
	np = nglib::Ng_GetNP(occ_mesh);
	for (int i = 1; i <= np; ++i)
	{
		nglib::Ng_GetPoint(occ_mesh, i, point);
		std::cout << i << ": " << point[0] << ", " << point[1] << ", " << point[2] << std::endl;
	}
	int ne = 0;
	ne = nglib::Ng_GetNSE(occ_mesh);
	int p[3] = { 0.0 };
	for (int i = 0; i < ne; i++)
	{
		nglib::Ng_GetSurfaceElement(occ_mesh, i, p);
		std::cout << i << ": " << p[0] << ", " << p[1] << ", " << p[2] << std::endl;
	}
	//
	cout << "Creating Volume Mesh....." << endl;

	ng_res = Ng_GenerateVolumeMesh(occ_mesh, &mp);

	cout << "Volume Mesh successfully created....." << endl;
	cout << "Number of points = " << Ng_GetNP(occ_mesh) << endl;
	cout << "Number of volume elements = " << Ng_GetNE(occ_mesh) << endl;

	cout << "Saving Mesh as VOL file....." << endl;
	Ng_SaveMesh(occ_mesh, "test_occ8.vol");
	system("pause");
	return 0;
}


