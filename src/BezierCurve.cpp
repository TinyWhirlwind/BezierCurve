#include "BezierCurve.h"
#include <Windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <map>

#define SUBDIV 5//控制曲线细分参数,中间插值顶点数目，不包括两端顶点

BezierCurve::BezierCurve()
{
	m_hitNode = -1;
	m_startNode = 0;
	b_depthTest = true;
	m_interpolationNumber = SUBDIV;
}
BezierCurve::~BezierCurve()
{
	Clear();
}

bool  BezierCurve::IsClosed()
{
	if (m_startNode == 0)
		return false;
	if (m_startNode->prev == 0)
		return false;
	return true;
}
void  BezierCurve::CloseCurve()
{
	int cnt = 0;
	BNode* last = m_startNode;

	while (last->next != 0) // 查找最后一个节点
	{
		last = last->next;
		cnt++;
	}

	if (cnt > 5)
	{
		m_startNode->prev = last;
		last->next = m_startNode;
	}

}
BNode* BezierCurve::GetStartNode()
{
	return m_startNode;
}
void  BezierCurve::Clear()
{
	if (m_startNode == 0)
		return;
	m_BezierCurve.clear();
	for (InterpolationNode* ptr : m_InterpolationPointList) {
		delete ptr;
	}
	m_InterpolationPointList.clear();
	std::vector<BNode*> nodeSet;
	BNode* cur = m_startNode;
	do
	{
		nodeSet.push_back(cur);
		cur = cur->next;
	} while (cur != 0 && cur != m_startNode);

	int siz = nodeSet.size();
	for (int i = 0; i < siz; i++)
	{
		delete  nodeSet[i];
	}

	m_startNode = 0;

}
void BezierCurve::InsertNode(Point3f p, int nearbyToothId)
{
	BNode* node = new BNode;

	node->pos = p;
	if (m_startNode == 0)
	{
		m_startNode = node;
		m_startNode->prev = 0;
		m_startNode->next = 0;
	}
	else
	{
		BNode* last = m_startNode;
		while (last->next != 0)
			last = last->next;
		last->next = node;
		node->prev = last;
		node->next = 0;
	}
}

void BezierCurve::SetInterpolationNumber(int num)
{
	m_interpolationNumber = num;
}

int BezierCurve::GetInterpolationNumber()
{
	return m_interpolationNumber;
}

