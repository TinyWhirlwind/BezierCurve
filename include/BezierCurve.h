#ifndef _BezierCurve_H_
#define _BezierCurve_H_
#include <vcg/space/triangle3.h>
using namespace vcg;
#pragma execution_character_set("utf-8")

struct InterpolationNode // 样条曲线插值节点
{
	InterpolationNode() {
		isS = false; isV = false; 
	}
	bool isV;
	bool isS;
	Point3f pos;
	Point3f attachPos;//附件位置
	void SetV() { isV = true; }
	void SetS() { isS = true; }

};
struct BNode // 样条曲线节点
{
	BNode() {
		m_flag = 0; prev = next = 0; curveIndex = -1; hit = false; isEnd = false; isV = false; 
	}

	BNode* prev;
	BNode* next;
	int curveIndex;
	int m_flag;
	Point3f pos;
	bool hit;
	bool isEnd;
	bool isV;
	void SetV() { isV = true; }
};

class BezierCurve
{
public:
	void  Clear();
	void  InsertNode(Point3f p, int nearbyToothId);
	void  CloseCurve(); // 封闭曲线
	void  UpdateCurve(); // 根据控制顶点，更新曲线
	void  SetInterpolationNumber(int num);//控制曲线细分参数,中间插值顶点数目，不包括两端顶点 不设置默认为1
	int  GetInterpolationNumber();
	bool  IsClosed(); // 是否封闭

	BNode* GetStartNode();

	BezierCurve();
	~BezierCurve();

	BNode* m_startNode;
	std::vector<Point3f> m_BezierCurve;//控制曲线
	std::vector<InterpolationNode*> m_InterpolationPointList;//插值点集
	std::vector<bool> m_CurveDepth;//深度信息

	int m_interpolationNumber;
	int	m_hitNode;
	bool b_depthTest;

};


#endif
