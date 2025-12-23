#pragma execution_character_set("utf-8")
#include "mainWidget.h"
#include <QtWidgets/QApplication>
#include "ModelDataManager.h"
#include "LoginDialog.h"


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

#ifndef LIBS_H
#define LIBS_H

#endif // LIBS_H


#ifdef _DEBUG
#pragma comment(lib,"TKBin.lib")
#pragma comment(lib,"TKBinL.lib")
#pragma comment(lib,"TKBinTObj.lib")
#pragma comment(lib,"TKBinXCAF.lib")
#pragma comment(lib,"TKBO.lib")
#pragma comment(lib,"TKBool.lib")
#pragma comment(lib,"TKBRep.lib")
#pragma comment(lib,"TKCAF.lib")
#pragma comment(lib,"TKCDF.lib")
#pragma comment(lib,"TKD3DHost.lib")
#pragma comment(lib,"TKDCAF.lib")
#pragma comment(lib,"TKDFBrowser.lib")
#pragma comment(lib,"TKDraw.lib")
#pragma comment(lib,"TKernel.lib")
#pragma comment(lib,"TKFeat.lib")
#pragma comment(lib,"TKFillet.lib")
#pragma comment(lib,"TKG2d.lib")
#pragma comment(lib,"TKG3d.lib")
#pragma comment(lib,"TKGeomAlgo.lib")
#pragma comment(lib,"TKGeomBase.lib")
#pragma comment(lib,"TKHLR.lib")
#pragma comment(lib,"TKIGES.lib")
#pragma comment(lib,"TKIVtk.lib")
#pragma comment(lib,"TKIVtkDraw.lib")
#pragma comment(lib,"TKLCAF.lib")
#pragma comment(lib,"TKMath.lib")
#pragma comment(lib,"TKMesh.lib")
#pragma comment(lib,"TKMeshVS.lib")
#pragma comment(lib,"TKOffset.lib")
#pragma comment(lib,"TKOpenGl.lib")
#pragma comment(lib,"TKPrim.lib")
#pragma comment(lib,"TKQADraw.lib")
#pragma comment(lib,"TKRWMesh.lib")
#pragma comment(lib,"TKService.lib")
#pragma comment(lib,"TKShapeView.lib")
#pragma comment(lib,"TKShHealing.lib")
#pragma comment(lib,"TKStd.lib")
#pragma comment(lib,"TKStdL.lib")
#pragma comment(lib,"TKSTEP.lib")
#pragma comment(lib,"TKSTEP209.lib")
#pragma comment(lib,"TKSTEPAttr.lib")
#pragma comment(lib,"TKSTEPBase.lib")
#pragma comment(lib,"TKSTL.lib")
#pragma comment(lib,"TKTInspector.lib")
#pragma comment(lib,"TKTInspectorAPI.lib")
#pragma comment(lib,"TKTObj.lib")
#pragma comment(lib,"TKTObjDRAW.lib")
#pragma comment(lib,"TKToolsDraw.lib")
#pragma comment(lib,"TKTopAlgo.lib")
#pragma comment(lib,"TKTopTest.lib")
#pragma comment(lib,"TKTreeModel.lib")
#pragma comment(lib,"TKV3d.lib")
#pragma comment(lib,"TKVCAF.lib")
#pragma comment(lib,"TKView.lib")
#pragma comment(lib,"TKViewerTest.lib")
#pragma comment(lib,"TKVInspector.lib")
#pragma comment(lib,"TKVRML.lib")
#pragma comment(lib,"TKXCAF.lib")
#pragma comment(lib,"TKXDEDRAW.lib")
#pragma comment(lib,"TKXDEIGES.lib")
#pragma comment(lib,"TKXDESTEP.lib")
#pragma comment(lib,"TKXMesh.lib")
#pragma comment(lib,"TKXml.lib")
#pragma comment(lib,"TKXmlL.lib")
#pragma comment(lib,"TKXmlTObj.lib")
#pragma comment(lib,"TKXmlXCAF.lib")
#pragma comment(lib,"TKXSBase.lib")
#pragma comment(lib,"TKXSDRAW.lib")
#else
//
#pragma comment(lib,"TKBin.lib")
#pragma comment(lib,"TKBinL.lib")
#pragma comment(lib,"TKBinTObj.lib")
#pragma comment(lib,"TKBinXCAF.lib")
#pragma comment(lib,"TKBO.lib")
#pragma comment(lib,"TKBool.lib")
#pragma comment(lib,"TKBRep.lib")
#pragma comment(lib,"TKCAF.lib")
#pragma comment(lib,"TKCDF.lib")
#pragma comment(lib,"TKD3DHost.lib")
#pragma comment(lib,"TKDCAF.lib")
#pragma comment(lib,"TKDFBrowser.lib")
#pragma comment(lib,"TKDraw.lib")
#pragma comment(lib,"TKernel.lib")
#pragma comment(lib,"TKFeat.lib")
#pragma comment(lib,"TKFillet.lib")
#pragma comment(lib,"TKG2d.lib")
#pragma comment(lib,"TKG3d.lib")
#pragma comment(lib,"TKGeomAlgo.lib")
#pragma comment(lib,"TKGeomBase.lib")
#pragma comment(lib,"TKHLR.lib")
#pragma comment(lib,"TKIGES.lib")
#pragma comment(lib,"TKIVtk.lib")
#pragma comment(lib,"TKIVtkDraw.lib")
#pragma comment(lib,"TKLCAF.lib")
#pragma comment(lib,"TKMath.lib")
#pragma comment(lib,"TKMesh.lib")
#pragma comment(lib,"TKMeshVS.lib")
#pragma comment(lib,"TKOffset.lib")
#pragma comment(lib,"TKOpenGl.lib")
#pragma comment(lib,"TKPrim.lib")
#pragma comment(lib,"TKQADraw.lib")
#pragma comment(lib,"TKRWMesh.lib")
#pragma comment(lib,"TKService.lib")
#pragma comment(lib,"TKShapeView.lib")
#pragma comment(lib,"TKShHealing.lib")
#pragma comment(lib,"TKStd.lib")
#pragma comment(lib,"TKStdL.lib")
#pragma comment(lib,"TKSTEP.lib")
#pragma comment(lib,"TKSTEP209.lib")
#pragma comment(lib,"TKSTEPAttr.lib")
#pragma comment(lib,"TKSTEPBase.lib")
#pragma comment(lib,"TKSTL.lib")
#pragma comment(lib,"TKTInspector.lib")
#pragma comment(lib,"TKTInspectorAPI.lib")
#pragma comment(lib,"TKTObj.lib")
#pragma comment(lib,"TKTObjDRAW.lib")
#pragma comment(lib,"TKToolsDraw.lib")
#pragma comment(lib,"TKTopAlgo.lib")
#pragma comment(lib,"TKTopTest.lib")
#pragma comment(lib,"TKTreeModel.lib")
#pragma comment(lib,"TKV3d.lib")
#pragma comment(lib,"TKVCAF.lib")
#pragma comment(lib,"TKView.lib")
#pragma comment(lib,"TKViewerTest.lib")
#pragma comment(lib,"TKVInspector.lib")
#pragma comment(lib,"TKVRML.lib")
#pragma comment(lib,"TKXCAF.lib")
#pragma comment(lib,"TKXDEDRAW.lib")
#pragma comment(lib,"TKXDEIGES.lib")
#pragma comment(lib,"TKXDESTEP.lib")
#pragma comment(lib,"TKXMesh.lib")
#pragma comment(lib,"TKXml.lib")
#pragma comment(lib,"TKXmlL.lib")
#pragma comment(lib,"TKXmlTObj.lib")
#pragma comment(lib,"TKXmlXCAF.lib")
#pragma comment(lib,"TKXSBase.lib")
#pragma comment(lib,"TKXSDRAW.lib")
//
//#pragma comment(lib,"osgTree.lib")
//#pragma comment(lib,"glfw3.lib")
#endif



int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	// 设置全局字体大小
	QFont font = QFont("Arial", 9);
	QApplication::setFont(font);
	mainWidget w;
	QCoreApplication::setApplicationName(QStringLiteral("固体发动机安全性分析与评估系统"));
	/*w.hide();
	LoginDialog loginDialog;
	if (loginDialog.exec() == QDialog::Accepted) {
		w.show();
		return a.exec();
	}*/
	w.show();
	return a.exec();



}
