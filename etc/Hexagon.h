#pragma once

#include <cmath>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include "../../Lib_Base/Inc/LogerDef.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
const int MAX_DIRECTION = 6;
struct HEX
{
	int q;
	int r;
	int s;
	HEX() :q(0), r(0), s(0) {}
	HEX(int q_, int r_, int s_) :q(q_), r(r_), s(s_)
	{
		if (q + r + s != 0) throw "q + r + s must be 0";
	}

	HEX(int q_, int r_) :q(q_), r(r_)
	{
		s = -q - r;
	}

	int Length()
	{
		return int((abs(q) + abs(r) + abs(s)) / 2);
	}

	void Normalize();


	HEX operator+(const HEX& b) const
	{
		return HEX(q + b.q, r + b.r, s + b.s);
	}
	HEX operator-(const HEX& b) const
	{
		return HEX(q - b.q, r - b.r, s - b.s);
	}
	HEX operator*(int k)
	{
		return HEX(q * k, r * k, s * k);
	}

	const bool operator==(const HEX& other)  const
	{
		return q == other.q && r == other.r && s == other.s;
	}

	const bool operator!=(const HEX& other)  const
	{
		return q != other.q || r != other.r;
	}

	size_t GetHash() const
	{
		hash<int> int_hash;
		size_t hq = int_hash(q);
		size_t hr = int_hash(r);
		return hq ^ (hr + 0x9e3779b9 + (hq << 6) + (hq >> 2));

	}

	uint32 Make_Int() const
	{
		return ((short)q & 0x0000ffff) | (((short)(r)) << 16);
	}

	static uint32 Make_Int(HEX a)
	{
		return ((short)a.q & 0x0000ffff) | (((short)(a.r)) << 16);
	}

	static HEX Make_FromInt(uint32 v)
	{
		short q = (short)(v & 0x0000ffff);
		short r = (short)((v & 0xffff0000) >> 16);

		return HEX(q, r);
	}

	static HEX Rotate_Left(HEX a)
	{
		return HEX(-a.s, -a.q, -a.r);
	}


	static HEX Rotate_Right(HEX a)
	{
		return HEX(-a.r, -a.s, -a.q);
	}

	static int Distance(HEX a, HEX b)
	{
		return (a - b).Length();
	}

	static HEX hex_directions[MAX_DIRECTION];
	static HEX Direction(int dir)
	{
		return hex_directions[dir];
	}
	static HEX GetNeighbor(HEX hex, int dir)
	{
		return hex + Direction(dir);
	}
	static void GetNeighbors(HEX hex, HEX* pOut)
	{
		for (int i = 0; i < MAX_DIRECTION; i++)
		{
			pOut[i] = GetNeighbor(hex, i);
		}
	}
	static void Ring(HEX Center, int Radius, std::vector<HEX>& vecHex)
	{
		HEX cube = Center + HEX::Direction(4) * Radius;
		for (int i = 0; i < MAX_DIRECTION; i++)
		{
			for (int j = 0; j < Radius; j++)
			{
				vecHex.push_back(cube);
				cube = HEX::GetNeighbor(cube, i);
			}
		}
	}
	static void Spiral(HEX Center, int Radius, std::vector<HEX>& vecHex)
	{
		vecHex.push_back(Center);
		for (int j = 1; j <= Radius; j++)
		{
			HEX::Ring(Center, j, vecHex);
		}
	}

	//获取弧形角点
	static void ArcTopAngle(HEX Center, HEX Start, HEX End, bool isClockwise, std::vector<HEX>& vecHex)
	{
		vecHex.clear();
		IF_NOT(Start != End)
			return;

		auto Radius = (Center - Start).Length();
		IF_NOT(Radius > 0)
			return;

		std::vector<HEX> vecRound;
		RoundTopAngle(Center, Radius, isClockwise, vecRound);
		for (auto& cube : vecRound)
		{
			if (cube == Start)
			{
				vecHex.push_back(Start);
				continue;
			}

			if (!vecHex.empty())
			{
				vecHex.push_back(cube);
				if (cube == End)
					break;
			}
		}
		IF_NOT(!vecHex.empty())
		{
			vecHex.clear();
			return;
		}
		if (vecHex.back() != End)
		{
			for (auto& cube : vecRound)
			{
				vecHex.push_back(cube);
				if (cube == End)
					break;
			}
		}

		IF_NOT(vecHex.back() == End)
		{
			vecHex.clear();
			return;
		}
	}