void  BezierCurve::UpdateCurve()
{
	Point3f p[4];//三次曲线四个控制顶点
	Point3f dir0, dir3;//切线方向
	Point3f dir;//中心线方向
	float diffX, diffY, diffZ, len;
	float projLen;//投影长度
	Point3f point;

	float interv = (float)1 / (float)m_interpolationNumber;//曲线密度
	float t;//控制曲线参数
	std::vector<float> coordX;//曲线系数
	std::vector<float> coordY;
	std::vector<float> coordZ;
	int n = 3;//三次曲线
	float result;//中间数值

	std::vector<Point3f> m_ControlPoint;
	std::map<Point3f, int> toothList;
	BNode* cur = m_startNode;

	do {
		Point3f p = cur->pos;
		m_ControlPoint.push_back(p);
		cur = cur->next;
	} while (cur != 0 && cur != m_startNode);

	if (m_ControlPoint.size() == 1)
		return;
	//===============================================
	int end = (int)m_ControlPoint.size();

	if ((int)m_BezierCurve.size() > 0)m_BezierCurve.clear();
	if ((int)m_InterpolationPointList.size() > 0)
	{
		for (InterpolationNode* ptr : m_InterpolationPointList) {
			delete ptr;
		}
		m_InterpolationPointList.clear();
	}

	int mm = 0;
	for (int i = 0; i < end; i++)//计算参数曲线段
	{
		//计算切线方向
		dir0 = m_ControlPoint[(i + 1) % end] - m_ControlPoint[(i - 1 + end) % end];

		//单位化切线方向
		diffX = dir0.X() * dir0.X();
		diffY = dir0.Y() * dir0.Y();
		diffZ = dir0.Z() * dir0.Z();
		len = sqrt(diffX + diffY + diffZ);
		dir0.X() /= len; dir0.Y() /= len; dir0.Z() /= len;

		//计算切线方向
		dir3 = m_ControlPoint[i] - m_ControlPoint[(i + 2) % end];

		//单位化切线方向
		diffX = dir3.X() * dir3.X();
		diffY = dir3.Y() * dir3.Y();
		len = sqrt(diffX + diffY + diffZ);
		dir3.X() /= len; dir3.Y() /= len; dir3.Z() /= len;

		////////////  计算控制顶点
		if (i != end - 1)
		{
			p[0] = m_ControlPoint[i];
			p[3] = m_ControlPoint[i + 1];
		}
		else
		{
			p[0] = m_ControlPoint[end - 1];
			p[3] = m_ControlPoint[0];
		}
		dir = p[3] - p[0];
		diffX = dir.X() * dir.X();
		diffY = dir.Y() * dir.Y();
		diffZ = dir.Z() * dir.Z();
		len = sqrt(diffX + diffY + diffZ);
		dir.X() /= len; dir.Y() /= len; dir.Z() /= len;

		//if (dir*dir0 < 0.1|| dir*dir3>-0.9) // 防止变化过大
		//{
		//  projLen = len / 4;
		//  dir0 = dir;
		//  dir3 = -dir;
		//}
		//else
		projLen = len / (4 * (dir * dir0));//计算投影长度

		p[1].X() = p[0].X() + dir0.X() * projLen;
		p[1].Y() = p[0].Y() + dir0.Y() * projLen;
		p[1].Z() = p[0].Z() + dir0.Z() * projLen;
		projLen = -len / (4 * (dir * dir3));

		p[2].X() = p[3].X() + dir3.X() * projLen;
		p[2].Y() = p[3].Y() + dir3.Y() * projLen;
		p[2].Z() = p[3].Z() + dir3.Z() * projLen;

		//////////////  计算控制曲线

		//计算系数
		coordX.resize(4);//三次曲线四个系数
		coordY.resize(4);
		coordZ.resize(4);
		n = 3;

		for (int j = 0; j < 4; j++)
		{
			result = 1.0;
			for (int k = n; k >= 1; k--)result *= (float)k;
			for (int k = j; k >= 1; k--)result /= (float)k;
			for (int k = n - j; k >= 1; k--)result /= (float)k;
			coordX[j] = result * p[j].X();
			coordY[j] = result * p[j].Y();
			coordZ[j] = result * p[j].Z();
		}

		//计算插值点
		for (int j = 0; j <= m_interpolationNumber; j++)
		{
			t = (float)j * interv;
			point.X() = 0.0;
			point.Y() = 0.0;
			point.Z() = 0.0;
			for (int k = 0; k < 4; k++)
			{
				result = 1.0;
				for (int l = 0; l < k; l++)  result *= t;
				for (int l = 0; l < n - k; l++)result *= (1 - t);

				point.X() += result * coordX[k];
				point.Y() += result * coordY[k];
				point.Z() += result * coordZ[k];
			}

			if (std::isnan(point.X()) || std::isnan(point.Y()) || std::isnan(point.Y()))
			{
				continue;
			}

			if (i < end - 1)
			{
				if (j == m_interpolationNumber)
					break;
				m_BezierCurve.push_back(point);//存入插值点
				InterpolationNode* iNode = new InterpolationNode;
				iNode->pos = point;
				m_InterpolationPointList.push_back(iNode);
			}
			else
			{
				//样条闭合
				if (m_startNode != 0 && m_startNode->prev != 0)
				{
					m_BezierCurve.push_back(point);//存入插值点
					InterpolationNode* iNode = new InterpolationNode;
					iNode->pos = point;
					m_InterpolationPointList.push_back(iNode);
				}
				else
				{
					if (j == 0)
					{
						m_BezierCurve.push_back(point);//存入插值点
						InterpolationNode* iNode = new InterpolationNode;
						iNode->pos = point;
						m_InterpolationPointList.push_back(iNode);
					}
				}
			}
		}
	}
	m_CurveDepth.resize(m_BezierCurve.size());
}