	//获取圆形6个顶角
	static void RoundTopAngle(HEX Center, int Radius, bool isClockwise, std::vector<HEX>& vecHex)
	{
		auto& vecRoundTopAngle = s_hsRoundTopAngle[Radius][isClockwise];
		if (vecRoundTopAngle.empty())
		{
			vecRoundTopAngle = MakeTemplateTop(Radius, isClockwise);
			IF_NOT(!vecRoundTopAngle.empty())
				return;
		}
		for (auto& cube : vecRoundTopAngle)
		{
			vecHex.push_back(Center + cube);
		}
	}

private:
	//生成圆形6个顶角
	static std::vector<HEX> MakeTemplateTop(int Radius, bool isClockwise) {
		std::vector<HEX> result;
		for (int i = 0; i < MAX_DIRECTION; i++)
		{
			HEX cube = Direction(i) * Radius;
			result.push_back(cube); //顺时针
		}
		if (!isClockwise) //逆时针存放
			std::reverse(result.begin(), result.end());
		return result;
	}
private:
	static std::unordered_map<int, std::map<bool, std::vector<HEX>>> s_hsRoundTopAngle; //<Radius, <isClockwise, >> //Clockwise=true为顺时针
};


namespace std {
	template <> struct hash<HEX> {
		size_t operator()(const HEX& h) const {
			hash<int> int_hash;
			size_t hq = int_hash(h.q);
			size_t hr = int_hash(h.r);
			return hq ^ (hr + 0x9e3779b9 + (hq << 6) + (hq >> 2));
		}
	};
}

struct FractionalHex
{
	const double q;
	const double r;
	const double s;
	FractionalHex(double q_, double r_, double s_) : q(q_), r(r_), s(s_) {
		if (::round(q + r + s) != 0) throw "q + r + s must be 0";
	}

	HEX hex_round()
	{
		int qi = int(::round(q));
		int ri = int(::round(r));
		int si = int(::round(s));
		double q_diff = abs(qi - q);
		double r_diff = abs(ri - r);
		double s_diff = abs(si - s);
		if (q_diff > r_diff && q_diff > s_diff)
		{
			qi = -ri - si;
		}
		else
			if (r_diff > s_diff)
			{
				ri = -qi - si;
			}
			else
			{
				si = -qi - ri;
			}
		return HEX(qi, ri, si);
	}
};



struct OffsetCoord
{
	const int col;
	const int row;
	OffsetCoord(int col_, int row_) : col(col_), row(row_) {}

	OffsetCoord operator-() const { return OffsetCoord(-col, -row); }

	static const int EVEN = 1;
	static const int ODD = -1;
	static OffsetCoord QoffsetFromCube(int offset, HEX h)
	{
		int col = h.q;
		int row = h.r + (int)((h.q + offset * (h.q & 1)) / 2);
		if (offset != OffsetCoord::EVEN && offset != OffsetCoord::ODD)
		{
			throw ("offset must be EVEN (+1) or ODD (-1)");
		}
		return OffsetCoord(col, row);
	}


	static HEX QoffsetToCube(int offset, OffsetCoord h)
	{
		int q = h.col;
		int r = h.row - (int)((h.col + offset * (h.col & 1)) / 2);
		int s = -q - r;
		if (offset != OffsetCoord::EVEN && offset != OffsetCoord::ODD)
		{
			throw ("offset must be EVEN (+1) or ODD (-1)");
		}
		return HEX(q, r, s);
	}


	static OffsetCoord RoffsetFromCube(int offset, HEX h)
	{
		int col = h.q + (int)((h.r + offset * (h.r & 1)) / 2);
		int row = h.r;
		if (offset != OffsetCoord::EVEN && offset != OffsetCoord::ODD)
		{
			throw ("offset must be EVEN (+1) or ODD (-1)");
		}
		return OffsetCoord(col, row);
	}


	static HEX RoffsetToCube(int offset, OffsetCoord h)
	{
		int q = h.col - (int)((h.row + offset * (h.row & 1)) / 2);
		int r = h.row;
		int s = -q - r;
		if (offset != OffsetCoord::EVEN && offset != OffsetCoord::ODD)
		{
			throw ("offset must be EVEN (+1) or ODD (-1)");
		}
		return HEX(q, r, s);
	}
};

template<class T>
struct Point
{
	const T x;
	const T y;
	Point(T x_, T y_) : x(x_), y(y_) {}
};



inline FractionalHex hex_lerp(FractionalHex a, FractionalHex b, double t)
{
	return FractionalHex(a.q * (1.0 - t) + b.q * t, a.r * (1.0 - t) + b.r * t, a.s * (1.0 - t) + b.s * t);
}


inline std::vector<HEX> hex_linedraw(HEX a, HEX b)
{
	int N = HEX::Distance(a, b);
	FractionalHex a_nudge = FractionalHex(a.q + 1e-06, a.r + 1e-06, a.s - 2e-06);
	FractionalHex b_nudge = FractionalHex(b.q + 1e-06, b.r + 1e-06, b.s - 2e-06);
	std::vector<HEX> results = {};
	double step = 1.0 / (N > 1 ? N : 1);
	for (int i = 0; i <= N; i++)
	{
		results.push_back((hex_lerp(a_nudge, b_nudge, step * i)).hex_round());
	}
	return results;
}
